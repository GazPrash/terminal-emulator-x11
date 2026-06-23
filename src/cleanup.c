#include "cleanup.h"
#include <stdio.h>
#include <stdlib.h>

void call_cleanup(X11_If *x11, PTY *pty) {
  printf("Window close requested, performing cleanup...\n");
  XftColorFree(x11->display, DefaultVisual(x11->display, x11->screen),
               DefaultColormap(x11->display, x11->screen), x11->xftcolor);
  XftFontClose(x11->display, x11->xftfont);
  XCloseDisplay(x11->display);
  XftDrawDestroy(x11->xftdraw);
  for (int i = 0; i < x11->cell_y; i++) {
    free(x11->buff[i]);
  }
  free(x11->buff);
  free(x11->xftcolor);
  free(x11);
  free(pty);
}
