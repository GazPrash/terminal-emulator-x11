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
  XStoreName(x11->display, x11->window, "temm 0.01");
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

// int run_shell(X11_If *x11, PTY *pty) {
//   int i, maxfd;
//   fd_set readable;
//   XEvent ev;
//   char buf[1];
//   int just_wrapped = 0; // bool
//   maxfd = pty->master > x11->fd ? pty->master : x11->fd;
//
//   while (1) {
//     FD_ZERO(&readable);
//     FD_SET(pty->master, &readable);
//     FD_SET(x11->fd, &readable);
//
//     if (select(maxfd + 1, &readable, NULL, NULL, NULL) == -1) {
//       perror("select");
//       return 1;
//     }
//
//     if (FD_ISSET(pty->master, &readable)) {
//       if (read(pty->master, buf, 1) <= 0) {
//         /* This is not necessarily an error but also happens
//          * when the child exits normally. */
//         fprintf(stderr, "Nothing to read from child: ");
//         perror(NULL);
//         return 1;
//       }
//
//       if (buf[0] == '\r') {
//         /* "Carriage returns" are probably the most simple
//          * "terminal command": They just make the cursor jump
//          * back to the very first column. */
//         x11->pos_x = 0;
//       } else {
//         if (buf[0] != '\n') {
//           /* If this is a regular byte, store it and advance
//            * the cursor one cell "to the right". This might
//            * actually wrap to the next line, see below. */
//           // x11->buff[x11->pos_y * x11->cell_x + x11->pos_x] = buf[0];
//           x11->buff[x11->pos_y][x11->pos_x] = buf[0];
//           x11->pos_x++;
//
//           if (x11->pos_x >= x11->cell_x) {
//             x11->pos_x = 0;
//             x11->pos_y++;
//             just_wrapped = 1;
//           } else
//             just_wrapped = 0;
//         } else if (!just_wrapped) {
//           /* We read a newline and we did *not* implicitly
//            * wrap to the next line with the last byte we read.
//            * This means we must *now* advance to the next
//            * line.
//            *
//            * This is the same behaviour that most other
//            * terminals have: If you print a full line and then
//            * a newline, they "ignore" that newline. (Just
//            * think about it: A full line of text could always
//            * wrap to the next line implicitly, so that
//            * additional newline could cause the cursor to jump
//            * to the next line *again*.) */
//           x11->pos_y++;
//           just_wrapped = 0;
//         }
//
//         /* We now check if "the next line" is actually outside
//          * of the buffer. If it is, we shift the entire content
//          * one line up and then stay in the very last line.
//          *
//          * After the memmove(), the last line still has the old
//          * content. We must clear it. */
//         if (x11->pos_y >= x11->cell_y) {
//           memmove(x11->buff, &x11->buff[x11->cell_x],
//                   x11->cell_x * (x11->cell_y - 1));
//           x11->pos_y = x11->cell_y - 1;
//
//           for (i = 0; i < x11->cell_x; i++)
//             x11->buff[x11->pos_y * x11->cell_x + i] = 0;
//         }
//       }
//
//       // x11_redraw(x11);
//     }
//
//     if (FD_ISSET(x11->fd, &readable)) {
//       while (XPending(x11->dpy)) {
//         XNextEvent(x11->dpy, &ev);
//         switch (ev.type) {
//         case Expose:
//           x11_redraw(x11);
//           break;
//         case KeyPress:
//           x11_key(&ev.xkey, pty);
//           break;
//         }
//       }
//     }
//   }
// }
