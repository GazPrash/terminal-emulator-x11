#include "include/x11_if.h"
#include <X11/Xlib.h>
// #include <stdio.h>
#include <unistd.h>

int main() {
  X11_If *x11 = build_x11_interface(800, 600, 20000);
  XSelectInput(x11->display, x11->window, ExposureMask | KeyPressMask);
  XMapWindow(x11->display, x11->window);
  // GC gc = XCreateGC(x11->display, x11->window, 0, NULL);
  // XFreeGC(x11->display, gc);
  /* Flush the output buffer */
  XFlush(x11->display);
  sleep(4);
  XCloseDisplay(x11->display);
}
