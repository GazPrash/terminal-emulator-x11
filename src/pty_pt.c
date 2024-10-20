#include "../include/pty_pt.h"

PTY *init_pty() {
  PTY *pty = (PTY *)malloc(sizeof(PTY));
  return pty;
}

int attatch_pty(PTY *pty) {
  char *slave_handle;
  pty->master = posix_openpt(O_RDWR | O_NOCTTY);
  if (pty->master == -1) {
    perror("posix_openpt");
    return 0;
  }
  if (grantpt(pty->master) == -1) {
    perror("grantpt");
    return 0;
  }
  if (unlockpt(pty->master) == -1) {
    perror("unlockpt");
    return 0;
  }
  slave_handle = ptsname(pty->master);
  if (slave_handle == NULL) {
    perror("ptsname");
    return 0;
  }
  pty->slave = open(slave_handle, O_RDWR | O_NOCTTY);
  if (pty->slave == -1) {
    perror("open(slave_handle)");
    return 0;
  }

  return 1;
}

int spawn_process(PTY *pty) {
  pid_t pid;
  char *env[] = {"TERM=dumb", NULL};

  // note:
  // use sys call fork a new child process and then once it is created
  // then we will dup the process a few times and attach
  // pty slave this basically establishes a connection with
  // (pty - master) attached to our terminal on the
  // x11 interface and finally we use the sys call
  // of execle to execute the shell command.
  // pty master - slave communication will transfer info between
  // shell and the terminal interface. thats it
  pid = fork();
  if (pid == 0) {
    close(pty->master);
    setsid();
    if (ioctl(pty->slave, TIOCSCTTY, NULL) == -1) {
      perror("ioctl(TIOCSCTTY)");
      return 0;
    }
    dup2(pty->slave, 0);
    dup2(pty->slave, 1);
    dup2(pty->slave, 2);
    close(pty->slave);
    execle(SHELL, "-" SHELL, (char *)NULL, env);
    return 0;
  } else if (pid > 0) {
    close(pty->slave);
    return 1;
  }
  perror("fork");
  return 0;
}
