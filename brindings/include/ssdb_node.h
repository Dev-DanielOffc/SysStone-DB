// ssdb_node.h — Cabeçalho do binding N-API (versão completa)

#ifndef SYSSTONE_NODE_H
#define SYSSTONE_NODE_H

#include <napi.h>
#include <memory>
#include "engine.h"

namespace sysstone_node {

class Database : public Napi::ObjectWrap<Database> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit Database(const Napi::CallbackInfo& info);
    ~Database();

private:
    std::unique_ptr<sysstone::Engine> engine_;

    Napi::Value Open(const Napi::CallbackInfo& info);
    Napi::Value Close(const Napi::CallbackInfo& info);
    Napi::Value Put(const Napi::CallbackInfo& info);
    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Remove(const Napi::CallbackInfo& info);
    Napi::Value Keys(const Napi::CallbackInfo& info);
    Napi::Value Find(const Napi::CallbackInfo& info);
    Napi::Value Count(const Napi::CallbackInfo& info);
};

} // namespace sysstone_node

#endif // SYSSTONE_NODE_H