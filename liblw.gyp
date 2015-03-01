{
    "target_defaults" : {
        "cflags" : [ "-Wall", "-std=c++11" ]
    },
    "targets" : [{
        "target_name" : "liblw",
        "type" : "shared_library",
        "include_dirs" : [ "./source" ],
        "cflags" : [ "-fPIC" ],
        "libraries" : [ "/usr/local/lib/libuv.a" ],
        "sources" : [
            "source/lw/Application.cpp",
            "source/lw/Application.hpp",
            "source/lw/Singleton.hpp",

            "source/lw/event/Loop.cpp",
            "source/lw/event/Loop.hpp"
        ]
    }]
}
