package = "inotify"
version = "1.0-0"
source = {
	url = "git://github.com/hoelzro/linotify.git"
}
description = {
	summary = "Inotify bindings for Lua",
	homepage = "http://hoelz.ro/projects/linotify",
	license = "MIT"
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