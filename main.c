#include "include/pty_pt.h"
#include "include/render.h"
#include "include/x11_if.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

#define dlog printf("%s\n", "dlog - main.c");

int x_error_handler(Display *display, XErrorEvent *error) {
  char error_text[1024];
  XGetErrorText(display, error->error_code, error_text, sizeof(error_text));
  // Output error details
  fprintf(stderr, "X Error: %s\n", error_text);
  fprintf(stderr, "Request code: %d\n", error->request_code);
  fprintf(stderr, "Minor code: %d\n", error->minor_code);
  fprintf(stderr, "Resource ID: 0x%lx\n", error->resourceid);

  return 0; // Return 0 to continue the program
}

int main() {

  char *fontname = "Fragment Mono:size=14";
  X11_If *x11 = x11_init(15, 51, 200);

  // xft config and setup
  load_font(x11, fontname);
  build_x11_interface(x11);
  setup_xft(x11);

  PTY *pty = init_pty();
  if (!attatch_pty(pty)) {
    return 1;
  }
  if (!record_termsize_ioctl_pty(x11, pty)) {
    return 1;
  }
  if (!spawn_process(pty)) {
    return 1;
  }

  // XSetErrorHandler(x_error_handler);
  // XSelectInput(x11->display, x11->window, ExposureMask | KeyPressMask);
  // XMapWindow(x11->display, x11->window);
  // GC gc = XCreateGC(x11->display, x11->window, 0, NULL);
  // XFreeGC(x11->display, gc);
  render_mainloop(x11, pty);
  /* Flush the output buffer */
  // XFlush(x11->display);
  XftColorFree(x11->display, DefaultVisual(x11->display, x11->screen),
               DefaultColormap(x11->display, x11->screen), x11->xftcolor);
  XftDrawDestroy(x11->xftdraw);
  XftFontClose(x11->display, x11->xftfont);
  // XUnloadFont(display, font);
  XCloseDisplay(x11->display);
  XftDrawDestroy(x11->xftdraw);
  free(x11->buff);
  free(x11->xftcolor);
  free(x11);
  XCloseDisplay(x11->display);
}
