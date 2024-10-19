#include "../include/render.h"
#include <X11/Xlib.h>
#include <stdio.h>

xft_renderer *setup(X11_If *x11, char *fontname) {
  // TODO : Instead of heap allocation just return the struct copy
  xft_renderer *xftr = (xft_renderer *)malloc(sizeof(xft_renderer) * 1);
  XftDraw *xftdraw;
  XftFont *xftfont;
  XftColor xftcolor;
  // char* fontname = "Fragment Mono:size=14";
  xftfont = XftFontOpenName(x11->display, x11->screen, fontname);
  if (!xftfont) {
    fprintf(stderr, "Error while loading fonts");
    return NULL;
  }
  xftdraw = XftDrawCreate(x11->display, x11->window,
                          DefaultVisual(x11->display, x11->screen),
                          DefaultColormap(x11->display, x11->screen));

  xftr->xftdraw = xftdraw;
  xftr->xftfont = xftfont;
  xftr->xftcolor = xftcolor;

  XftColorAllocName(x11->display, DefaultVisual(x11->display, x11->screen),
                    DefaultColormap(x11->display, x11->screen), "black",
                    &(xftr->xftcolor));
  return xftr;
}

void text_render(X11_If *x11) {}
int scroll_one(X11_If *x11);

void render_mainloop(xft_renderer *xftr, X11_If *x11) {
  char lines[x11->cell_y][x11->cell_x];
  int line_number = 0;
  int line_input_len = 0;
  while (1) {
    XNextEvent(x11->display, &x11->event);
    // XFillRectangle(display, window, gc, 20, 20, 10, 10);
    if (x11->event.type == Expose) {
      // XftDrawRect(XftDraw *draw, const XftColor *color, int x, int y,
      // unsigned int width, unsigned int height)
      XFillRectangle(x11->display, x11->window, x11->gc, 20, 20, 10, 10);
      // const XftChar8 *convtText = (XftChar8 *)input_text;
      // XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, 20, 50, convtText,
      //                   input_text_length);
      // TODO: Render here
      //
      // RenderNow(&TextRenderer, text_x, text_y);
      text_render(x11);
      // XDrawString(display, window, gc, 20, 50, input_text,
      // input_text_length);
    }
    if (x11->event.type == KeyPress) {
      int buffer_limit = 32;
      char buffer[buffer_limit];
      KeySym ksym;
      int input_buffer_len_recv = XLookupString(
          &x11->event.xkey, buffer, sizeof(buffer) - 1, &ksym, NULL);
      buffer[input_buffer_len_recv] = '\0';

      if (input_buffer_len_recv > 0 &&
          (line_input_len + input_buffer_len_recv < x11->cell_x)) {
        if (ksym == XK_Escape) {
          printf("Exiting Terminal");
          break;
        }
        if (ksym == XK_BackSpace && can_backtrack(x11) > 0) {
          // printf("Backspace detected \n");
          if (line_input_len > 0) {
            lines[line_number][--line_input_len] =
                '\0'; // Remove the last character
          }
          XClearWindow(x11->display, x11->window);
          XFillRectangle(x11->display, x11->window, x11->gc, 20, 20, 10, 10);

          // TODO: Render here
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
          // input_text_length += input_buffer_len_recv;
          // strcat(input_text, " ");
          XClearWindow(x11->display, x11->window);
          // TODO: Render here
          //
          // RenderNow(&TextRenderer, text_x, text_y);
          continue;
        }

        // WARN: strcat is not safe, but we have a if-check here in place
        // Unsafe here
        line_input_len += input_buffer_len_recv;
        strcat(lines[line_number], buffer);
        XClearWindow(x11->display, x11->window);
        XFillRectangle(x11->display, x11->window, x11->gc, 20, 20, 10, 10);

        // TODO: Render here
        //
        // RenderNow(&TextRenderer, text_x, text_y);
        //
        // XftChar8 *convtText = (XftChar8 *)input_text;
        // XftDrawString8(xftdraw, &xftcolor, xftfont, 20, 50, convtText,
        //                input_text_length);
        // XDrawString(display, window, gc, 20, 50, input_text,
        // input_text_length);
        //
        // Backspace handling innit
        printf("Key pressed: '%s' (keycode: %d, keysym: %lu, inptexlen : %d)\n",
               lines[line_number], x11->event.xkey.keycode, ksym,
               line_input_len);

        // printf("%s", buffer);
        // for (int i = 0; i < BUFFER_LEN; i++) {
        //   printf("TEXT %d : %c", i, buffer[i] - '0');
        //   printf("\n");
        // }
      } else if (line_input_len + input_buffer_len_recv == x11->cell_y) {
        if (line_number >= x11->cell_y) {
          // write code for scrolling
          if (scroll_one(x11) == 1) {
            line_number++;
            line_number %= x11->cell_y;
          } else {
            perror("scroller");
          }
        }
      }
      // break;
    }
  }
}
