
SRCNAME = linotify.c
OBJNAME = linotify.o
LIBNAME = inotify.so

# Gives a nice speedup but also spoils debugging on x86. Comment
# out this line when debugging.
OMIT_FRAME_POINTER = -fomit-frame-pointer

# Seach for lua .pc file
LUAPKG_CMD = pkg-config --list-all | grep Lua | awk '{print $$1}'
CFLAGS = `sh -c "pkg-config \`$(LUAPKG_CMD)\` --cflags"` -fPIC -O3 -Wall
LFLAGS = -shared $(OMIT_FRAME_POINTER)
INSTALL_PATH = `sh -c "pkg-config \`$(LUAPKG_CMD)\` --variable=INSTALL_CMOD"`

## If your system doesn't have pkg-config, comment out the previous
## lines and uncomment and change the following ones according to your
## building enviroment.

#CFLAGS = -I/usr/include/lua5.1/ -fPIC -O3 -Wall
#LFLAGS = -shared $(OMIT_FRAME_POINTER)
#INSTALL_PATH = /usr/lib/lua/5.1

all: $(LIBNAME)

$(OBJNAME): $(SRCNAME)
	$(CC) -o $(OBJNAME) -c $(SRCNAME) $(CFLAGS)

$(LIBNAME): $(OBJNAME)
	$(CC) -o $(LIBNAME) -shared $(OBJNAME) $(LFLAGS)

install: $(LIBNAME)
	install -D -s $(LIBNAME) $(DESTDIR)/$(INSTALL_PATH)/$(LIBNAME)

clean:
	$(RM) $(LIBNAME) $(OBJNAME)

