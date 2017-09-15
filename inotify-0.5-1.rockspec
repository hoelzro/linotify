package = "inotify"
version = "0.5-1"
source = {
        url = 'https://github.com/hoelzro/linotify/archive/0.5.tar.gz',
        dir = 'linotify-0.5',
}
description = {
        summary  = "Inotify bindings for Lua",
        homepage = "http://hoelz.ro/projects/linotify",
        license  = "MIT"
}
dependencies = {
        "lua >= 5.1"
}
external_dependencies = {
        INOTIFY = {
                header = "sys/inotify.h"
        }
}
build = {
        type = "builtin",
        modules = {
                inotify = {
                        sources = {"linotify.c"},
                        incdirs = {"$(INOTIFY_INCDIR)"},
                        libdirs = {"$(INOTIFY_LIBDIR)"}
                }
        }
}
