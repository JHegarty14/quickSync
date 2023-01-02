{
    "targets": [
        {
            "target_name": "sharedMemoryNode",
            "sources": ["sharedMemory.cpp"],
            "include_dirs": [
                "<!(node -p \"require('node-addon-api').include_dir\")",
                "./deps/rapidjson" 
            ],
            "cflags!": [
            "-fno-exceptions"
            ],
            "cflags_cc!": [
            "-fno-exceptions"
            ],
            "conditions": [
                [
                    "OS=='win'", {
                    "msvs_settings": {
                        "VCCLCompilerTool": {
                        "ExceptionHandling": 1
                        }
                    }
                    }
                ],
                [
                    "OS=='mac' or OS=='linux'", {
                    "xcode_settings": {
                        "OTHER_CFLAGS": ["-fexceptions"],
                        "OTHER_CPLUSPLUSFLAGS": ["-fexceptions"],
                    },
                    "link_settings": {
                        "OTHER_LDFLAGS": ["-rdynamic"]
                    }
                    }
                ]
            ],
            "xcode-settings": {
                "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
            }
        }
    ]
}