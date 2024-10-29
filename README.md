>[!NOTE]
> Currently in development

# temm - terminal emulator
- tell (and) evaluate (on) my machine - terminal emulator
- A very basic and minimal terminal built using the default X11/Xlib (X-Window System), hence making it capable of running on virtually
  every linux distribution

# Demo
![temm demo](demo/temm_demo.gif)

# How to use?

## Pre-requisites
- [1] `gcc` or any other c compiler
- [2] Cmake (>= v3.10)
## Build:
- [1] Create a `build` directory and `cd build` into it.
- [2] `cmake ..`
- [4] `cmake --build .`
- [3] `./temm`

## Sources and References
- [1]: X11/Xlib.c Docs: https://www.x.org/docs/X11/xlib.pdf
- [2]: Eduterm: https://www.uninformativ.de/git/eduterm/file/README.html
- [3]: tty/pty (Linux Manual Page): https://man7.org/linux/man-pages/man7/pty.7.html
- [4]: Monospace font used in testing : https://github.com/be5invis/Iosevka/releases/tag/v31.9.1
