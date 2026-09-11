// storage.h — Cabeçalho do sistema de armazenamento em disco
// Aqui definimos como o SysStone-DB grava dados no disco.
// Diferente do SQLite (um arquivo só), usamos SEGMENTOS em um diretório.

#ifndef SYSSTONE_STORAGE_H
#define SYSSTONE_STORAGE_H

#include <string>
#include <vector>
#include <mutex>

namespace sysstone {

// Um segmento é um arquivo append-only no disco.
// Cada segmento tem um tamanho máximo; quando enche, criamos outro.
class Segment {
public:
    // Construtor recebe o caminho completo do arquivo do segmento
    explicit Segment(const std::string& file_path);

    // Abre o segmento para leitura e escrita
    bool open();

    // Fecha o segmento
    void close();

    // Adiciona um registro no final do arquivo (append-only)
    // Formato: [tamanho_key][key][tamanho_value][value]
    bool append(const std::string& key, const std::string& value);

    // Lê todos os registros do segmento (para carregar em memória)
    std::vector<std::pair<std::string, std::string>> read_all();

    // Retorna o tamanho atual do arquivo em bytes
    size_t size() const;

    // Retorna o caminho do arquivo
    const std::string& path() const { return file_path_; }

private:
    std::string file_path_;
    bool is_open_;
    size_t current_size_;
    std::mutex mutex_;
};

// Gerencia vários segmentos de uma coleção.
// Quando um segmento passa do tamanho máximo, cria outro.
class SegmentManager {
public:
    // Construtor recebe a pasta onde os segmentos ficam
    explicit SegmentManager(const std::string& dir_path);

    // Abre ou cria a pasta de segmentos
    bool open();

    // Fecha todos os segmentos abertos
    void close();

    // Escreve um par chave/valor (vai pro segmento atual)
    bool write(const std::string& key, const std::string& value);

    // Lê TODOS os registros de TODOS os segmentos (ordem cronológica)
    std::vector<std::pair<std::string, std::string>> read_all();

    // Retorna quantos segmentos existem
    size_t segment_count() const;

private:
    std::string dir_path_;
    std::vector<Segment> segments_;
    size_t current_segment_index_;
    bool is_open_;
    std::mutex mutex_;

    // Tamanho máximo de cada segmento (1 MB por enquanto)
    static constexpr size_t MAX_SEGMENT_SIZE = 1024 * 1024;

    // Descobre o próximo índice de segmento disponível
    size_t find_next_segment_index();
};

} // namespace sysstone

#endif // SYSSTONE_STORAGE_H