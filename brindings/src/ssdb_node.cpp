// ssdb_node.cpp — Binding N-API completo
// Expõe: open, close, put, get, remove, keys, find, count

#include "ssdb_node.h"

namespace sysstone_node {

Napi::Object Database::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Database", {
        InstanceMethod("open",   &Database::Open),
        InstanceMethod("close",  &Database::Close),
        InstanceMethod("put",    &Database::Put),
        InstanceMethod("get",    &Database::Get),
        InstanceMethod("remove", &Database::Remove),
        InstanceMethod("keys",   &Database::Keys),
        InstanceMethod("find",   &Database::Find),
        InstanceMethod("count",  &Database::Count),
    });

    exports.Set("Database", func);
    return exports;
}

Database::Database(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Database>(info) {

    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Caminho do banco é obrigatório (string)")
            .ThrowAsJavaScriptException();
        return;
    }

    std::string path = info[0].As<Napi::String>().Utf8Value();
    engine_ = std::make_unique<sysstone::Engine>(path);
}

Database::~Database() {}

Napi::Value Database::Open(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, engine_->open());
}

Napi::Value Database::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, engine_->close());
}

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

    return Napi::Boolean::New(env, engine_->put(collection, key, value));
}

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

    if (!engine_->get(collection, key, value)) {
        return env.Null();
    }
    return Napi::String::New(env, value);
}

Napi::Value Database::Remove(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "remove precisa de 2 argumentos")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    std::string key        = info[1].As<Napi::String>().Utf8Value();

    return Napi::Boolean::New(env, engine_->remove(collection, key));
}

Napi::Value Database::Keys(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "keys precisa de 1 argumento")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    auto keys = engine_->keys(collection);

    Napi::Array arr = Napi::Array::New(env, keys.size());
    for (size_t i = 0; i < keys.size(); i++) {
        arr[i] = Napi::String::New(env, keys[i]);
    }
    return arr;
}

// find(collection, filters_json)
// filters_json: string JSON tipo '[{"field":"idade","op":"$gt","value":"18"}]'
Napi::Value Database::Find(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "find precisa de 2 argumentos")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    std::string filters_raw = info[1].As<Napi::String>().Utf8Value();

    // Parse manual simples do JSON de filtros
    // Formato esperado: [{"field":"x","op":"$gt","value":"10"}, ...]
    std::vector<sysstone::Filter> filters;

    size_t pos = 0;
    while ((pos = filters_raw.find("\"field\"", pos)) != std::string::npos) {
        sysstone::Filter f;

        // Extrai field
        size_t fstart = filters_raw.find('"', pos + 8);
        size_t fend = filters_raw.find('"', fstart + 1);
        if (fstart == std::string::npos || fend == std::string::npos) break;
        f.field = filters_raw.substr(fstart + 1, fend - fstart - 1);

        // Extrai op
        size_t opos = filters_raw.find("\"op\"", fend);
        if (opos == std::string::npos) break;
        size_t ostart = filters_raw.find('"', opos + 5);
        size_t oend = filters_raw.find('"', ostart + 1);
        if (ostart == std::string::npos || oend == std::string::npos) break;
        std::string op_str = filters_raw.substr(ostart + 1, oend - ostart - 1);
        f.op = sysstone::parse_operator(op_str);

        // Extrai value
        size_t vpos = filters_raw.find("\"value\"", oend);
        if (vpos == std::string::npos) break;
        size_t vstart = filters_raw.find('"', vpos + 8);
        size_t vend = filters_raw.find('"', vstart + 1);
        if (vstart == std::string::npos || vend == std::string::npos) break;
        f.value = filters_raw.substr(vstart + 1, vend - vstart - 1);

        filters.push_back(f);
        pos = vend + 1;
    }

    auto results = engine_->find(collection, filters);

    // Retorna array de strings JSON (cada uma é um documento)
    Napi::Array arr = Napi::Array::New(env, results.size());
    for (size_t i = 0; i < results.size(); i++) {
        arr[i] = Napi::String::New(env, results[i].second);
    }
    return arr;
}

Napi::Value Database::Count(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "count precisa de 1 argumento")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string collection = info[0].As<Napi::String>().Utf8Value();
    size_t total = engine_->count(collection);
    return Napi::Number::New(env, static_cast<double>(total));
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return sysstone_node::Database::Init(env, exports);
}

NODE_API_MODULE(sysstone, InitAll)

} // namespace sysstone_node