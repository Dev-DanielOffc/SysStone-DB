// query.cpp — Implementação das queries
// Aqui fazemos o parsing e a comparação de valores.

#include "query.h"

#include <regex>
#include <sstream>

namespace sysstone {

// Converte "$gt" em Operator::GT, etc
Operator parse_operator(const std::string& op_str) {
    if (op_str == "$eq")  return Operator::EQ;
    if (op_str == "$ne")  return Operator::NE;
    if (op_str == "$gt")  return Operator::GT;
    if (op_str == "$gte") return Operator::GTE;
    if (op_str == "$lt")  return Operator::LT;
    if (op_str == "$lte") return Operator::LTE;
    if (op_str == "$contains") return Operator::CONTAINS;
    return Operator::EQ; // padrão
}

// Extrai o valor de um campo em um JSON plano
// Exemplo: extract_field(R"({"idade":20})", "idade") -> "20"
static std::string extract_field(const std::string& json,
                                 const std::string& field) {
    // Busca "field": seguido de valor
    std::string pattern = "\"" + field + "\"\\s*:\\s*";
    std::regex re(pattern);
    std::smatch match;

    if (!std::regex_search(json, match, re)) {
        return "";
    }

    size_t start = match.position() + match.length();
    size_t pos = start;

    // Se for string (começa com aspas), pega até fechar
    if (pos < json.size() && json[pos] == '"') {
        pos++;
        size_t end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }

    // Senão, pega até vírgula ou chave de fechamento
    size_t end = pos;
    while (end < json.size() &&
           json[end] != ',' &&
           json[end] != '}' &&
           json[end] != ' ') {
        end++;
    }
    return json.substr(pos, end - pos);
}

// Compara dois valores como números (se possível)
static bool try_compare_numeric(const std::string& a,
                                const std::string& b,
                                int& result) {
    try {
        double da = std::stod(a);
        double db = std::stod(b);
        if (da < db) result = -1;
        else if (da > db) result = 1;
        else result = 0;
        return true;
    } catch (...) {
        return false;
    }
}

bool matches_filter(const std::string& json_value, const Filter& filter) {
    std::string field_value = extract_field(json_value, filter.field);
    if (field_value.empty()) return false;

    switch (filter.op) {
        case Operator::EQ:
            return field_value == filter.value;

        case Operator::NE:
            return field_value != filter.value;

        case Operator::CONTAINS:
            return field_value.find(filter.value) != std::string::npos;

        case Operator::GT:
        case Operator::GTE:
        case Operator::LT:
        case Operator::LTE: {
            int cmp = 0;
            if (!try_compare_numeric(field_value, filter.value, cmp)) {
                cmp = field_value.compare(filter.value);
            }
            switch (filter.op) {
                case Operator::GT:  return cmp > 0;
                case Operator::GTE: return cmp >= 0;
                case Operator::LT:  return cmp < 0;
                case Operator::LTE: return cmp <= 0;
                default: return false;
            }
        }
    }
    return false;
}

bool matches_all_filters(const std::string& json_value,
                         const std::vector<Filter>& filters) {
    // Se não há filtros, tudo passa
    if (filters.empty()) return true;

    for (const auto& f : filters) {
        if (!matches_filter(json_value, f)) {
            return false;
        }
    }
    return true;
}

} // namespace sysstone