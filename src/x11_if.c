#include "../include/x11_if.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#define dlog printf("%s\n", "dlog - x11_if.c");

X11_If *x11_init(int row, int col, int timeout) {

  Display *display;
  display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Error while opening display");
    return NULL;
  }
  int screen = DefaultScreen(display);
  // printf("screen : %d \n", screen);

  X11_If *x11 = (X11_If *)malloc(sizeof(X11_If) * 1);
  x11->display = display;
  x11->screen = screen;
  x11->timeout = timeout;
  XEvent event;
  x11->event = event;
  x11->fd = ConnectionNumber(x11->display);
  /* The terminal will have a fixed size of 80x25 cells. This is an
   * arbitrary number. No resizing has been implemented and child
   * processes can't even ask us for the current size (for now).
   *
   * buff_x, buff_y will be the current cursor position. */
  x11->cell_y = row;
  x11->cell_x = col;

  x11->buff = (char **)malloc(sizeof(char *) * row);
  for (int i = 0; i < row; i++) {
    x11->buff[i] = (char *)malloc((col + 1) * sizeof(char));
  }

  x11->pos_x = 0;
  x11->pos_y = 0;
  x11->topline = 0;
  x11->scroll_on = 0;
  if (x11->buff == NULL) {
    perror("malloc");
    return NULL;
  }

  return x11;
}

void load_font(X11_If *x11, char *fontname) {
  XftFont *xftfont;
  xftfont = XftFontOpenName(x11->display, x11->screen, fontname);
  if (!xftfont) {
    perror("Error while loading the specified font");
  }
  x11->xftfont = xftfont;

  // measuring font width and height
  XGlyphInfo extents;
  const char *text = "A";
  XftTextExtentsUtf8(x11->display, xftfont, (const FcChar8 *)text, strlen(text),
                     &extents);
  x11->font_w = extents.xOff;
  x11->font_h = xftfont->ascent + xftfont->descent;
}

void setup_xft(X11_If *x11) {
  XftDraw *xftdraw;
  XftColor *xftcolor = (XftColor *)malloc(sizeof(XftColor));
  // printf("w and s : %ld - %d \n", x11->window, x11->screen);
  xftdraw = XftDrawCreate(x11->display, x11->window,
                          DefaultVisual(x11->display, x11->screen),
                          DefaultColormap(x11->display, x11->screen));
  if (!xftdraw) {
    perror("Error creating XftDraw\n");
    return;
  }
  x11->xftdraw = xftdraw;
  x11->xftcolor = xftcolor;
  printf("w and s : %p - %d \n", x11->display, x11->screen);
  XftColorAllocName(x11->display, DefaultVisual(x11->display, x11->screen),
                    DefaultColormap(x11->display, x11->screen), "black",
                    x11->xftcolor);
}

void build_x11_interface(X11_If *x11) {

  if ((x11->font_h == -1) || (x11->font_w == -1))
    perror("A font has not been loaded");

  x11->w = x11->cell_x * x11->font_w;
  x11->h = x11->cell_y * x11->font_h;

  printf("%d, %d \n", x11->w, x11->h);
  Window window = XCreateSimpleWindow(
      x11->display, RootWindow(x11->display, x11->screen), 0, 0, x11->w, x11->h,
      1, BlackPixel(x11->display, x11->screen),
      WhitePixel(x11->display, x11->screen));

  x11->window = window;
  XStoreName(x11->display, x11->window, "temm 0.1.0");
  XSelectInput(x11->display, x11->window, ExposureMask | KeyPressMask);
  XMapWindow(x11->display, x11->window);

  XSync(x11->display, False);
}

int record_termsize_ioctl_pty(X11_If *x11, PTY *pty) {

  struct winsize wsz = {
      .ws_col = x11->cell_x,
      .ws_row = x11->cell_y,
  };

  if (ioctl(pty->master, TIOCSWINSZ, &wsz) == -1) {
    perror("ioctl(TIOCSWINSZ)");
    return 0;
  }

  return 1;
}

void handle_event(XKeyEvent *event, PTY *pty) {
  char buffer[32];
  KeySym ksym;
  int num = XLookupString(event, buffer, sizeof(buffer), &ksym, 0);
  for (int i = 0; i < num; i++)
    write(pty->master, &buffer[i], 1);
}
