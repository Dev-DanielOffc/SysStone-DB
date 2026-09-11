// query.h — Cabeçalho do sistema de queries do SysStone-DB
// Permite buscar documentos com filtros tipo MongoDB: { idade: { $gt: 18 } }

#ifndef SYSSTONE_QUERY_H
#define SYSSTONE_QUERY_H

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace sysstone {

// Operadores suportados nas queries
enum class Operator {
    EQ,   // igual
    NE,   // diferente
    GT,   // maior que
    GTE,  // maior ou igual
    LT,   // menor que
    LTE,  // menor ou igual
    CONTAINS // contém (busca substring)
};

// Um filtro simples: campo + operador + valor
struct Filter {
    std::string field;
    Operator op;
    std::string value;
};

// Aplica um filtro em um valor JSON simples
// Por enquanto assumimos que o valor é um JSON plano de 1 nível
bool matches_filter(const std::string& json_value, const Filter& filter);

// Verifica se um documento (JSON) passa por TODOS os filtros
bool matches_all_filters(const std::string& json_value,
                         const std::vector<Filter>& filters);

// Converte string de operador ("$gt", "$lt") em enum
Operator parse_operator(const std::string& op_str);

} // namespace sysstone

#endif // SYSSTONE_QUERY_H