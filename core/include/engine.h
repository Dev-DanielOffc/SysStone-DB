// engine.h — Cabeçalho principal do core do SysStone-DB
// Esta versão já integra storage (segmentos) e query (filtros).

#ifndef SYSSTONE_ENGINE_H
#define SYSSTONE_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

#include "storage.h"
#include "query.h"

namespace sysstone {

// Estrutura que representa um documento
struct Document {
    std::string key;
    std::string value;
};

// Classe principal do motor — agora com persistência real em segmentos
class Engine {
public:
    explicit Engine(const std::string& path);
    ~Engine();

    // Ciclo de vida
    bool open();
    bool close();
    bool is_open() const { return is_open_; }

    // Operações básicas
    bool put(const std::string& collection,
             const std::string& key,
             const std::string& value);

    bool get(const std::string& collection,
             const std::string& key,
             std::string& out_value);

    bool remove(const std::string& collection,
                const std::string& key);

    std::vector<std::string> keys(const std::string& collection);

    // Busca com filtros (tipo MongoDB)
    // Retorna lista de pares (key, value) que batem com os filtros
    std::vector<std::pair<std::string, std::string>>
    find(const std::string& collection,
         const std::vector<Filter>& filters);

    // Conta quantos documentos existem na coleção
    size_t count(const std::string& collection);

private:
    std::string path_;
    bool is_open_;

    // Cache em memória: coleção -> (chave -> valor)
    // Fonte da verdade é o disco (segmentos), memória é cache rápido
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> data_;

    // Um gerenciador de segmentos por coleção
    std::unordered_map<std::string,
        std::unique_ptr<SegmentManager>> stores_;

    std::mutex mutex_;

    // Helpers internos
    void load_collection(const std::string& collection);
    void save_to_disk(const std::string& collection);
    SegmentManager* get_or_create_store(const std::string& collection);
    std::string collection_path(const std::string& collection);
};

} // namespace sysstone

#endif // SYSSTONE_ENGINE_H