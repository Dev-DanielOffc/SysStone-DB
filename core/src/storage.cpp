// storage.cpp — Implementação do armazenamento em segmentos
// Aqui gravamos e lemos dados do disco de verdade.

#include "storage.h"

#include <fstream>
#include <filesystem>
#include <cstring>
#include <algorithm>

namespace sysstone {

namespace fs = std::filesystem;

// ============================
// Segment
// ============================

Segment::Segment(const std::string& file_path)
    : file_path_(file_path), is_open_(false), current_size_(0) {}

bool Segment::open() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_open_) return true;

    // Cria o arquivo se não existir
    if (!fs::exists(file_path_)) {
        std::ofstream create(file_path_, std::ios::binary);
        create.close();
    }

    // Pega o tamanho atual do arquivo
    current_size_ = fs::file_size(file_path_);
    is_open_ = true;
    return true;
}

void Segment::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    is_open_ = false;
}

bool Segment::append(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_open_) return false;

    // Abre em modo append binário
    std::ofstream out(file_path_, std::ios::binary | std::ios::app);
    if (!out) return false;

    // Formato do registro:
    // [uint32 tamanho_key][key][uint32 tamanho_value][value]
    uint32_t key_size   = static_cast<uint32_t>(key.size());
    uint32_t value_size = static_cast<uint32_t>(value.size());

    out.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
    out.write(key.data(), key.size());
    out.write(reinterpret_cast<const char*>(&value_size), sizeof(value_size));
    out.write(value.data(), value.size());

    out.close();

    // Atualiza tamanho atual
    current_size_ += sizeof(key_size) + key.size()
                   + sizeof(value_size) + value.size();

    return true;
}

std::vector<std::pair<std::string, std::string>> Segment::read_all() {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::pair<std::string, std::string>> records;

    std::ifstream in(file_path_, std::ios::binary);
    if (!in) return records;

    while (in.good()) {
        uint32_t key_size = 0;
        in.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        if (!in.good() || key_size == 0) break;

        std::string key(key_size, '\0');
        in.read(&key[0], key_size);
        if (!in.good()) break;

        uint32_t value_size = 0;
        in.read(reinterpret_cast<char*>(&value_size), sizeof(value_size));
        if (!in.good()) break;

        std::string value(value_size, '\0');
        in.read(&value[0], value_size);
        if (!in.good()) break;

        records.emplace_back(std::move(key), std::move(value));
    }

    return records;
}

size_t Segment::size() const {
    return current_size_;
}

// ============================
// SegmentManager
// ============================

SegmentManager::SegmentManager(const std::string& dir_path)
    : dir_path_(dir_path), current_segment_index_(0), is_open_(false) {}

bool SegmentManager::open() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_open_) return true;

    // Cria o diretório se não existir
    try {
        if (!fs::exists(dir_path_)) {
            fs::create_directories(dir_path_);
        }
    } catch (...) {
        return false;
    }

    // Descobre o próximo índice disponível
    current_segment_index_ = find_next_segment_index();

    // Se não existe nenhum segmento, cria o primeiro
    if (current_segment_index_ == 0) {
        std::string first = dir_path_ + "/segment-00000.log";
        segments_.emplace_back(first);
        segments_.back().open();
    } else {
        // Carrega todos os segmentos existentes
        for (size_t i = 0; i < current_segment_index_; i++) {
            char buf[32];
            snprintf(buf, sizeof(buf), "/segment-%05zu.log", i);
            std::string path = dir_path_ + buf;
            if (fs::exists(path)) {
                segments_.emplace_back(path);
                segments_.back().open();
            }
        }
    }

    is_open_ = true;
    return true;
}

void SegmentManager::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& seg : segments_) {
        seg.close();
    }
    segments_.clear();
    is_open_ = false;
}

bool SegmentManager::write(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_open_) return false;

    // Se não tem segmento, cria um
    if (segments_.empty()) {
        std::string path = dir_path_ + "/segment-00000.log";
        segments_.emplace_back(path);
        segments_.back().open();
    }

    // Se o segmento atual está cheio, cria um novo
    if (segments_.back().size() >= MAX_SEGMENT_SIZE) {
        char buf[32];
        snprintf(buf, sizeof(buf), "/segment-%05zu.log",
                 ++current_segment_index_);
        std::string path = dir_path_ + buf;
        segments_.emplace_back(path);
        segments_.back().open();
    }

    return segments_.back().append(key, value);
}

std::vector<std::pair<std::string, std::string>>
SegmentManager::read_all() {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::pair<std::string, std::string>> all;

    // Lê todos os segmentos em ordem cronológica
    for (auto& seg : segments_) {
        auto records = seg.read_all();
        all.insert(all.end(),
                   std::make_move_iterator(records.begin()),
                   std::make_move_iterator(records.end()));
    }

    return all;
}

size_t SegmentManager::segment_count() const {
    return segments_.size();
}

size_t SegmentManager::find_next_segment_index() {
    size_t max_index = 0;
    try {
        for (const auto& entry : fs::directory_iterator(dir_path_)) {
            std::string name = entry.path().filename().string();
            if (name.rfind("segment-", 0) == 0) {
                size_t idx = std::stoul(name.substr(8, 5));
                if (idx + 1 > max_index) max_index = idx + 1;
            }
        }
    } catch (...) {
        return 0;
    }
    return max_index;
}

} // namespace sysstone