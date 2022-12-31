{
    "targets": [
        {
            "target_name": "sharedMemoryNode",
            "sources": ["sharedMemory.cpp"],
            "include_dirs": [
                "<!(node -p \"require('node-addon-api').include_dir\")",
                "./deps/rapidjson" 
            ],
            'cflags': [ '-fno-exceptions' ],
            'cflags_cc': [ '-fno-exceptions' ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
            "conditions": [
                [ 'OS=="mac"', {
                    "xcode_settings": {
                        "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++17", "-stdlib=libc++", "-pthread", '-fno-exceptions', '-frtti' ],
                        "OTHER_CFLAGS": [ "-std=c++17", "-stdlib=libc++", "-pthread", '-fno-exceptions', '-frtti' ],
                        "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                        "MACOSX_DEPLOYMENT_TARGET": "10.7",
                        "CLANG_CXX_LANGUAGE_STANDARD":"c++17",
                        "CLANG_CXX_LIBRARY": "libc++"
                    }
                }]
            ],
        }
    ]
}