#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "cleanup.h"
#include <X11/extensions/Xrender.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define dlog printf("%s\n", "dlog - render.c");
#define xposn                                                                  \
  printf("xposn(%d, %d, %d)\n", x11->pos_y, x11->pos_x, x11->topline);
#define rposn printf("xposn(%d, %d, %d)\n", rg->pos_y, rg->pos_x, rg->topline);

render_group *render_init(X11_If *x11) {
  render_group *rg = (render_group *)malloc(sizeof(render_group));
  rg->renbuf = (char **)malloc(sizeof(char *) * x11->cell_y);
  for (int i = 0; i < x11->cell_y; i++) {
    rg->renbuf[i] = (char *)malloc((x11->cell_x + 1) * sizeof(char));
  }
  rg->cell_y = x11->cell_y;
  rg->cell_x = x11->cell_x;
  rg->topline = 0;
  rg->scroll_on = 0;

  for (int i = 0; i < rg->cell_y; i++) {
    for (int j = 0; j < rg->cell_x; j++) {
      rg->renbuf[i][j] = ' ';
    }
    rg->renbuf[i][x11->cell_x] = '\0';
  }
  rg->pos_y = 0;
  rg->pos_x = 1;
  return rg;
}

void render_screen_scrollable(X11_If *x11) {
  // i != (start - 1) % n + 1; i = i % n + 1
  // for (int i = x11->topline; i != (x11->topline - 1) % x11->cell_y;
  //      i = i % x11->cell_y + 1) {
  char buf[1];
  int rectangle_posy = 0;
  // char buf2[x11->cell_x];
  for (int i = 0; i < x11->cell_y; i++) {
    int ind = (i + x11->topline) % x11->cell_y;
    for (int j = 0; j < x11->cell_x; j++) {
      buf[0] = x11->buff[ind][j];
      if (!iscntrl(buf[0])) {
        XftChar8 *convtText = (XftChar8 *)buf;
        XftDrawString8(x11->xftdraw, x11->xftcolor, x11->xftfont,
                       j * x11->font_w, i * x11->font_h + x11->xftfont->ascent,
                       convtText, 1);
      }
    }
    rectangle_posy = x11->scroll_on ? (x11->cell_y - 2) : x11->pos_y;
    XftDrawRect(x11->xftdraw, x11->xftcolor, x11->pos_x * x11->font_w,
                rectangle_posy * x11->font_h, x11->font_w, x11->font_h);
  }
  XSync(x11->display, False);
}

void render_screen_non_scrollable(X11_If *x11) {
  char buf[1];
  for (int i = 0; i < x11->cell_y; i++) {
    for (int j = 0; j < x11->cell_x; j++) {
      buf[0] = x11->buff[i][j];
      if (!iscntrl(buf[0])) {
        XftChar8 *convtText = (XftChar8 *)buf;
        XftDrawString8(x11->xftdraw, x11->xftcolor, x11->xftfont,
                       j * x11->font_w, i * x11->font_h + x11->xftfont->ascent,
                       convtText, 1);
      }
    }
  }

  XftDrawRect(x11->xftdraw, x11->xftcolor, x11->pos_x * x11->font_w,
              x11->pos_y * x11->font_h, x11->font_w, x11->font_h);
  XSync(x11->display, False);
}

void register_key_input(X11_If *x11, PTY *pty) {
  char buf[32];
  int i, input_buffer_len_recv;
  KeySym ksym;

  input_buffer_len_recv =
      XLookupString(&x11->event.xkey, buf, sizeof buf, &ksym, 0);
  if (ksym == XK_BackSpace) {
    if (x11->pos_x <= 2)
      return;
    printf("backspace detected 2 \n");
    // record_index = x11->pos_x;
    x11->g_backspace = 1;
    char delete_char = 0x7f;
    write(pty->master, &delete_char, 1);
    // XClearWindow(x11->display, x11->window);
    // x11_redraw(x11);
    return;
  }
  for (i = 0; i < input_buffer_len_recv; i++)
    write(pty->master, &buf[i], 1);
}

void draw_on_screen(X11_If *x11, PTY *pty, Atom wm_delete_window) {
  while (XPending(x11->display)) {
    XNextEvent(x11->display, &x11->event);
    switch (x11->event.type) {
    case Expose:
      XClearWindow(x11->display, x11->window);
      // render_screen_scrollable_x11buf(rg, x11);
      // x11_redraw(x11);
      // render_screen_non_scrollable(x11);
      render_screen_scrollable(x11);
      break;
    case KeyPress:
      register_key_input(x11, pty);
      // x11_redraw(x11);
      break;
    case ClientMessage:
      // cleanup
      if ((Atom)x11->event.xclient.data.l[0] == wm_delete_window) {
        // Handle the window close event (WM_DELETE_WINDOW)
        call_cleanup(x11, pty);
        exit(0);
      }
    }
  }
}

void render_shell_mainloop(X11_If *x11, PTY *pty, Atom wm_delete_window) {
  int maxfd;
  fd_set readable;
  char buf[8];
  int just_wrapped = 0; // bool
  maxfd = pty->master > x11->fd ? pty->master : x11->fd;

  // if (!rg->renbuf) {
  //   perror("Render Buffer not initialized - x11-buff - render.c");
  // }

  if (!x11->buff) {
    perror("X11 Buffer not initialized - x11-buff - render.c");
  }

  x11->pos_x = 1;

  while (1) {
    FD_ZERO(&readable);
    FD_SET(pty->master, &readable);
    FD_SET(x11->fd, &readable);

    if (select(maxfd + 1, &readable, NULL, NULL, NULL) == -1) {
      perror("select");
      return;
    }

    if (FD_ISSET(pty->master, &readable)) {
      if (read(pty->master, buf, 1) <= 0) {
        fprintf(stderr, "Nothing to read - render.c | read/sys \n");
        return;
      }
      if (buf[0] == '\r') {
        x11->pos_x = 0;
      } else {
        if (x11->g_backspace) {
          if (x11->wait_counter == 0) {
            x11->buff[x11->pos_y][--x11->pos_x] = 0;
          }
          if (x11->wait_counter == 2) {
            x11->g_backspace = 0;
            x11->wait_counter = 0;
          } else {
            x11->wait_counter++;
          }
        } else {
          if (buf[0] != '\n') {
            x11->buff[x11->pos_y][x11->pos_x++] = buf[0];
            if (x11->pos_x >= x11->cell_x - 1) {
              x11->pos_x = 0;
              x11->pos_y = (x11->pos_y + 1) % x11->cell_y;
              if (x11->scroll_on) {
                x11->topline = (x11->topline + 1) % (x11->cell_y);
                for (int i = 0; i < x11->cell_x; i++) {
                  x11->buff[x11->topline][i] = 0;
                }
              }
              just_wrapped = 1;
            } else
              just_wrapped = 0;
          } else if (!just_wrapped) {
            // x11->pos_y++;
            x11->pos_y = (x11->pos_y + 1) % x11->cell_y;
            if (x11->scroll_on) {
              x11->topline = (x11->topline + 1) % (x11->cell_y);
              for (int i = 0; i < x11->cell_x; i++) {
                x11->buff[x11->topline][i] = 0;
              }
            }
            // x11->topline = (x11->topline + 1) % (x11->cell_y);
            just_wrapped = 0;
          }

          if ((x11->pos_y >= x11->cell_y - 1)) {
            if (!x11->scroll_on) {
              x11->scroll_on = 1;
              x11->topline = (x11->topline + 1) % (x11->cell_y);
              for (int i = 0; i < x11->cell_x; i++) {
                x11->buff[x11->topline][i] = 0;
              }
            }
          }
        }
      }
      XClearWindow(x11->display, x11->window);
      // render_screen_scrollable_x11buf(rg, x11);
      // render_screen_non_scrollable(x11);
      render_screen_scrollable(x11);
    }

    if (FD_ISSET(x11->fd, &readable)) {
      draw_on_screen(x11, pty, wm_delete_window);
    }
  }
}
