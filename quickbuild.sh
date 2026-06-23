#!/bin/bash
set -e

echo "Building..."

OS="$(uname -s)"
if [ "$OS" = "Darwin" ]; then
    gcc main.c src/*.c -Iinclude -I/opt/X11/include -I/opt/X11/include/freetype2 -L/opt/X11/lib -lX11 -lXft -lfreetype -o temm
else
    gcc main.c src/*.c -Iinclude -I/usr/include/freetype2 -lX11 -lXft -lfreetype -o temm
fi

echo "Build successful! added: ./temm"
