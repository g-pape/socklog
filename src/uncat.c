#include <unistd.h>
#include "strerr.h"
#include "error.h"
#include "sgetopt.h"
#include "scan.h"
#include "stralloc.h"
#include "buffer.h"
#include "pathexec.h"
#include "fd.h"
#include "wait.h"
#include "taia.h"
#include "iopause.h"
#include "ndelay.h"
#include "sig.h"

/* defaults */
#define TIMEOUT 300
#define SIZEMAX 1024

#define USAGE " [ -v ] [ -t timeout ] [ -s size ] prog"
#define WARNING "uncat: warning: "
#define FATAL "uncat: fatal: "

const char *progname;
int exitasap =0;

void exit_asap() {
  exitasap =1;
}

void usage () {
  strerr_die4x(1, "usage: ", progname, USAGE, "\n");
}

int main (int argc, const char * const *argv, const char * const *envp) {
  int opt;
  unsigned long timeout =TIMEOUT;
  unsigned long sizemax =SIZEMAX;
  int verbose =0;
  static stralloc sa;
  int eof =0;

  progname =*argv;

  sig_block(sig_term);
  sig_catch(sig_term, exit_asap);

  while ((opt =getopt(argc, argv, "t:s:vV")) != opteof) {
    switch(opt) {
    case 'V':
      strerr_warn1("$Id$\n", 0);
    case '?':
      usage();
    case 't':
      scan_ulong(optarg, &timeout);
      if (timeout <= 0) timeout =TIMEOUT;
      break;
    case 's':
      scan_ulong(optarg, &sizemax);
      if (sizemax <= 0) sizemax =SIZEMAX;
      break;
    case 'v':
      verbose =1;
      break;
    }
  }
  argv +=optind;
  if (!*argv) usage();
  
  if (verbose) strerr_warn2(WARNING, "starting.", 0);

  ndelay_on(0);

  for (;;) {
    struct taia now, deadline;
    int cpipe[2];
    int pid;
    int wstat;

    stralloc_copys(&sa, "");

    /* set timeout */
    taia_now(&now);
    taia_uint(&deadline, timeout);
    taia_add(&deadline, &now, &deadline);

    /* read fd 0, stop at maxsize bytes or timeout */
    for (;;) {
      int r;
      char *s;
      iopause_fd sin;

      taia_now(&now);
      if (taia_less(&deadline, &now)) {
      	if (verbose && sa.len) strerr_warn2(WARNING, "timeout reached.", 0);
      	break;
      }
      sin.fd =0;
      sin.events =IOPAUSE_READ;

      sig_unblock(sig_term);
      iopause(&sin, 1, &deadline, &now);
      sig_block(sig_term);
      
      if (exitasap) {
      	if (verbose) strerr_warn2(WARNING, "got sigterm.", 0);
      	break;
      }
      
      r =buffer_feed(buffer_0);
      if (r < 0) {
	if (errno == error_again) continue;
	strerr_die2sys(111, FATAL, "failed reading fd 0: ");
      }
      if (r == 0) {
	++eof;
	if (verbose) strerr_warn2(WARNING, "end of input.", 0);
	break;
      }
      if (r >= sizemax) r =sizemax;
      if ((sa.len +r) > sizemax) {
      	if (verbose) strerr_warn2(WARNING, "max size reached.", 0);
      	break;
      }
      s =buffer_peek(buffer_0);
      if (! stralloc_catb(&sa, s, r)) {
	strerr_die2sys(111, FATAL, "out of memory: ");
      }
      buffer_seek(buffer_0, r);
    }
    if (sa.len) {
      /* run prog to process sa.s */
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
	
	sig_uncatch(sig_term);
	sig_unblock(sig_term);

	close(cpipe[1]);
	fd_move(0, cpipe[0]);
	
	if (verbose) strerr_warn2(WARNING, "starting child.", 0);
	pathexec_run(*argv, argv, envp);
	strerr_die2sys(111, FATAL, "could not start child: ");
      }
      
      close(cpipe[0]);
      if (write(cpipe[1], sa.s, sa.len) < sa.len) {
	strerr_warn2(WARNING, "failed writing to child: ", &strerr_sys);
      }
      close(cpipe[1]);
      
      if (wait_pid(&wstat, pid) != pid) {
	strerr_die2sys(111, FATAL, "wait_pid: ");
      }
      if (verbose) {
	if (wait_crashed(wstat)) {
	  strerr_warn2(WARNING, "child crashed.", 0);
	} else {
	  strerr_warn2(WARNING, "child exited.", 0);
	}
      }
    }

    if (exitasap || eof) break;
  }

  strerr_warn2(WARNING, "exit.", 0);
  exit(0);
}
