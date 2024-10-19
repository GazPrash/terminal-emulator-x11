#include "../include/x11_if.h"
#include <X11/Xlib.h>
#include <stdlib.h>
#include <unistd.h>

#define CHAR_LIMIT_SCREEN 4096;

X11_If *build_x11_interface(XftFont *xftfont, int row, int col, int timeout) {

  Display *display;
  display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Error while opening display");
    return NULL;
  }
  int screen = DefaultScreen(display);

  X11_If *x11 = (X11_If *)malloc(sizeof(X11_If) * 1);
  x11->display = display;
  x11->screen = screen;
  x11->timeout = timeout;
  /* The terminal will have a fixed size of 80x25 cells. This is an
   * arbitrary number. No resizing has been implemented and child
   * processes can't even ask us for the current size (for now).
   *
   * buff_x, buff_y will be the current cursor position. */
  // x11->buff_w = w;
  // x11->buff_h = h;
  x11->cell_x = row;
  x11->cell_y = col;
  x11->pos_x = 0;
  x11->pos_y = 0;
  XGlyphInfo extents;
  const char *text = "A";
  XftTextExtentsUtf8(x11->display, xftfont, (const FcChar8 *)text, strlen(text),
                     &extents);
  x11->font_w = extents.xOff;
  x11->font_h = xftfont->ascent + xftfont->descent;

  x11->buff = (char *)malloc(sizeof(char) * 4096);
  if (x11->buff == NULL) {
    perror("malloc");
    return NULL;
  }

  x11->w = x11->cell_x * x11->font_w;
  x11->h = x11->cell_y * x11->font_h;

  Window window = XCreateSimpleWindow(
      display, RootWindow(display, screen), 0, 0, x11->w, x11->h, 1,
      BlackPixel(display, screen), WhitePixel(display, screen));
  x11->window = window;

  XStoreName(x11->display, x11->window, "temm 0.01");
  return x11;
}

void handle_event(XKeyEvent *event, PTY *pty) {
  char buffer[32];
  KeySym ksym;
  int num = XLookupString(event, buffer, sizeof(buffer), &ksym, 0);
  for (int i = 0; i < num; i++)
    write(pty->master, &buffer[i], 1);
}
