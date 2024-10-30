#include "../include/render.h"
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xft/XftCompat.h>
#include <X11/Xlib.h>
// #include <X11/Xft/Xft.h>
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

void render_screen_scrollable(render_group *rg, X11_If *x11) {
  // i != (start - 1) % n + 1; i = i % n + 1
  // for (int i = x11->topline; i != (x11->topline - 1) % x11->cell_y;
  //      i = i % x11->cell_y + 1) {
  for (int i = 0; i < rg->cell_y; i++) {
    int ind = (i + rg->topline) % rg->cell_y;
    // printf("topline : %d \n", rg->topline);
    // printf("i : %d \n", ind);
    // sleep(1);
    // if (i == 14)
    //   rg->topline++;
    char actual_line[rg->cell_x];
    for (int j = 0; j < rg->cell_x; j++) {
      actual_line[j] = rg->renbuf[ind][j];
    }
    XftChar8 *convtText = (XftChar8 *)(actual_line);
    if ((!x11->xftfont) || (!x11->xftdraw) || (!convtText)) {
      perror("Error with xft font or drawable - render.c");
    }
    XftDrawString8(x11->xftdraw, x11->xftcolor, x11->xftfont, 0,
                   20 + i * x11->font_h, convtText, rg->cell_x);
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
      render_screen_non_scrollable(x11);
      break;
    case KeyPress:
      register_key_input(x11, pty);
      // x11_redraw(x11);
      break;
    case ClientMessage:
      // cleanup
      if ((Atom)x11->event.xclient.data.l[0] == wm_delete_window) {
        // Handle the window close event (WM_DELETE_WINDOW)
        printf("Window close requested, performing cleanup...\n");
        XftColorFree(x11->display, DefaultVisual(x11->display, x11->screen),
                     DefaultColormap(x11->display, x11->screen), x11->xftcolor);
        XftFontClose(x11->display, x11->xftfont);
        // XUnloadFont(display, font);
        XCloseDisplay(x11->display);
        XftDrawDestroy(x11->xftdraw);
        free(x11->buff);
        free(x11->xftcolor);
        free(x11);
        free(pty);
        exit(0);
      }
    }
  }
}

// TODO: SCROLLABLES :::
//
//         if (x11->pos_x >= (x11->cell_x - 2 * 1)) {
//           if (x11->pos_y >= (x11->cell_y - 1) || (x11->scroll_on)) {
//             // new line and scroll
//             x11->scroll_on = 1;
//             empty_line(x11->buff[x11->topline], x11->cell_x);
//             x11->topline = (x11->topline + 1) % x11->cell_y;
//           }
//           x11->pos_x = 1;
//           x11->pos_y = (x11->pos_y + 1) % x11->cell_y;
//           // x11->buff[x11->pos_y][x11->pos_x++] = buffer[0];
//         } else {
//           // x11->buff[x11->pos_y][x11->pos_x++] = buffer[0];
//         }
//         XClearWindow(x11->display, x11->window);
//         // render_screen_alt(rg, x11);
//         render_screen_scrollable(rg, x11);

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
              x11->pos_y++;
              just_wrapped = 1;
            } else
              just_wrapped = 0;
          } else if (!just_wrapped) {
            x11->pos_y++;
            just_wrapped = 0;
          }

          if (x11->pos_y >= x11->cell_y - 1) {
            x11->pos_y = x11->cell_y - 1;
            for (int i = 0; i < x11->cell_x; i++) {
              x11->buff[x11->pos_y][i] = 0;
            }
          }
        }
      }
      XClearWindow(x11->display, x11->window);
      // render_screen_scrollable_x11buf(rg, x11);
      render_screen_non_scrollable(x11);
    }

    if (FD_ISSET(x11->fd, &readable)) {
      draw_on_screen(x11, pty, wm_delete_window);
    }
  }
}
