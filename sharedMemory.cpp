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
      sem_close(_semaphore);
      sem_unlink(_atomName);
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
      if (_atomValue.get() == currentDocument) {
        std::string callbackResult = callback.Call({ DocumentToString(env, *currentDocument) }).As<Napi::String>().Utf8Value();
        rapidjson::Document newDocument;
        newDocument.Parse(callbackResult.c_str());
        _atomValue = std::make_shared<rapidjson::Document>(std::move(newDocument));
        sem_post(_semaphore);
        return true;
      }

      return this->CompareAndSwap(callback);
    }

    bool ResetAtomValue(const char * newValue) {
      sem_wait(_semaphore);
      std::lock_guard<std::mutex> lock(_mutex);
      rapidjson::Document document;
      document.Parse(newValue);
      _atomValue = std::make_shared<rapidjson::Document>(std::move(document));
      sem_post(_semaphore);
      return true;
    }

  private:
    sem_t* _semaphore;
    std::mutex _mutex;
    const char * _atomName;
    std::shared_ptr<rapidjson::Document> _atomValue;
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

  bool result = atom->CompareAndSwap(callback);

  return Napi::Boolean::New(env, result);  
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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "createAtom"), Napi::Function::New(env, CreateAtom));
  exports.Set(Napi::String::New(env, "getAtomValue"), Napi::Function::New(env, GetAtomValue));
  exports.Set(Napi::String::New(env, "compareAndSwap"), Napi::Function::New(env, CompareAndSwap));
  exports.Set(Napi::String::New(env, "deleteAtom"), Napi::Function::New(env, DeleteAtom));
  exports.Set(Napi::String::New(env, "resetAtom"), Napi::Function::New(env, ResetAtom));

  return exports;
}

NODE_API_MODULE(sharedMemoryNode, Init);
