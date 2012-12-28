package = "inotify"
version = "0.2-1"
source = {
	url = 'https://github.com/hoelzro/linotify/archive/0.2.tar.gz',
        dir = 'linotify-0.2',
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
