{
  "targets": [
    {
      "target_name": "sysstone",

      "sources": [
        "bindings/src/ssdb_node.cpp",
        "core/src/engine.cpp",
        "core/src/storage.cpp",
        "core/src/query.cpp"
      ],

      "include_dirs": [
        "core/include",
        "bindings/include",
        "<!(node -p \"require('node-addon-api').include_dir\")"
      ],

      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],

      "cflags_cc": [
        "-std=c++17",
        "-O2",
        "-fexceptions"
      ],

      "conditions": [
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "Optimization": 2,
              "AdditionalOptions": ["/std:c++17"]
            }
          }
        }],
        ["OS=='mac'", {
          "xcode_settings": {
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "OTHER_CPLUSPLUSFLAGS": ["-O2"]
          }
        }],
        ["OS=='linux'", {
          "cflags_cc": ["-std=c++17", "-O2", "-fexceptions"]
        }]
      ]
    }
  ]
}