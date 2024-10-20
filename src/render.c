#include "../include/render.h"
#include <X11/Xlib.h>
// #include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>
#include <stdio.h>

#define dlog printf("%s\n", "dlog - render.c");

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

void render_screen_alt(X11_If *x11) {
  for (int i = 0; i < x11->cell_y; i++) {
    char actual_line[x11->cell_x];
    for (int j = 0; j < x11->cell_x; j++) {
      actual_line[j] = x11->buff[i][j];
    }
    XftChar8 *convtText = (XftChar8 *)(actual_line);
    if ((!x11->xftfont) || (!x11->xftdraw) || (!convtText)) {
      perror("Error with xft font or drawable - render.c");
    }
    XftDrawString8(x11->xftdraw, x11->xftcolor, x11->xftfont, 0,
                   20 + i * x11->font_h, convtText, x11->cell_x);
  }
}

int scroll_one(X11_If *x11) { return 1; }
int can_backtrack(X11_If *x11) { return 1; }

void render_mainloop(X11_If *x11, PTY *pty) {
  // char lines[x11->cell_y][x11->cell_x];
  if (!x11->buff) {
    perror("Buffer not initialized - x11-buff - render.c");
  }
  for (int i = 0; i < x11->cell_y; i++) {
    for (int j = 0; j < x11->cell_x; j++) {
      x11->buff[i][j] = ' ';
    }
    x11->buff[i][x11->cell_x] = '\0';
  }
  // int line_number = 0;
  // int line_input_len = 0;

  x11->pos_x = 1;
  printf("posn : %d, %d \n", x11->pos_x, x11->pos_y);
  while (1) {

    printf("posn : %d, %d | %d \n", x11->pos_y, x11->pos_x, x11->cell_x);
    XNextEvent(x11->display, &x11->event);
    // XFillRectangle(display, window, gc, 20, 20, 10, 10);
    if (x11->event.type == Expose) {
      // XftDrawRect(XftDraw *draw, const XftColor *color, int x, int y,
      // unsigned int width, unsigned int height)
      // (x11->display, x11->window, x11->gc, 20, 20, 10, 10);
      // const XftChar8 *convtText = (XftChar8 *)input_text;
      // XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, 20, 50, convtText,
      //                   input_text_length);
      // TODO: Render here
      //
      // RenderNow(&TextRenderer, text_x, text_y);

      // render_screen(x11, x11->cell_y, x11->cell_x, lines);
      render_screen_alt(x11);

      // XDrawString(display, window, gc, 20, 50, input_text,
      // input_text_length);
    }

    if (x11->event.type == KeyPress) {
      // int buffer_limit = 32;
      // char buffer[buffer_limit];
      char buffer[32];
      KeySym ksym;

      int input_buffer_len_recv = XLookupString(
          &x11->event.xkey, buffer, sizeof(buffer) - 1, &ksym, NULL);
      buffer[input_buffer_len_recv] = '\0';
      if (input_buffer_len_recv > 0 &&
          (x11->pos_x + input_buffer_len_recv < x11->cell_x)) {
        if (ksym == XK_Escape) {
          printf("Exiting Terminal");
          break;
        }

        if (ksym == XK_BackSpace) {
          // printf("Backspace detected \n");
          // Backspace handling innit
          if (x11->pos_x > 1) {
            x11->buff[x11->pos_y][--x11->pos_x] =
                ' '; // Replace the last character by a space (empty char kinda)
          }
          XClearWindow(x11->display, x11->window);
          // XFillRectangle(x11->display, x11->window, x11->gc, 20, 20, 10, 10);
          // TODO: Render here
          // render_screen(x11, x11->cell_y, x11->cell_x, lines);
          render_screen_alt(x11);
          //
          // RenderNow(&TextRenderer, text_x, text_y);
          // XftChar8 *convtText = (XftChar8 *)input_text;
          // XftDrawString8(xftdraw, &xftcolor, xftfont, 20, 50, convtText,
          //                input_text_length);
          // XDrawString(display, window, gc, 20, 50, input_text,
          //             input_text_length);
          continue;
        }
        if (ksym == XK_Return) {
          // NOTE: we need to specify what rules to follow when
          // typing a command and then also moving on to the next line

          // XClearWindow(x11->display, x11->window);
          // // TODO: Render here
          //
          // render_screen(x11, lines[line_number], line_input_len);
          // continue;
        }

        if (x11->pos_x >= x11->cell_x - 2) {
          // if (x11->pos_y >= x11->cell_y) {
          // }
          x11->pos_x = 1;
          dlog;
          x11->buff[++x11->pos_y][x11->pos_x] = buffer[0];
        } else {
          x11->buff[x11->pos_y][x11->pos_x++] = buffer[0];
        }
        XClearWindow(x11->display, x11->window);
        // XFillRectangle(x11->display, x11->window, x11->gc, 20, 20, 10, 10);

        // TODO: Render here
        //
        render_screen_alt(x11);
        // render_screen(x11, x11->cell_y, x11->cell_x, lines);
        //
        // printf("Key pressed: '%s' (keycode: %d, keysym: %lu, inptexlen :
        // %d)\n",
        //        lines[x11->pos_x], x11->event.xkey.keycode, ksym,
        //        line_input_len);

        // if (x11->pos_y >= x11->cell_y) {
        //   // write code for scrolling
        //   if (scroll_one(x11) == 1) {
        //     x11->pos_y++;
        //     x11->pos_y %= x11->cell_y;
        //   } else {
        //     perror("scroller");
        //   }
        // }
      }
    }
  }
}
