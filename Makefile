inotify.so:
	gcc -c -fPIC linotify.c
	gcc -o inotify.so -shared linotify.o -llua -lm -ldl

clean:
	rm -f inotify.so linotify.o
