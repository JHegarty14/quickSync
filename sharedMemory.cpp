#include <atomic>
#include <string>
#include <napi.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include <errno.h>

const std::size_t SHM_SIZE = 1024;

template<class T>
class SharedMem {
  int fd;
  T* ptr;
  const char* name;

  public:    
    SharedMem(const char* name, bool owner=false) {
        fd = shm_open(name, O_RDWR | O_CREAT, 0600);
        if (fd == -1) {
            std::cerr << "Failed to open a shared memory region" << std::endl;
            return;
        }
        if (ftruncate(fd, sizeof(T)) < 0) {
            close(fd);
            std::cerr << "Failed to set size of a shared memory region" << std::endl;
            return;
        };
        ptr = (T*)mmap(nullptr, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (!ptr) {
            close(fd);
            std::cerr << "Failed to mmap a shared memory region" << std::endl;
            return;
        }
        this->name = owner ? name : nullptr;
    }

    ~SharedMem() {
        munmap(ptr, sizeof(T));
        close(fd);
        if (name) {
            std::cout << "Remove shared mem instance " << name << std::endl;
            shm_unlink(name);
        }
    }

    T& get() const {
        return *ptr;
    }
};

class Atom {
  private:
    std::shared_ptr<std::atomic<std::string>> sharedMemory;
    const char* atomName;

  public:
    Atom(std::string& name, Napi::Value value) {
      this->atomName = name.c_str();
      sharedMemory = std::make_shared<std::atomic<std::string>>(value.As<Napi::String>().Utf8Value());
    }

    ~Atom() {
      this->DeleteAtom(atomName);
    }

    void CompareAndSwap(Napi::Env env, Napi::Function callback) {
      std::cout << "CompareAndSwap" << std::endl;
      std::string loadedVal = sharedMemory->load();
      Napi::String currentVal = Napi::String::New(env, loadedVal);

      std::string result = callback.Call({ currentVal }).As<Napi::String>().Utf8Value();

      if (sharedMemory->compare_exchange_strong(loadedVal, result)) {
        return;
      }
      return CompareAndSwap(env, callback);
    }

    std::string GetValue() {
      return sharedMemory->load();
    }

    void DeleteAtom(const std::string& name) {
      shm_unlink(name.c_str());
    }
};

std::unordered_map<std::string, Atom*> atoms;

Napi::Value CreateAtom(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  Napi::String value = info[1].As<Napi::String>();
  std::string valueString = value.ToString();

  Atom* atom = new Atom(atomRef, value);
  atoms[atomRef] = atom;

  std::string atomStr = atom->GetValue();

  return Napi::String::New(env, atomStr);
}

Napi::Value CompareAndSwap(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Expected arg for callback to be a function.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();
  Napi::Function callback = info[1].As<Napi::Function>();

  Atom* atom = atoms[atomRef];

  if (!atom) {
    Napi::TypeError::New(env, "Atom does not exist").ThrowAsJavaScriptException();
    return env.Null();
  }

  atom->CompareAndSwap(env, callback);

  return;
}

Napi::Value DeleteAtom(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();

  Atom* atom = atoms[atomRef];
  if (!atom) {
    Napi::TypeError::New(env, "Atom does not exist").ThrowAsJavaScriptException();
    return env.Null();
  }

  delete atom;
  atoms.erase(atomRef);

  return env.Null();
}

Napi::Value GetAtomValue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Expected arg for atomRef to be string.").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomRef = info[0].As<Napi::String>().ToString();

  Atom* atom = atoms[atomRef];

  if (!atom) {
    Napi::TypeError::New(env, "Atom does not exist").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string atomStr = atom->GetValue();

  return Napi::String::New(env, atomStr);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "createAtom"),
              Napi::Function::New(env, CreateAtom));
  exports.Set(Napi::String::New(env, "compareAndSwap"),
              Napi::Function::New(env, CompareAndSwap));
  exports.Set(Napi::String::New(env, "deleteAtom"),
              Napi::Function::New(env, DeleteAtom));
  exports.Set(Napi::String::New(env, "getAtomValue"),
              Napi::Function::New(env, GetAtomValue));

  return exports;
};

NODE_API_MODULE(sharedMemoryNode, Init);
