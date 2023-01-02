{
    "targets": [
        {
            "target_name": "sharedMemoryNode",
            "sources": ["sharedMemory.cpp"],
            "include_dirs": [
                "<!(node -p \"require('node-addon-api').include_dir\")",
                "./deps/rapidjson" 
            ],
            "conditions": [
                [ 'OS=="mac"', {
                    "defines": ["NAPI_CPP_EXCEPTIONS"],
                    "xcode_settings": {
                        "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++17", "-stdlib=libc++", "-pthread", '-fexceptions', '-frtti' ],
                        "OTHER_CFLAGS": [ "-std=c++17", "-stdlib=libc++", '-fexceptions' "-pthread",'-fexceptions', '-frtti' ],
                        "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                        "MACOSX_DEPLOYMENT_TARGET": "10.7",
                        "CLANG_CXX_LANGUAGE_STANDARD":"c++17",
                        "CLANG_CXX_LIBRARY": "libc++"
                    }
                },
                 'OS=="linux"', {
                    "defines": ["NAPI_CPP_EXCEPTIONS"],
                     "xcode_settings": {
                        "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++17", "-stdlib=libc++", "-pthread", '-fexceptions', '-frtti' ],
                        "OTHER_CFLAGS": [ "-std=c++17", "-stdlib=libc++", "-pthread", '-fexceptions', '-frtti' ],
                        "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                        "CLANG_CXX_LANGUAGE_STANDARD":"c++17",
                        "CLANG_CXX_LIBRARY": "libc++"
                    }
                 }]
            ],
        }
    ]
}