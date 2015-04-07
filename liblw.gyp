{
    "variables" : {
        "uv_library" : "static_library"
    },
    "target_defaults" : {
        "cflags" : [ "-Wall", "-fPIC" ],
    },
    "includes" : [ "external/libuv/uv.gypi" ],
    "targets" : [{
        "target_name" : "liblw",
        "type" : "shared_library",
        "include_dirs" : [ "./source" ],
        "dependencies" : [ "libuv" ],
        "cflags" : [ "-std=c++1y" ],
        "direct_dependent_settings" : {
            "include_dirs" : [ "./source" ],
            "libraries" : [ "-pthread" ],
            "cflags" : [ "-std=c++1y" ]
        },
        "sources" : [
            "source/lw/Application.cpp",
            "source/lw/Application.hpp",
            "source/lw/event.hpp",
            "source/lw/fs.hpp",
            "source/lw/iter.hpp"
            "source/lw/memory.hpp",
            "source/lw/Singleton.hpp",

            "source/lw/event/Idle.cpp",
            "source/lw/event/Idle.hpp",
            "source/lw/event/Loop.cpp",
            "source/lw/event/Loop.hpp",
            "source/lw/event/Promise.hpp",
            "source/lw/event/Promise.impl.hpp",
            "source/lw/event/Promise.void.hpp",
            "source/lw/event/Timeout.cpp",
            "source/lw/event/Timeout.hpp",
            "source/lw/event/Timeout.impl.hpp",

            "source/lw/fs/File.cpp",
            "source/lw/fs/File.hpp",

            "source/lw/iter/Iterable.hpp",
            "source/lw/iter/RandomAccessIterator.hpp",

            "source/lw/memory/Buffer.cpp",
            "source/lw/memory/Buffer.hpp"
        ]
    },{
        "target_name" : "libgtest",
        "type" : "static_library",
        "include_dirs" : [ "./external/gtest/include" ],
        "direct_dependent_settings" : {
            "include_dirs" : [ "./external/gtest/include" ]
        },
        "sources" : [ "external/gtest/fused-src/gtest/gtest-all.cc" ]
    },{
        "target_name" : "liblw-tests",
        "type" : "executable",
        "dependencies" : [ "liblw", "libgtest" ],
        "include_dirs" : [ "./tests" ],
        "sources" : [
            "tests/main.cpp",

            "tests/event/LoopBasicTests.cpp",
            "tests/event/PromiseDestructionTests.cpp",
            "tests/event/PromiseIntSynchronousTests.cpp",
            "tests/event/PromiseVoidSynchronousTests.cpp",
            "tests/event/PromiseRejectionTests.cpp",
            "tests/event/TimeoutHelperTests.cpp",
            "tests/event/TimeoutTests.cpp",

            "tests/fs/FileTests.cpp"
        ]
    }]
}
