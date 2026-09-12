// engine.cpp — Implementação completa do motor do SysStone-DB
// Agora com persistência real em segmentos e queries funcionais.

#include "engine.h"

#include <fstream>
#include <filesystem>
#include <iostream>

namespace sysstone {

namespace fs = std::filesystem;

// ============================
// Construtor / Destrutor
// ============================

Engine::Engine(const std::string& path)
    : path_(path), is_open_(false) {}

Engine::~Engine() {
    if (is_open_) {
        close();
    }
}

// ============================
// Ciclo de vida
// ============================

bool Engine::open() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (is_open_) return true;

    try {
        if (!fs::exists(path_)) {
            fs::create_directories(path_);
        }
    } catch (const std::exception& e) {
        std::cerr << "[SysStone] Erro ao criar diretório: "
                  << e.what() << std::endl;
        return false;
    }

    // Descobre coleções existentes no disco e carrega cada uma
    try {
        for (const auto& entry : fs::directory_iterator(path_)) {
            if (entry.is_directory()) {
                std::string coll = entry.path().filename().string();
                load_collection(coll);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[SysStone] Erro ao carregar coleções: "
                  << e.what() << std::endl;
        return false;
    }

    is_open_ = true;
    return true;
}

bool Engine::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) return true;

    // Fecha todos os segment managers
    for (auto& [name, store] : stores_) {
        store->close();
    }
    stores_.clear();
    data_.clear();

    is_open_ = false;
    return true;
}

// ============================
// Helpers internos
// ============================

std::string Engine::collection_path(const std::string& collection) {
    return path_ + "/" + collection;
}

SegmentManager* Engine::get_or_create_store(const std::string& collection) {
    auto it = stores_.find(collection);
    if (it != stores_.end()) {
        return it->second.get();
    }

    // Cria um novo SegmentManager para essa coleção
    std::string coll_path = collection_path(collection);
    auto store = std::make_unique<SegmentManager>(coll_path);

    if (!store->open()) {
        return nullptr;
    }

    SegmentManager* ptr = store.get();
    stores_[collection] = std::move(store);
    return ptr;
}

void Engine::load_collection(const std::string& collection) {
    // Cria o store e lê todos os segmentos da coleção
    SegmentManager* store = get_or_create_store(collection);
    if (!store) return;

    auto records = store->read_all();

    // Aplica em memória — o último valor de uma chave sobrescreve
    auto& coll_map = data_[collection];
    for (auto& [key, value] : records) {
        coll_map[key] = value;
    }
}

void Engine::save_to_disk(const std::string& collection) {
    // Garante que o store existe (mesmo que não tenha nada novo)
    get_or_create_store(collection);
    // Os writes já vão direto pro disco no put(), então aqui é no-op.
    // Mantido para futuras compactações.
}

// ============================
// Operações básicas
// ============================

bool Engine::put(const std::string& collection,
                 const std::string& key,
                 const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) return false;

    // Garante que o store existe
    SegmentManager* store = get_or_create_store(collection);
    if (!store) return false;

    // Escreve no disco (append-only)
    if (!store->write(key, value)) {
        return false;
    }

    // Atualiza cache em memória
    data_[collection][key] = value;
    return true;
}

bool Engine::get(const std::string& collection,
                 const std::string& key,
                 std::string& out_value) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) return false;

    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) return false;

    auto it_key = it_coll->second.find(key);
    if (it_key == it_coll->second.end()) return false;

    out_value = it_key->second;
    return true;
}

bool Engine::remove(const std::string& collection,
                    const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) return false;

    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) return false;

    bool erased = it_coll->second.erase(key) > 0;

    // Grava um "tombstone" no disco pra marcar remoção
    // Formato especial: chave com prefixo especial e valor vazio
    if (erased) {
        SegmentManager* store = get_or_create_store(collection);
        if (store) {
            store->write("\x01__deleted__" + key, "");
        }
    }

    return erased;
}

std::vector<std::string> Engine::keys(const std::string& collection) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> result;
    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) return result;

    for (const auto& [k, v] : it_coll->second) {
        result.push_back(k);
    }
    return result;
}

std::vector<std::pair<std::string, std::string>>
Engine::find(const std::string& collection,
             const std::vector<Filter>& filters) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::pair<std::string, std::string>> result;

    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) return result;

    for (const auto& [key, value] : it_coll->second) {
        if (matches_all_filters(value, filters)) {
            result.emplace_back(key, value);
        }
    }
    return result;
}

size_t Engine::count(const std::string& collection) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) return 0;
    return it_coll->second.size();
}

} // namespace sysstone