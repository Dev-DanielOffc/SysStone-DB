// ssdb_node.cpp — Implementação do binding N-API
// Aqui traduzimos chamadas JavaScript em chamadas C++ e vice-versa.

#include "ssdb_node.h"

namespace sysstone_node {

// Inicializa a classe Database e registra os métodos no Node
Napi::Object Database::Init(Napi::Env env, Napi::Object exports) {
    // Define a função construtora "Database"
    Napi::Function func = DefineClass(env, "Database", {
        InstanceMethod("open",   &Database::Open),
        InstanceMethod("close",  &Database::Close),
        InstanceMethod("put",    &Database::Put),
        InstanceMethod("get",    &Database::Get),
        InstanceMethod("remove", &Database::Remove),
        InstanceMethod("keys",   &Database::Keys),
    });

    // Permite "new Database()" no JavaScript
    exports.Set("Database", func);
    return exports;
}

// Construtor — recebe o caminho do banco como primeiro argumento
Database::Database(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Database>(info) {

    Napi::Env env = info.Env();

    // Verifica se o usuário passou o caminho
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Caminho do banco é obrigatório (string)")
            .ThrowAsJavaScriptException();
        return;
    }

    // Pega o caminho como string C++
    std::string path = info[0].As<Napi::String>().Utf8Value();

    // Cria o Engine de verdade
    engine_ = std::make_unique<sysstone::Engine>(path);
}

// Destrutor — garante que o Engine seja liberado
Database::~Database() {
    // O unique_ptr cuida disso automaticamente
}

// Implementa db.open()
Napi::Value Database::Open(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool ok = engine_->open();
    return Napi::Boolean::New(env, ok);
}

// Implementa db.close()
Napi::Value Database::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool ok = engine_->close();
    return Napi::Boolean::New(env, ok);
}

// Implementa db.put(collection, key, value)
Napi::Value Database::Put(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3) {
        Napi::TypeError::New(env, "put precisa de 3 argumentos")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    std::string key        = info[1].As<Napi::String>().Utf8Value();
    std::string value      = info[2].As<Napi::String>().Utf8Value();

    bool ok = engine_->put(collection, key, value);
    return Napi::Boolean::New(env, ok);
}

// Implementa db.get(collection, key)
Napi::Value Database::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "get precisa de 2 argumentos")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    std::string key        = info[1].As<Napi::String>().Utf8Value();
    std::string value;

    bool found = engine_->get(collection, key, value);

    // Se não encontrou, retorna null no JS
    if (!found) {
        return env.Null();
    }

    return Napi::String::New(env, value);
}

// Implementa db.remove(collection, key)
Napi::Value Database::Remove(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "remove precisa de 2 argumentos")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    std::string key        = info[1].As<Napi::String>().Utf8Value();

    bool ok = engine_->remove(collection, key);
    return Napi::Boolean::New(env, ok);
}

// Implementa db.keys(collection)
Napi::Value Database::Keys(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "keys precisa de 1 argumento")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    auto keys = engine_->keys(collection);

    // Converte std::vector<std::string> em array JS
    Napi::Array arr = Napi::Array::New(env, keys.size());
    for (size_t i = 0; i < keys.size(); i++) {
        arr[i] = Napi::String::New(env, keys[i]);
    }
    return arr;
}

// Função de entrada do módulo — chamada quando o Node faz "require"
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return sysstone_node::Database::Init(env, exports);
}

// Registra o módulo como "sysstone"
NODE_API_MODULE(sysstone, InitAll)

} // namespace sysstone_node