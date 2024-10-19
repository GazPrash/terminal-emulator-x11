#pragma once
#include "pty_pt.h"
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xft/XftCompat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// X11 interface and config
typedef struct {
  int fd;
  Display *display;
  int screen;
  Window window;
  int timeout;

  GC gc;
  // width and height of the terminal | ideal 800 * 600
  int w, h;
  // font's width and height
  int font_w, font_h;

  char *buff;
  // total cell in x dirns and cells in y-dirns
  int cell_x, cell_y;
  // cursor posn:
  int pos_x, pos_y;
  int *buff_len;

  XEvent event;

} X11_If;

X11_If *build_x11_interface(XftFont *xftr, int w, int h, int timeout);
void handle_event(XKeyEvent *event, PTY *pty);
