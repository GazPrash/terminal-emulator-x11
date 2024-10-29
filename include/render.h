#include "x11_if.h"
#include <X11/Xft/Xft.h>

/*typedef struct {*/
/*  XftDraw *xftdraw;*/
/*  XftFont *xftfont;*/
/*  XftColor xftcolor;*/
/**/
/*  char **lines;*/
/*  int line_input_max_limit;*/
/**/
/*} xft_renderer;*/

/*typedef struct {*/
/*  char **lines;*/
/*  int line_input_max_limit;*/
/*  int line_max_limit;*/
/**/
/*} text_group;*/

/*xft_renderer *setup(X11_If *x11, char *fontname);*/

typedef struct {
  char **renbuf;
  int cell_x, cell_y;
  // render posn:
  int pos_x, pos_y;
  int scroll_on;
  int topline;

} render_group;

render_group *render_init(X11_If *x11);
void render_shell_mainloop(render_group *rg, X11_If *x11, PTY *pty,
                           Atom wm_delete_window);
int can_backtrack(X11_If *x11);

/*void render_screen(X11_If *x11, int total_lines, int total_line_input_len,*/
/*                   char lines[total_lines][total_line_input_len]);*/
/*void render_screen_alt(render_group *rg, X11_If *x11);*/
void render_screen_scrollable(render_group *rg, X11_If *x11);
void render_screen_non_scrollable(X11_If *x11);
/*void x11_redraw(X11_If *x11);*/

void register_key_input(X11_If *x11, PTY *pty);
int scroll_one(X11_If *x11);
void xevent_handler(render_group *rg, X11_If *x11, PTY *pty,
                    Atom wm_delete_window);
