// ssdb_node.h — Cabeçalho do binding entre o core C++ e o Node.js
// Aqui declaramos o que será exposto como módulo N-API.

#ifndef SYSSTONE_NODE_H
#define SYSSTONE_NODE_H

#include <napi.h>
#include <memory>
#include "engine.h"

// Namespace próprio do binding
namespace sysstone_node {

// Classe que envolve o Engine para ser usada pelo Node
// Ela herda de Napi::ObjectWrap, que é o jeito moderno de expor C++ pro JS
class Database : public Napi::ObjectWrap<Database> {
public:
    // Método estático que inicializa a classe no Node
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    // Construtor — chamado quando o JS faz "new Database(path)"
    explicit Database(const Napi::CallbackInfo& info);

    // Destrutor — limpa recursos quando o objeto morre no JS
    ~Database();

private:
    // O motor de verdade, gerenciado por um smart pointer
    std::unique_ptr<sysstone::Engine> engine_;

    // Métodos expostos ao JavaScript
    Napi::Value Open(const Napi::CallbackInfo& info);
    Napi::Value Close(const Napi::CallbackInfo& info);
    Napi::Value Put(const Napi::CallbackInfo& info);
    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Remove(const Napi::CallbackInfo& info);
    Napi::Value Keys(const Napi::CallbackInfo& info);
};

} // namespace sysstone_node

#endif // SYSSTONE_NODE_H