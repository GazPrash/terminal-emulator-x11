#include "x11_if.h"
#include <X11/Xft/Xft.h>

typedef struct {
  XftDraw *xftdraw;
  XftFont *xftfont;
  XftColor xftcolor;

  char **lines;
  int line_input_max_limit;

} xft_renderer;

typedef struct {
  char **lines;
  int line_input_max_limit;
  int line_max_limit;

} text_group;

xft_renderer *setup(X11_If *x11, char *fontname);
void render_mainloop(xft_renderer *xftr, X11_If *x11);

int can_backtrack(X11_If *x11);
