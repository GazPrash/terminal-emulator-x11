# -I/usr/include/freetype2 -I/include/ -lX11 -lXft -lfreetype

cmake_minimum_required(VERSION 3.10)
project(temm C)
set(CMAKE_C_STANDARD 99)

file(GLOB SRC "src/*.c")
add_executable(temm ${SRC} "main.c")

target_include_directories(temm PRIVATE /usr/include/freetype2 include)
target_link_libraries(temm X11 Xft freetype)
