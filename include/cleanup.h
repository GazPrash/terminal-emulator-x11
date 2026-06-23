#pragma once
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>
#include "x11_if.h"
#include "pty_pt.h"

void call_cleanup(X11_If *x11, PTY *pty);
