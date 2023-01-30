#define NAPI_CPP_EXCEPTIONS

#include <napi.h>
#include <semaphore.h>
#include <fcntl.h>
#include <mutex>
#include <iostream>
#include <unordered_map>
#include "deps/rapidjson/document.h"
#include "deps/rapidjson/writer.h"
#include "deps/rapidjson/stringbuffer.h"

Napi::String DocumentToString(const Napi::Env& env, const rapidjson::Document& doc) {
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
  doc.Accept(writer);
  return Napi::String::New(env, buf.GetString(), buf.GetSize());
}

void CallbackHandler(Napi::Env env, Napi::Function jsCallback, const char * data) {
  std::cout << "CallbackHandler" << std::endl;
  std::cout.flush();

  jsCallback.Call({ Napi::String::New(env, data) });
};

void WatcherHandler(Napi::Env env, Napi::Function jsCallback, std::initializer_list<napi_value> *args) {
  std::cout << "WatcherHandler" << jsCallback << std::endl;
  std::cout.flush();

  try {
    jsCallback.Call(*args);
  } catch (const Napi::Error& e) {
    std::cout << "Error: " << e.what() << std::endl;
    std::cout.flush();
  }
};

bool DefaultValidator(const std::initializer_list<napi_value> &_) {
  return true;
}

class Atom {
  public:
    Atom(Napi::Env env, const char * atomName, const char * atomValue) : _atomName(atomName) {
      _semaphore = sem_open(atomName, O_CREAT, 0644, 1);
      if (_semaphore == SEM_FAILED) {
        Napi::Error::New(env, "Failed to open semaphore").ThrowAsJavaScriptException();
      }

      sem_wait(_semaphore);
      std::lock_guard<std::mutex> lock(_mutex);
      rapidjson::Document document;
      document.Parse(atomValue);
      _atomValue = std::make_shared<rapidjson::Document>(std::move(document));
      
      sem_post(_semaphore);
    }

    ~Atom() {
      try {
        sem_close(_semaphore);
        sem_unlink(_atomName);
      } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
      }
    }

    const char * GetAtomValue(Napi::Env env) {
      sem_wait(_semaphore);
      std::lock_guard<std::mutex> lock(_mutex);
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      _atomValue->Accept(writer);

      size_t valueLength = strlen(buffer.GetString()) + 1;
      char * value = new char[valueLength];
      strncpy(value, buffer.GetString(), valueLength);

      sem_post(_semaphore);

      return value;
    }

    bool CompareAndSwap(Napi::Function callback) {
      sem_wait(_semaphore);
      std::lock_guard<std::mutex> lock(_mutex);
      Napi::Env env = callback.Env();
      rapidjson::Document *currentDocument = _atomValue.get();
      std::string callbackResult;
      if (_atomValue.get() == currentDocument) {
        try {
          callbackResult = callback.Call({ DocumentToString(env, *currentDocument) }).As<Napi::String>().Utf8Value();
          if (_validatorFn != nullptr) {
            Napi::HandleScope scope(env);
            Napi::ThreadSafeFunction tsValidator = *_validatorFn.get();

            auto status = tsValidator.BlockingCall(callbackResult.c_str(), CallbackHandler);
            if (status != napi_ok) {
              std::cout << "VALIDATION ERROR" << status << std::endl;
              std::cout.flush();

              Napi::Error::New(env, "Validation failed.").ThrowAsJavaScriptException();
              sem_post(_semaphore);
              return false;
            }
          }
        } catch (const Napi::Error& e) {
          sem_post(_semaphore);
          Napi::Error::New(env, e.Message()).ThrowAsJavaScriptException();
          return false;
        }
        rapidjson::Document newDocument;
        newDocument.Parse(callbackResult.c_str());
        _atomValue = std::make_shared<rapidjson::Document>(std::move(newDocument));
        sem_post(_semaphore);

        for (auto const& [key, fn] : _watcherFnMap) {
          Napi::ThreadSafeFunction tsFn = *fn.get();

          std::initializer_list<napi_value> argList = { Napi::String::New(env, _atomName), DocumentToString(env, *currentDocument), DocumentToString(env, newDocument) };

          try {
            tsFn.NonBlockingCall(&argList, WatcherHandler);
          } catch (const Napi::Error& e) {
            std::cout << "WATCHER ERROR" << e.Message() << std::endl;
            std::cout.flush();
          }
          // fn.MakeCallback(fnEnv.Global(), { Napi::String::New(fnEnv, _atomName), DocumentToString(fnEnv, *currentDocument), DocumentToString(fnEnv, newDocument) });

          std::cout << "AFTER CALL " << &_atomName << std::endl;
          std::cout.flush();
        }

        return true;
      }

      std::cout << "RECURRING" << std::endl;
      std::cout.flush();
      return this->CompareAndSwap(callback);
    }

    bool ResetAtomValue(const char * newValue) {
      try {
        sem_wait(_semaphore);
        std::lock_guard<std::mutex> lock(_mutex);
        rapidjson::Document document;
        document.Parse(newValue);
        _atomValue = std::make_shared<rapidjson::Document>(std::move(document));
        sem_post(_semaphore);
      } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
      }
      return true;
    }

    bool SetValidator(Napi::Function callback) {
      try {
        sem_wait(_semaphore);
        std::lock_guard<std::mutex> lock(_mutex);
        Napi::ThreadSafeFunction tsFn = Napi::ThreadSafeFunction::New(callback.Env(), callback , "validator", 0, 1);
        _validatorFn = std::make_shared<Napi::ThreadSafeFunction>(std::move(tsFn));
        sem_post(_semaphore);
      } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
      }
      return true;
    }

    bool RemoveValidator() {
      try {
        sem_wait(_semaphore);
        std::lock_guard<std::mutex> lock(_mutex);
        _validatorFn = nullptr;
        sem_post(_semaphore);
      } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
      }

      return true;
    }

    bool AddWatcher(const char * watcherName, Napi::Function callback) {
      try {
        sem_wait(_semaphore);
        std::lock_guard<std::mutex> lock(_mutex);
        std::shared_ptr<Napi::ThreadSafeFunction> tsFn = std::make_shared<Napi::ThreadSafeFunction>(Napi::ThreadSafeFunction::New(callback.Env(), callback, watcherName, 0, 1));
        _watcherFnMap.insert(std::pair<std::string, std::shared_ptr<Napi::ThreadSafeFunction>>(watcherName, tsFn));
        // _watcherFnMap[watcherName] = Napi::ThreadSafeFunction::New(callback.Env(), callback, watcherName, 0, 1);
        sem_post(_semaphore);
      } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
      }
      return true;
    }

    bool RemoveWatcher(const char * watcherName) {
      try {
        sem_wait(_semaphore);
        std::lock_guard<std::mutex> lock(_mutex);
        _watcherFnMap.erase(watcherName);
        sem_post(_semaphore);
      } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
      }
      return true;
    }

  private:
    sem_t* _semaphore;
    std::mutex _mutex;
    const char * _atomName;
    std::shared_ptr<rapidjson::Document> _atomValue;
    std::shared_ptr<Napi::ThreadSafeFunction> _validatorFn;
    std::unordered_map<std::string, std::shared_ptr<Napi::ThreadSafeFunction>> _watcherFnMap;
};

std::unordered_map<std::string, std::shared_ptr<Atom>> atomMap;

Napi::Value CreateAtom(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  std::string atomValue = info[1].As<Napi::String>().ToString();

  std::shared_ptr<Atom> atom = std::make_shared<Atom>(env, atomRef.c_str(), atomValue.c_str());

  atomMap[atomRef] = atom;

  return Napi::Boolean::New(env, true);
}

Napi::Value GetAtomValue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  const char * result = atom->GetAtomValue(env);

  if (strlen(result) < 1) {
    Napi::Error::New(env, "Atom " + atomRef + " value is empty").ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::String::New(env, result);
}

Napi::Value CompareAndSwap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Expected arg for callback to be function.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  Napi::Function callback = info[1].As<Napi::Function>();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  try {
    bool result = atom->CompareAndSwap(callback);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return Napi::Boolean::New(env, false);
  }
}

Napi::Value DeleteAtom(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  atom->~Atom();

  atomMap.erase(atomRef);

  return Napi::Boolean::New(env, true);
}

Napi::Value ResetAtom(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomValue = info[1].As<Napi::String>().ToString();

  atom->ResetAtomValue(atomValue.c_str());

  return Napi::Boolean::New(env, true);
}

Napi::Value SetAtomValidator(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Expected arg for callback to be function.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  Napi::Function callback = info[1].As<Napi::Function>();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  atom->SetValidator(callback);

  return Napi::Boolean::New(env, true);
}

Napi::Value RemoveAtomValidator(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  atom->RemoveValidator();

  return Napi::Boolean::New(env, true);
}

Napi::Value AddAtomWatcher(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for watcher key to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[2].IsFunction()) {
    Napi::TypeError::New(env, "Expected arg for watcher function to be function.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  std::string watcherKey = info[1].As<Napi::String>().ToString();
  Napi::Function callback = info[2].As<Napi::Function>();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  atom->AddWatcher(watcherKey.c_str(), callback);

  return Napi::Boolean::New(env, true);
}

Napi::Value RemoveAtomWatcher(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for watcher key to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  std::string watcherKey = info[1].As<Napi::String>().ToString();

  std::shared_ptr<Atom> atom = atomMap[atomRef];

  if (atom == nullptr) {
    Napi::Error::New(env, "Atom not found").ThrowAsJavaScriptException();
    return env.Null();
  }

  atom->RemoveWatcher(watcherKey.c_str());

  return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "createAtom"), Napi::Function::New(env, CreateAtom));
  exports.Set(Napi::String::New(env, "getAtomValue"), Napi::Function::New(env, GetAtomValue));
  exports.Set(Napi::String::New(env, "compareAndSwap"), Napi::Function::New(env, CompareAndSwap));
  exports.Set(Napi::String::New(env, "deleteAtom"), Napi::Function::New(env, DeleteAtom));
  exports.Set(Napi::String::New(env, "resetAtom"), Napi::Function::New(env, ResetAtom));
  exports.Set(Napi::String::New(env, "setAtomValidator"), Napi::Function::New(env, SetAtomValidator));
  exports.Set(Napi::String::New(env, "removeAtomValidator"), Napi::Function::New(env, RemoveAtomValidator));
  exports.Set(Napi::String::New(env, "addAtomWatcher"), Napi::Function::New(env, AddAtomWatcher));
  exports.Set(Napi::String::New(env, "removeAtomWatcher"), Napi::Function::New(env, RemoveAtomWatcher));

  return exports;
}

NODE_API_MODULE(sharedMemoryNode, Init);
