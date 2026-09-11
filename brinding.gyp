# Arquivo de configuração do node-gyp
# Ele diz como compilar o core em C++ para gerar o módulo .node

{
  "//": "Nome do alvo que será compilado",
  "targets": [
    {
      "//": "Nome do módulo final (vira sysstone.node)",
      "target_name": "sysstone",

      "//sources": "Lista de arquivos C++ que serão compilados",
      "sources": [
        "bindings/src/ssdb_node.cpp",
        "core/src/engine.cpp"
      ],

      "//include_dirs": "Pastas onde o compilador procura os headers (.h)",
      "include_dirs": [
        "core/include",
        "bindings/include"
      ],

      "//cflags_cc": "Flags extras para o compilador C++",
      "cflags_cc": [
        "-std=c++17",
        "-O2",
        "-fexceptions"
      ],

      "//conditions": "Configurações específicas por sistema operacional",
      "conditions": [
        ["OS=='win'", {
          "//": "Configurações para Windows (MSVC)",
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "Optimization": 2,
              "AdditionalOptions": ["/std:c++17"]
            }
          }
        }],
        ["OS=='mac'", {
          "//": "Configurações para macOS (Clang)",
          "xcode_settings": {
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "OTHER_CPLUSPLUSFLAGS": ["-O2"]
          }
        }]
      ]
    }
  ]
}