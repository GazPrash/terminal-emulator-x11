#pragma once
#define _XOPEN_SOURCE 600
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define SHELL "/bin/dash"

typedef struct {
  int master, slave;
} PTY;

int attatch_pty(PTY *pty);
int spawn_process(PTY *pty);
