main:
	gcc -o main main.c src/x11_if.c -I/usr/include/freetype2 -I/include/x11_if.h -lX11 -lXft -lfreetype
