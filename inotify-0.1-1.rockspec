package = "inotify"
version = "0.1-1"
source = {
	url = 'https://github.com/downloads/hoelzro/linotify/linotify-0.1.tar.gz'
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
