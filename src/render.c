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

void render_screen(X11_If *x11, int total_lines, int total_line_input_len,
                   char lines[total_lines][total_line_input_len]) {
  for (int i = 0; i < x11->cell_y; i++) {
    char actual_line[x11->cell_x];
    for (int j = 0; j < x11->cell_x; j++) {
      actual_line[j] = lines[i][j];
    }
    XftChar8 *convtText = (XftChar8 *)(actual_line);
    if ((!x11->xftfont) || (!x11->xftdraw) || (!convtText)) {
      perror("Error with xft font or drawable - render.c");
    }
    printf("cellx : %d \n", x11->cell_x);
    XftDrawString8(x11->xftdraw, x11->xftcolor, x11->xftfont, 0,
                   20 + i * x11->font_h, convtText, x11->cell_x);
  }
  // XSync(x11->display, False);
}

void render_screen_alt(render_group *rg, X11_If *x11) {
  // i != (start - 1) % n + 1; i = i % n + 1
  // for (int i = x11->topline; i != (x11->topline - 1) % x11->cell_y;
  //      i = i % x11->cell_y + 1) {
  for (int i = 0; i < rg->cell_y; i++) {
    char actual_line[x11->cell_x];
    for (int j = 0; j < x11->cell_x; j++) {
      actual_line[j] = rg->renbuf[i][j];
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

void empty_line(char *line, int len) {
  for (int i = 0; i < len; i++) {
    line[i] = ' ';
  }
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

void render_screen_scrollable_x11buf(render_group *rg, X11_If *x11) {
  // i != (start - 1) % n + 1; i = i % n + 1
  // for (int i = x11->topline; i != (x11->topline - 1) % x11->cell_y;
  //      i = i % x11->cell_y + 1) {
  for (int i = 0; i < x11->cell_y; i++) {
    int ind = (i + x11->topline) % x11->cell_y;
    // printf("topline : %d \n", rg->topline);
    // printf("i : %d \n", ind);
    // sleep(1);
    // if (i == 14)
    //   rg->topline++;
    char actual_line[x11->cell_x];
    for (int j = 0; j < x11->cell_x; j++) {
      char athand;
      athand = x11->buff[i][j];
      if (!iscntrl(athand)) {
        actual_line[j] = x11->buff[i][j];
      }
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

void x11_redraw(X11_If *x11) {
  int x, y;
  char buf[1];

  // XSetForeground(x11->dpy, x11->termgc, x11->col_bg);
  XftDrawRect(x11->xftdraw, x11->xftcolor, 0, 0, x11->font_w, x11->font_h);
  // XSetForeground(x11->dpy, x11->termgc, x11->col_fg);

  for (y = 0; y < x11->cell_y; y++) {
    for (x = 0; x < x11->cell_x; x++) {
      buf[0] = x11->buff[y][x];
      if (!iscntrl(buf[0])) {
        // XDrawString(x11->display, x11->termwin, x11->termgc, x *
        // x11->font_width,
        //             y * x11->font_height + x11->xfont->ascent, buf, 1);

        // printf("hello harry");
        // printf("buf : %s \n", buf);
        XftChar8 *convtText = (XftChar8 *)buf;
        XftDrawString8(x11->xftdraw, x11->xftcolor, x11->xftfont,
                       x * x11->font_w, y * x11->font_h + x11->xftfont->ascent,
                       convtText, 1);
      }
    }
  }

  // XSetForeground(x11->dpy, x11->termgc, x11->col_fg);
  // XFillRectangle(x11->dpy, x11->termwin, x11->termgc,
  //                x11->buf_x * x11->font_width, x11->buf_y * x11->font_height,
  //                x11->font_width, x11->font_height);

  XftDrawRect(x11->xftdraw, x11->xftcolor, 0, 0, x11->font_w, x11->font_h);
  XSync(x11->display, False);
}

int scroll_one(X11_If *x11) {
  //
  // x11->topline++;
  return 1;
}
int can_backtrack(X11_If *x11) { return 1; }

void x11_key(XKeyEvent *ev, PTY *pty) {
  char buf[32];
  int i, num;
  KeySym ksym;

  num = XLookupString(ev, buf, sizeof buf, &ksym, 0);
  for (i = 0; i < num; i++)
    write(pty->master, &buf[i], 1);
}

void xevent_handler(render_group *rg, X11_If *x11, PTY *pty,
                    Atom wm_delete_window) {
  // char **rbuf = rg->renbuf;

  while (XPending(x11->display)) {
    XNextEvent(x11->display, &x11->event);
    if (x11->event.type == Expose) {
      XClearWindow(x11->display, x11->window);
      render_screen_scrollable_x11buf(rg, x11);
      break;
    }
    if (x11->event.type == KeyPress) {

      char buffer[32];
      KeySym ksym;

      int input_buffer_len_recv = XLookupString(
          &x11->event.xkey, buffer, sizeof(buffer) - 1, &ksym, NULL);
      buffer[input_buffer_len_recv] = '\0';
      for (int i = 0; i < input_buffer_len_recv; i++) {
        write(pty->master, &buffer[i], 1);
      }
      if (input_buffer_len_recv > 0 &&
          (x11->pos_x + input_buffer_len_recv < x11->cell_x)) {
        if (ksym == XK_Escape) {
          printf("Exiting Terminal");
          return;
        }
        if (ksym == XK_BackSpace) {
          // printf("Backspace detected \n");
          // Backspace handling innit
          if (x11->pos_x > 1) {
            x11->buff[x11->pos_y][--x11->pos_x] =
                ' '; // Replace the last character by a space (empty char
                     // kinda)
          }
          XClearWindow(x11->display, x11->window);
          // render_screen_alt(rg, x11);
          render_screen_scrollable(rg, x11);
          break;
          // continue;
        }
        if (ksym == XK_Return) {
          // NOTE: we need to specify what rules to follow when
          // typing a command and then also moving on to the next line
          // // TODO: Render here
          break;
        }

        if (x11->pos_x >= (x11->cell_x - 2 * 1)) {
          if (x11->pos_y >= (x11->cell_y - 1) || (x11->scroll_on)) {
            // new line and scroll
            x11->scroll_on = 1;
            empty_line(x11->buff[x11->topline], x11->cell_x);
            x11->topline = (x11->topline + 1) % x11->cell_y;
          }
          x11->pos_x = 1;
          x11->pos_y = (x11->pos_y + 1) % x11->cell_y;
          // x11->buff[x11->pos_y][x11->pos_x++] = buffer[0];
        } else {
          // x11->buff[x11->pos_y][x11->pos_x++] = buffer[0];
        }
        XClearWindow(x11->display, x11->window);
        // render_screen_alt(rg, x11);
        render_screen_scrollable(rg, x11);
      }
      break;
    }
    if (x11->event.type == ClientMessage) {
      if ((Atom)x11->event.xclient.data.l[0] == wm_delete_window) {
        // Handle the window close event (WM_DELETE_WINDOW)
        printf("Window close requested, performing cleanup...\n");
        //
        // for (int i = 0; i < x11->cell_y; i++) {
        //   char *cstr = x11->buff[i];
        //   puts(cstr);
        // }
        XftColorFree(x11->display, DefaultVisual(x11->display, x11->screen),
                     DefaultColormap(x11->display, x11->screen), x11->xftcolor);
        XftFontClose(x11->display, x11->xftfont);
        // XUnloadFont(display, font);
        XCloseDisplay(x11->display);
        XftDrawDestroy(x11->xftdraw);
        free(x11->buff);
        free(x11->xftcolor);
        free(x11);
        exit(0);
      }
    }
  }
}

void render_shell_mainloop(render_group *rg, X11_If *x11, PTY *pty,
                           Atom wm_delete_window) {
  // char lines[x11->cell_y][x11->cell_x];
  int i, maxfd;
  fd_set readable;
  char buf[8];
  int just_wrapped = 0; // bool
  maxfd = pty->master > x11->fd ? pty->master : x11->fd;
  // printf("maxfd : %d", maxfd);

  if (!rg->renbuf) {
    perror("Render Buffer not initialized - x11-buff - render.c");
  }

  if (!x11->buff) {
    perror("X11 Buffer not initialized - x11-buff - render.c");
  }

  // for (int i = 0; i < x11->cell_y; i++) {
  //   // ini
  //   for (int j = 0; j < x11->cell_x; j++) {
  //     x11->buff[i][j] = ' ';
  //   }
  //   x11->buff[i][x11->cell_x] = '\0';
  // }

  x11->pos_x = 1;
  int temp_iter = 0;
  int wait_interupt = 0;
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
        /* This is not necessarily an error but also happens
         * when the child exits normally. */
        fprintf(stderr, "Nothing to read from child: ");
        perror(NULL);
        return;
      }

      xposn;
      if (wait_interupt) {
        wait_interupt = 0;
        if (buf[0] == 'H') {
          printf("backspace detected \n");
          if (x11->pos_x > 2) {
            x11->buff[x11->pos_y][--x11->pos_x] = 0;
            XClearWindow(x11->display, x11->window);
            x11_redraw(x11);
          }
          continue;
        } else if (buf[0] == 'C') {
          printf("keyboard interupt \n");
          continue;
        }
      }
      if (buf[0] == '^') {
        wait_interupt = 1;
        continue;
      }
      if (buf[0] == '\r') {
        /* "Carriage returns" are probably the most simple
         * "terminal command": They just make the cursor jump
         * back to the very first column. */
        x11->pos_x = 0;
      } else {
        if (buf[0] != '\n') {
          /* If this is a regular byte, store it and advance
           * the cursor one cell "to the right". This might
           * actually wrap to the next line, see below. */
          // x11->buff[x11->pos_y * x11->cell_x + x11->pos_x] = buf[0];
          x11->buff[x11->pos_y][x11->pos_x++] = buf[0];
          if (x11->pos_x >= x11->cell_x - 1) {
            x11->pos_x = 0;
            x11->pos_y++;
            just_wrapped = 1;
          } else
            just_wrapped = 0;
        } else if (!just_wrapped) {
          /* We read a newline and we did *not* implicitly
           * wrap to the next line with the last byte we read.
           * This means we must *now* advance to the next
           * line.
           *
           * This is the same behaviour that most other
           * terminals have: If you print a full line and then
           * a newline, they "ignore" that newline. (Just
           * think about it: A full line of text could always
           * wrap to the next line implicitly, so that
           * additional newline could cause the cursor to jump
           * to the next line *again*.) */
          x11->pos_y++;
          just_wrapped = 0;
        }

        /* We now check if "the next line" is actually outside
         * of the buffer. If it is, we shift the entire content
         * one line up and then stay in the very last line.
         *
         * After the memmove(), the last line still has the old
         * content. We must clear it. */
        if (x11->pos_y >= x11->cell_y - 1) {
          // memmove(x11->buff, &x11->buff[x11->cell_x],
          //         x11->cell_x * (x11->cell_y - 1));
          x11->pos_y = x11->cell_y - 1;
          for (i = 0; i < x11->cell_x; i++) {
            x11->buff[x11->pos_y][i] = 0;
          }
        }
      }

      XClearWindow(x11->display, x11->window);
      // render_screen_scrollable_x11buf(rg, x11);
      x11_redraw(x11);
    }

    if (FD_ISSET(x11->fd, &readable)) {
      while (XPending(x11->display)) {
        XNextEvent(x11->display, &x11->event);
        switch (x11->event.type) {
        case Expose:
          XClearWindow(x11->display, x11->window);
          // render_screen_scrollable_x11buf(rg, x11);
          x11_redraw(x11);
          break;
        case KeyPress:
          x11_key(&x11->event.xkey, pty);
          // x11_redraw(x11);
          break;
        case ClientMessage:
          if ((Atom)x11->event.xclient.data.l[0] == wm_delete_window) {
            // Handle the window close event (WM_DELETE_WINDOW)
            printf("Window close requested, performing cleanup...\n");
            //
            // for (int i = 0; i < x11->cell_y; i++) {
            //   char *cstr = x11->buff[i];
            //   puts(cstr);
            // }
            XftColorFree(x11->display, DefaultVisual(x11->display, x11->screen),
                         DefaultColormap(x11->display, x11->screen),
                         x11->xftcolor);
            XftFontClose(x11->display, x11->xftfont);
            // XUnloadFont(display, font);
            XCloseDisplay(x11->display);
            XftDrawDestroy(x11->xftdraw);
            free(x11->buff);
            free(x11->xftcolor);
            free(x11);
            exit(0);
          }
        }
      }
    }

    // // if (x11->pos_y >= (x11->cell_y))
    // //   return;
    //
    // FD_ZERO(&readable);
    // FD_SET(pty->master, &readable);
    // FD_SET(x11->fd, &readable);
    // if (select(maxfd + 1, &readable, NULL, NULL, NULL) == -1) {
    //   perror("select");
    //   return;
    // }
    //
    // // printf("posn : %d, %d | %d \n", rg->pos_y, rg->pos_x, rg->cell_x);
    //
    // if (FD_ISSET(pty->master, &readable)) {
    //   if (read(pty->master, buf, 1) <= 0) {
    //     /* This is not necessarily an error but also happens
    //      * when the child exits normally. */
    //     fprintf(stderr, "Nothing to read from child: ");
    //     perror(NULL);
    //     return;
    //   }
    //
    //   if (buf[0] == '\r') {
    //     /* "Carriage returns" are probably the most simple
    //      * "terminal command": They just make the cursor jump
    //      * back to the very first column. */
    //     x11->pos_x = 0;
    //   } else {
    //     // printf("sopmething moves");
    //     if (buf[0] != '\n') {
    //       /* If this is a regular byte, store it and advance
    //        * the cursor one cell "to the right". This might
    //        * actually wrap to the next line, see below. */
    //       // x11->buff[x11->pos_y * x11->cell_x + x11->pos_x] = buf[0];
    //       //   ORIGINALL HEREEHEHEHREHEHHE
    //       //   x11->buff[x11->pos_y][x11->pos_x] = buf[0];
    //       //   x11->pos_x++;
    //       //
    //       //   if (x11->pos_x >= x11->cell_x) {
    //       //     x11->pos_x = 0;
    //       //     x11->pos_y++;
    //       //     just_wrapped = 1;
    //       //   } else
    //       //     just_wrapped = 0;
    //       // } else if (!just_wrapped) {
    //       //   x11->pos_y++;
    //       //   just_wrapped = 0;
    //       // }
    //
    //       //   ORIGINALL HEREEHEHEHREHEHHE
    //
    //       xposn;
    //       if (x11->pos_x >= (x11->cell_x - 2 * 1)) {
    //         if (x11->pos_y >= (x11->cell_y - 1) || (x11->scroll_on)) {
    //           // new line and scroll
    //           x11->scroll_on = 1;
    //           empty_line(x11->buff[x11->topline], x11->cell_x);
    //           x11->topline = (x11->topline + 1) % x11->cell_y;
    //         }
    //         x11->pos_x = 1;
    //         x11->pos_y = (x11->pos_y + 1) % x11->cell_y;
    //         x11->buff[x11->pos_y][x11->pos_x++] = buf[0];
    //       } else {
    //         x11->buff[x11->pos_y][x11->pos_x++] = buf[0];
    //       }
    //       // XClearWindow(x11->display, x11->window);
    //       // render_screen_scrollable_x11buf(rg, x11);
    //     }
    //
    //     /* We now check if "the next line" is actually outside
    //      * of the buffer. If it is, we shift the entire content
    //      * one line up and then stay in the very last line.
    //      *
    //      * After the memmove(), the last line still has the old
    //      * content. We must clear it. */
    //     // if (x11->pos_y >= x11->cell_y) {
    //     //   for (int i = 0; i < x11->cell_y; i++) {
    //     //     for (int j = 0; j < x11->cell_x; j++) {
    //     //
    //     //       x11->buff[i][j] = ' ';
    //     //     }
    //     //   }
    //     //   x11->pos_x = 0;
    //     //   x11->pos_y = 0;
    //     //   // if (x11->topline >= x11->cell_y) {
    //     //   //   x11->topline = 0;
    //     //   //   x11->pos_y = 0;
    //     //   //   continue;
    //     //   // }
    //     //   // x11->topline++;
    //     // }
    //   }
    //   // x11_redraw(x11);
    //
    //   XClearWindow(x11->display, x11->window);
    //   render_screen_scrollable_x11buf(rg, x11);
    // temp_iter++;
  }

  // if (FD_ISSET(x11->fd, &readable)) {
  //   xevent_handler(rg, x11, pty, wm_delete_window);
  // } else {
  // }
  // sleep(100);
}
