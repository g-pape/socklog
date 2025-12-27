#include <unistd.h>
#include <signal.h>
#include "strerr.h"
#include "pathexec.h"
#include "iopause.h"
#include "taia.h"
#include "wait.h"
#include "sig.h"
#include "coe.h"
#include "ndelay.h"
#include "fd.h"
#include "buffer.h"
#include "error.h"
#include "sgetopt.h"
#include "scan.h"

/* defaults */
#define TIMEOUT 180
#define KTIMEOUT 5
#define TRYMAX  5

#define USAGE " [-vp] [-t seconds] [-k kseconds] [-n tries] prog"
#define WARNING "tryto: warning: "
#define FATAL "tryto: fatal: "

const char *progname;
int selfpipe[2];
int try =0;

void sig_child_handler(int unused) {
  try++;
  write(selfpipe[1], "", 1);
}

void usage () {
  strerr_die4x(1, "usage: ", progname, USAGE, "\n");
}

int main (int argc, char * const *argv, char * const *envp) {
  int opt;
  struct taia now, deadline;
  iopause_fd x[2];
  int pid;
  int rc =111;
  unsigned long timeout =TIMEOUT;
  unsigned long ktimeout =KTIMEOUT;
  unsigned long trymax =TRYMAX;
  int verbose =0;
  char ch;
  int processor =0;
  unsigned int pgroup =0;
  int cpipe[2];

  progname =*argv;

  while ((opt =getopt(argc,argv,"t:k:n:pPvV")) != opteof) {
    switch(opt) {
    case 'V':
      strerr_warn1("$Id$\n", 0);
    case '?':
      usage();
    case 't':
      scan_ulong(optarg, &timeout);
      if (timeout <= 0) timeout =TIMEOUT;
      break;
    case 'k':
      scan_ulong(optarg, &ktimeout);
      if (ktimeout <= 0) ktimeout =KTIMEOUT;
      break;
    case 'n':
      scan_ulong(optarg, &trymax);
      break;
    case 'p':
      processor =1;
      break;
    case 'P':
      pgroup =1;
      break;
    case 'v':
      verbose =1;
      break;
    }
  }
  argv +=optind;
  if (!*argv) usage();

  /* create selfpipe */
  if (pipe(selfpipe) == -1) {
    strerr_die2sys(111, FATAL, "unable to create selfpipe: ");
  }
  coe(selfpipe[0]);
  coe(selfpipe[1]);
  ndelay_on(selfpipe[0]);
  ndelay_on(selfpipe[1]);

  ndelay_on(0);
  if (processor) ndelay_on(4);

  sig_block(sig_pipe);
  sig_block(sig_child);
  sig_catch(sig_child, sig_child_handler);

  /* set timeout */
  taia_now(&now);
  taia_uint(&deadline, timeout);
  taia_add(&deadline, &now, &deadline);
  timeout =0;

  for (;;) {
    int iopausefds;
    char buffer_x_space[BUFFER_INSIZE];
    buffer buffer_x;

    if (processor) {
      buffer_init(&buffer_x, buffer_unixread, 4, buffer_x_space,
                  sizeof buffer_x_space);
    } else {
      buffer_init(&buffer_x, buffer_unixread, 0, buffer_x_space,
                  sizeof buffer_x_space);
    }

    /* start real processor */
    if (pipe(cpipe) == -1) {
      strerr_die2sys(111, FATAL, "unable to create pipe for child: ");
    }
    while ((pid =fork()) == -1) {
      strerr_warn4(WARNING, "unable to fork for \"", *argv, "\" pausing: ",
                   &strerr_sys);
      sleep(5);
    }
    if (!pid) {
      /* child */

      sig_unblock(sig_pipe);
      sig_unblock(sig_child);
      sig_uncatch(sig_child);

      close(cpipe[1]);
      fd_move(0, cpipe[0]);
      if (processor) {
        fd_move(2, 5);
        close(4);
      }
      if (pgroup) setsid();
      pathexec_run(*argv, argv, envp);
      strerr_die2sys(111, FATAL, "unable to start child: ");
    }
    close(cpipe[0]);

    x[0].fd =selfpipe[0];
    x[0].events =IOPAUSE_READ;
    if (processor) {
      fd_move(2, 5);
      x[1].fd =4;
    } else {
      x[1].fd =0;
    }
    x[1].events =IOPAUSE_READ;
    iopausefds =2;

    /* feed + watch child */
    for (;;) {
      int r;
      int i;
      char *s;

      sig_unblock(sig_child);
      iopause(x, iopausefds, &deadline, &now);
      sig_block(sig_child);
      
      while (read(selfpipe[0], &ch, 1) == 1) {}

      taia_now(&now);
      if ((timeout =taia_less(&deadline, &now))) break;
      if (wait_nohang(&rc) == pid) break;
      rc =111;

      r = buffer_feed(&buffer_x);
      if (r < 0) {
        if ((errno == error_intr) || (errno == error_again)) continue;
      }
      if (r == 0) {
        if (processor && (buffer_x.fd == 4)) {
          x[1].fd =0;
          buffer_init(&buffer_x, buffer_unixread, 0, buffer_x_space,
                      sizeof buffer_x_space);
          continue;
        }
        if (iopausefds == 2) {
          close(cpipe[1]);
          iopausefds =1;
        }
        continue;
      }
      s =buffer_peek(&buffer_x);
      i =write(cpipe[1], s, r);
      if (i == -1) strerr_die2sys(111, FATAL, "unable to write to child: ");
      if (i < r)
        strerr_die2x(111, FATAL, "unable to write to child: partial write");

      buffer_seek(&buffer_x, r);
    }
    close(cpipe[1]);

    if (timeout) {
      if (wait_nohang(&rc) == pid) break;
      /* child not finished */
      strerr_warn4(WARNING,
                   "child \"", *argv, "\" timed out. sending TERM...", 0);
      kill(pgroup ? -pid : pid, SIGTERM);

      /* ktimeout sec timeout */
      taia_now(&now);
      taia_uint(&deadline, ktimeout);
      taia_add(&deadline, &now, &deadline);
      ktimeout =0;

      for (;;) {
        sig_unblock(sig_child);
        iopause(x, 1, &deadline, &now);
        sig_block(sig_child);

        while (read(selfpipe[0], &ch, 1) == 1) {}

        if (wait_nohang(&rc) == pid) {
          strerr_warn2(WARNING, "child terminated.", 0);
          break;
        }
        rc =111;
        taia_now(&now);
        if ((ktimeout =taia_less(&deadline, &now))) break;
      }
      if (ktimeout) {
        strerr_warn4(WARNING, "child \"", *argv,
                     "\" not terminated. sending KILL...", 0);
        kill(pgroup ? -pid : pid, SIGKILL);
      }
      break;
    }
    if (rc == 0) _exit(0);
    if (verbose) strerr_warn2(WARNING, "child crashed.", 0);
    if (lseek(0, 0, SEEK_SET) != 0)
      if (verbose) strerr_warn2(WARNING,
                                "unable to lseek fd 0: ", &strerr_sys);
    if (try >= trymax) break;
    sleep(1);
  }

  if (processor) {
    for (;;) {
      int r;
      char *s;

      r = buffer_feed(buffer_0);
      if (r < 0) {
        if ((errno == error_intr) || (errno == error_again)) continue;
      }
      if (r == 0) {
        break;
      }
      s =buffer_peek(buffer_0);
      buffer_putflush(buffer_1, s, r);
      buffer_seek(buffer_0, r);
    }
  }
  if (timeout) {
    if (processor) strerr_die2x(0, FATAL, "child timed out, giving up.");
    strerr_die2x(100, FATAL, "child timed out, giving up.");
  }
  if (try >= trymax) {
    if (processor) strerr_die2x(0, FATAL, "child crashed, giving up.");
    strerr_die2x(rc >> 8, FATAL, "child crashed, giving up.");
  }
  _exit(0);
}
