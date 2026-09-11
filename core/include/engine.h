// engine.h — Cabeçalho principal do core do SysStone-DB
// Aqui declaramos a classe Engine, que é o coração do banco de dados.

#ifndef SYSSTONE_ENGINE_H
#define SYSSTONE_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

// Namespace próprio do projeto — evita conflito com outras libs
namespace sysstone {

// Estrutura simples que representa um documento armazenado
// (por enquanto é só uma string, mas depois vira algo mais sério)
struct Document {
    std::string key;    // chave única do documento
    std::string value;  // conteúdo serializado (JSON por enquanto)
};

// Classe principal do motor do banco de dados
// Ela vai gerenciar coleções, documentos e persistência
class Engine {
public:
    // Construtor — recebe o caminho do diretório onde o banco fica
    explicit Engine(const std::string& path);

    // Destrutor — fecha o banco e limpa recursos
    ~Engine();

    // Abre o banco (carrega dados do disco, se existirem)
    bool open();

    // Fecha o banco (salva tudo antes de sair)
    bool close();

    // Insere ou atualiza um documento dentro de uma coleção
    bool put(const std::string& collection,
             const std::string& key,
             const std::string& value);

    // Busca um documento pela chave
    bool get(const std::string& collection,
             const std::string& key,
             std::string& out_value);

    // Remove um documento
    bool remove(const std::string& collection,
                const std::string& key);

    // Lista todas as chaves de uma coleção
    std::vector<std::string> keys(const std::string& collection);

private:
    // Caminho do diretório onde o banco está salvo
    std::string path_;

    // Flag que indica se o banco está aberto
    bool is_open_;

    // Estrutura em memória: coleção -> (chave -> valor)
    // Depois isso vira algo mais otimizado (B-tree / LSM)
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> data_;

    // Mutex para evitar condições de corrida entre threads
    std::mutex mutex_;

    // Método interno para carregar dados do disco
    void load_from_disk();

    // Método interno para salvar dados no disco
    void save_to_disk();
};

} // namespace sysstone

#endif // SYSSTONE_ENGINE_H