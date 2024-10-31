#include "include/pty_pt.h"
#include "include/render.h"
#include "include/x11_if.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

#define dlog printf("%s\n", "dlog - main.c");

int x_error_handler(Display *display, XErrorEvent *error) {
  char error_text[1024];
  XGetErrorText(display, error->error_code, error_text, sizeof(error_text));
  fprintf(stderr, "X Error: %s\n", error_text);
  fprintf(stderr, "Request code: %d\n", error->request_code);
  fprintf(stderr, "Minor code: %d\n", error->minor_code);
  fprintf(stderr, "Resource ID: 0x%lx\n", error->resourceid);

  return 0;
}

int main() {
  char *fontname = "SourceCodePro-Regular.ttf:size=15";
  X11_If *x11 = x11_init(35, 80, 200);

  // shell perference
  const char *shell = "/bin/dash";
  const char *login = "-/bin/dash";

  // xft config and setup
  load_font(x11, fontname);
  build_x11_interface(x11);
  setup_xft(x11);

  Atom wm_delete_window;
  wm_delete_window = XInternAtom(x11->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(x11->display, x11->window, &wm_delete_window, 1);

  PTY *pty = init_pty();
  if (!attatch_pty(pty)) {
    return 1;
  }
  if (!record_termsize_ioctl_pty(x11, pty)) {
    return 1;
  }

  if (!spawn_process(pty, shell, login)) {
    return 1;
  }
  // render_group *rg = render_init(x11);
  render_shell_mainloop(x11, pty, wm_delete_window);
}
