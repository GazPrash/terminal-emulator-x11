#include "../include/pty_pt.h"

int attatch_pty(PTY *pty) {
  char *slave_handle;
  pty->master = posix_openpt(O_RDWR | O_NOCTTY);
  if (pty->master == -1) {
    perror("posix_openpt");
    return 0;
  }

  /* grantpt() and unlockpt() are housekeeping functions that have to
   * be called before we can open the slave FD. Refer to the manpages
   * on what they do. */
  if (grantpt(pty->master) == -1) {
    perror("grantpt");
    return 0;
  }
  if (unlockpt(pty->master) == -1) {
    perror("unlockpt");
    return 0;
  }

  /* Up until now, we only have the master FD. We also need a file
   * descriptor for our child process. We get it by asking for the
   * actual path in /dev/pts which we then open using a regular
   * open(). So, unlike pipe(), you don't get two corresponding file
   * descriptors in one go. */

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
  pid_t p;
  char *env[] = {"TERM=dumb", NULL};

  p = fork();
  if (p == 0) {
    close(pty->master);

    /* Create a new session and make our terminal this process'
     * controlling terminal. The shell that we'll spawn in a second
     * will inherit the status of session leader. */
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
  } else if (p > 0) {
    close(pty->slave);
    return 1;
  }

  perror("fork");
  return 0;
}
