#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include "error.h"
#include "strerr.h"
#include "open.h"
#include "buffer.h"

#define USAGE1 " unix|inet|ucspi-tcp acct logacct [/etc/socklog] [logdir]"
#define USAGE2 " notify acct grp [/etc/socklog] [pipe]"

#define VERSION "$Id$"
#define WARNING "socklog-conf: warning: "
#define FATAL "socklog-conf: fatal: "

#define CONF_DIR "/etc/socklog"
#define LOG_DIR_UNIX "/var/log/socklog"
#define LOG_DIR_INET "/var/log/socklog-inet"
#define LOG_DIR_UCSPI_TCP "/var/log/socklog-ucspi-tcp"
#define PATH_NOTIFY "/var/log/socklog/.notify"

#define CONF_UNIX 0
#define CONF_INET 1
#define CONF_UCSPI_TCP 2
#define CONF_NOTIFY 3

const char *progname;

void usage() {
  strerr_warn3("usage: ", progname, USAGE1, 0);
  strerr_die4x(1, "usage: ", progname, USAGE2, "\n");
}

const char *dir;
const char *fn;
char buf[1024];
int fd;
buffer b;

char *user;
char *loguser;
const char *path;

struct passwd *upw, *pw;
struct group *gr;

void fail() {
  strerr_die6sys(111, FATAL, "unable to create ", dir, "/", fn, ": ");
}

void start(const char *s) {
  fn = s;
  fd = open_trunc(fn);
  if (fd == -1) fail();
  buffer_init(&b, write, fd, buf, sizeof buf);
}

void outs(const char *s) {
  if (buffer_puts(&b, s) == -1) fail();
}

void finish(void) {
  if (buffer_flush(&b) == -1) fail();
  if (fsync(fd) == -1) fail();
  close(fd);
}

void perm(int mode) {
  if (chmod(fn, mode) == -1) fail();
}

void makedir(const char *s) {
  fn =s;
  if (mkdir(fn, 0750) == -1) fail();
}
void makechdir(const char *s) {
  makedir(s);
  if (chown(s, pw->pw_uid, pw->pw_gid) == -1)
    strerr_die6sys(111, FATAL, "unable to set owner of ", dir, "/", s, ": ");
}

void conf_unix() {
  makedir("unix");
  perm(01750);
  makedir("unix/log");
  perm(0755);
  makechdir("unix/log/main");
  if (rename("unix/log/main", path) != -1) {
    if (symlink(path, "unix/log/main") == -1)
      strerr_die4sys(111, FATAL, "unable to link ", path, ": ");
  }
  else
    strerr_warn4(WARNING, "unable to move to ", path, ": ", &strerr_sys);
  
  makechdir("unix/log/main/auth");
  makechdir("unix/log/main/cron");
  makechdir("unix/log/main/daemon");
  makechdir("unix/log/main/debug");
  makechdir("unix/log/main/ftp");
  makechdir("unix/log/main/kern");
  makechdir("unix/log/main/local");
  makechdir("unix/log/main/mail");
  makechdir("unix/log/main/main");
  makechdir("unix/log/main/news");
  makechdir("unix/log/main/syslog");
  makechdir("unix/log/main/user");
  
  start("unix/run");
  outs("#!/bin/sh\n");
  outs("exec 2>&1\n");
  outs("exec softlimit -m 2000000 envuidgid ");
  outs(user);
  outs(" socklog unix /dev/log\n");
  finish();
  perm(0750);

  start("unix/log/run");
  outs("#!/bin/sh\n");
  outs("exec setuidgid ");
  outs(loguser);
#define MULTILOG "\
 multilog s4999999 n10 ./main/main \\\n\
  s999999 n5 -* +kern.* ./main/kern \\\n\
  s999999 n5 -* +user.* ./main/user \\\n\
  s999999 n5 -* +mail.* ./main/mail \\\n\
  s999999 n5 -* +daemon.* ./main/daemon \\\n\
  s999999 n5 -* +auth.* +authpriv.* ./main/auth \\\n\
  s999999 n5 -* +syslog.* ./main/syslog \\\n\
  s999999 n5 -* +news.* ./main/news \\\n\
  s999999 n5 -* +cron.* ./main/cron \\\n\
  s999999 n5 -* +ftp.* ./main/ftp \\\n\
  s999999 n5 -* +local*.* ./main/local \\\n\
  s999999 n5 -* +*.debug* ./main/debug\n\
"
  outs(MULTILOG);
  finish();
  perm(0750);
}

void conf_inet() {
  makedir("inet");
  perm(01750);
  makedir("inet/log");
  perm(0755);
  makechdir("inet/log/main");
  if (rename("inet/log/main", path) != -1) {
    if (symlink(path, "inet/log/main") == -1)
      strerr_die4sys(111, FATAL, "unable to link ", path, ": ");
  }
  else
    strerr_warn4(WARNING, "unable to move to ", path, ": ", &strerr_sys);
  
  makechdir("inet/log/main/main");

  start("inet/run");
  outs("#!/bin/sh\n");
  outs("exec 2>&1\n");
  outs("exec softlimit -m 2000000 envuidgid ");
  outs(user);
  outs(" socklog inet 0 514\n");
  finish();
  perm(0750);

  start("inet/log/run");
  outs("#!/bin/sh\n");
  outs("exec setuidgid ");
  outs(loguser);
  outs(" multilog t ./main/main\n");
  finish();
  perm(0750);
}

void conf_ucspi_tcp() {
  makedir("ucspi-tcp");
  perm(01750);
  makedir("ucspi-tcp/log");
  perm(0755);
  makechdir("ucspi-tcp/log/main");
  if (rename("ucspi-tcp/log/main", path) != -1) {
    if (symlink(path, "ucspi-tcp/log/main") == -1)
      strerr_die4sys(111, FATAL,
		     "unable to link ", path, ": ");
  }
  else
    strerr_warn4(WARNING, "unable to move to ", path, ": ", &strerr_sys);
  
  makechdir("ucspi-tcp/log/main/main");

  start("ucspi-tcp/run");
  outs("#!/bin/sh\n");
  outs("PORT=10116\n\n");
  outs("exec 2>&1\n");
  outs("exec softlimit -m 2000000 \\\n  envuidgid ");
  outs(user);
  outs(" tcpserver -vUHR -l0 0 \"$PORT\" socklog ucspi TCPREMOTEIP\n");
  finish();
  perm(0750);

  start("ucspi-tcp/log/run");
  outs("#!/bin/sh\n");
  outs("exec setuidgid ");
  outs(loguser);
  outs(" multilog t ./main/main\n");
  finish();
  perm(0750);
}

void conf_notify() {
  makedir("notify");
  perm(0755);

  umask(007);
  if (mkfifo(path, 0620) == -1)
    strerr_die4sys(111, FATAL,
		   "unable to create \"", path, "\": ");
  umask(022);
  if (chown(path, upw->pw_uid, gr->gr_gid) == -1)
    strerr_die4sys(111, FATAL, "unable to set owner of ", path, ": ");

  start("notify/run");
  outs("#!/bin/sh -e\n");
  outs("MAILTO=root\n");
  outs("PIPE="); outs(path); outs("\n\n");
  outs("if [ ! -p \"$PIPE\" ]; then mkfifo -m0620 \"$PIPE\"; chown ");
  outs(user); outs(":"); outs(loguser);
  outs(" \"$PIPE\"; fi\n");
  outs("exec < \"$PIPE\"\n");
  outs("exec setuidgid ");
  outs(user);
  outs(" uncat -s49999 -t180 \\\n");
  outs("  env QMAILUSER=log QMAILNAME='socklog notify' \\\n");
  outs("  sh -c \"mailsubj socklog-notify $MAILTO\"\n");
  finish();
  perm(0750);
}


int main(int argc, char **argv) {
  int mode =0;

  progname =*argv++;
  umask(022);

  if (! *argv) usage();
  switch (**argv) {
  case 'u':
    if (! *(++*argv)) usage();
    switch (**argv) {
    case 'n':
      mode =CONF_UNIX;
      break;
    case 'c':
      mode =CONF_UCSPI_TCP;
      break;
    default:
      usage();
    }
    break;
  case 'i':
    mode =CONF_INET;
    break;
  case 'n':
    mode =CONF_NOTIFY;
    break;
  case '-':
    if ((*argv)[1] && (*argv)[1] == 'V') {
      strerr_warn2(VERSION, "\n", 0);
    }
  default:
    usage();
  }
  argv++;

  user =*argv++;
  if (!user) usage();
  loguser =*argv++;
  if (!loguser) usage();
  dir =*argv++;
  if (!dir) {
    dir =CONF_DIR;
  } else {
    if (dir[0] != '/') usage();
    if (*argv) {
      path =*argv;
    }
  }

  upw =getpwnam(user);
  if (!upw)
    strerr_die3x(111, FATAL, "unknown account ", user);
  if (mode != CONF_NOTIFY) {
    pw =getpwnam(loguser);
    if (!pw)
      strerr_die3x(111, FATAL, "unknown account ", loguser);
  } else {
    gr =getgrnam(loguser);
    if (!gr)
      strerr_die3x(111, FATAL, "unknown group ", loguser);
  }

  if (chdir(dir) == -1) {
    if (errno != error_noent)
      strerr_die4sys(111, FATAL, "unable to switch to ", dir, ": ");
  
    if (mkdir(dir, 0700) == -1)
      strerr_die4sys(111, FATAL, "unable to create ", dir, ": ");
    if (chmod(dir, 0750) == -1)
      strerr_die4sys(111, FATAL, "unable to set mode of ", dir, ": ");
    if (chdir(dir) == -1)
      strerr_die4sys(111, FATAL, "unable to switch to ", dir, ": ");
  }
  
  switch(mode) {
  case CONF_UNIX:
    if (!path) path =LOG_DIR_UNIX;
    conf_unix();
    break;
  case CONF_INET:
    if (!path) path =LOG_DIR_INET;
    conf_inet();
    break;
  case CONF_UCSPI_TCP:
    if (!path) path =LOG_DIR_UCSPI_TCP;
    conf_ucspi_tcp();
    break;
  case CONF_NOTIFY:
    if (!path) path =PATH_NOTIFY;
    conf_notify();
    break;
  }

  exit(0);
}
