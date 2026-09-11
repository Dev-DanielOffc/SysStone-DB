// engine.cpp — Implementação do motor do SysStone-DB
// Aqui a mágica acontece: gerenciamos dados, disco e concorrência.

#include "engine.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

// Namespace do projeto
namespace sysstone {

// Atalho para não escrever std::filesystem toda hora
namespace fs = std::filesystem;

// Construtor — só guarda o caminho e marca como fechado
Engine::Engine(const std::string& path)
    : path_(path), is_open_(false) {
    // Nada mais a fazer aqui por enquanto
}

// Destrutor — garante que o banco seja fechado corretamente
Engine::~Engine() {
    if (is_open_) {
        close();
    }
}

// Abre o banco: cria a pasta se não existir e carrega os dados
bool Engine::open() {
    // Trava o mutex para evitar acesso simultâneo
    std::lock_guard<std::mutex> lock(mutex_);

    // Se já estiver aberto, não faz nada
    if (is_open_) {
        return true;
    }

    // Cria o diretório do banco se ele não existir
    try {
        if (!fs::exists(path_)) {
            fs::create_directories(path_);
        }
    } catch (const std::exception& e) {
        std::cerr << "[SysStone] Erro ao criar diretório: "
                  << e.what() << std::endl;
        return false;
    }

    // Carrega dados existentes do disco
    load_from_disk();

    is_open_ = true;
    return true;
}

// Fecha o banco, salvando tudo antes
bool Engine::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) {
        return true;
    }

    // Salva os dados no disco antes de fechar
    save_to_disk();

    is_open_ = false;
    data_.clear();
    return true;
}

// Insere ou atualiza um documento
bool Engine::put(const std::string& collection,
                 const std::string& key,
                 const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) {
        return false;
    }

    // Insere ou sobrescreve o valor
    data_[collection][key] = value;
    return true;
}

// Busca um documento pela chave
bool Engine::get(const std::string& collection,
                 const std::string& key,
                 std::string& out_value) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) {
        return false;
    }

    // Procura a coleção
    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) {
        return false;
    }

    // Procura a chave dentro da coleção
    auto it_key = it_coll->second.find(key);
    if (it_key == it_coll->second.end()) {
        return false;
    }

    out_value = it_key->second;
    return true;
}

// Remove um documento
bool Engine::remove(const std::string& collection,
                    const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!is_open_) {
        return false;
    }

    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) {
        return false;
    }

    return it_coll->second.erase(key) > 0;
}

// Lista todas as chaves de uma coleção
std::vector<std::string> Engine::keys(const std::string& collection) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> result;
    auto it_coll = data_.find(collection);
    if (it_coll == data_.end()) {
        return result;
    }

    // Percorre todas as chaves e adiciona ao resultado
    for (const auto& pair : it_coll->second) {
        result.push_back(pair.first);
    }
    return result;
}

// Carrega dados do disco (implementação simples por enquanto)
void Engine::load_from_disk() {
    // Por enquanto não carregamos nada — isso virá na Fase 1
    // Aqui é onde vamos ler os arquivos de segmento depois
}

// Salva dados no disco (implementação simples por enquanto)
void Engine::save_to_disk() {
    // Por enquanto não salvamos nada — isso virá na Fase 1
    // Aqui é onde vamos escrever os segmentos depois
}

} // namespace sysstone