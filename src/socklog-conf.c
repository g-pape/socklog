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

#ifdef SOLARIS
#include <sys/utsname.h>
#include "scan.h"
#endif

#ifndef SOLARIS
#include <paths.h>
#endif
#ifndef _PATH_KLOG
#define _PATH_KLOG "/dev/klog"
#endif

#define USAGE1 " unix|inet|klog|ucspi acct logacct [/etc/socklog] [/logdir]"
#define USAGE2 " notify acct grp [/etc/socklog] [pipe]"

#define VERSION "$Id$"
#define WARNING "socklog-conf: warning: "
#define FATAL "socklog-conf: fatal: "

#define CONF_DIR "/etc/socklog"
#define LOG_DIR_UNIX "/var/log/socklog"
#define LOG_DIR_INET "/var/log/socklog-inet"
#define LOG_DIR_UCSPI_TCP "/var/log/socklog-ucspi-tcp"
#define LOG_DIR_KLOG "/var/log/socklog-klog"
#define PATH_NOTIFY "/var/log/socklog/.notify"

#define CONF_UNIX 0
#define CONF_INET 1
#define CONF_UCSPI_TCP 2
#define CONF_NOTIFY 3
#define CONF_KLOG 4

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
  if (fn[0] == '/') 
    strerr_die4sys(111, FATAL, "unable to create ", fn, ": ");
  else
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

void perm(int mode) { if (chmod(fn, mode) == -1) fail(); }

void mkfile(const char *f, const char *s) {
  start(f);
  outs(s); outs("\n");
  finish();
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
#ifdef SOLARIS
#if WANT_SUN_DOOR
  struct utsname u;
  unsigned long sunos_version;
  uname(&u);
  scan_ulong(u.release+strlen(u.release)-1, &sunos_version);
#endif
#endif
  makedir("unix");
  perm(0750);
  makedir("unix/log");
  perm(0755);

  makechdir(path);
  if (symlink(path, "unix/log/main") == -1)
    strerr_die4sys(111, FATAL, "unable to link ", path, ": ");
  makechdir("unix/log/main/auth");
  mkfile("unix/log/main/auth/config", "s999999\nn5\n-*\n+auth.*\n+authpriv.*");
  makechdir("unix/log/main/cron");
  mkfile("unix/log/main/cron/config", "s999999\nn5\n-*\n+cron.*");
  makechdir("unix/log/main/daemon");
  mkfile("unix/log/main/daemon/config", "s999999\nn5\n-*\n+daemon.*");
  makechdir("unix/log/main/debug");
  mkfile("unix/log/main/debug/config", "s999999\nn5\n-*\n+*.debug*");
  makechdir("unix/log/main/ftp");
  mkfile("unix/log/main/ftp/config", "s999999\nn5\n-*\n+ftp.*");
  makechdir("unix/log/main/kern");
  mkfile("unix/log/main/kern/config", "s999999\nn5\n-*\n+kern.*");
  makechdir("unix/log/main/local");
  mkfile("unix/log/main/local/config", "s999999\nn5\n-*\n+local.*");
  makechdir("unix/log/main/mail");
  mkfile("unix/log/main/mail/config", "s999999\nn5\n-*\n+mail.*");
  makechdir("unix/log/main/main");
  mkfile("unix/log/main/main/config", "s999999\nn10");
  makechdir("unix/log/main/news");
  mkfile("unix/log/main/news/config", "s999999\nn5\n-*\n+news.*");
  makechdir("unix/log/main/syslog");
  mkfile("unix/log/main/syslog/config", "s999999\nn5\n-*\n+syslog.*");
  makechdir("unix/log/main/user");
  mkfile("unix/log/main/user/config", "s999999\nn5\n-*\n+user.*");
  
  start("unix/run");
  outs("#!/bin/sh\n");
  outs("exec 2>&1\n");
  outs("USRID=`id -u "); outs(user); outs("`\n");
  outs("GRPID=`id -g "); outs(user); outs("`\n");
#ifndef SOLARIS
  outs("exec env UID=\"$USRID\" GID=\"$GRPID\" socklog unix /dev/log\n");
#else
  outs("exec env UID=\"$USRID\" GID=\"$GRPID\" socklog unix");
  outs(" socklog sun_stream /dev/log");
#if WANT_SUN_DOOR
  if (sunos_version == 7) outs(" /etc/.syslog_door");
  if (sunos_version >= 8) outs(" /var/run/syslog_door");
#endif
  outs("\n");
#endif
  finish();
  perm(0750);

  start("unix/log/run");
  outs("#!/bin/sh\n");
  outs("exec chpst -u");
  outs(loguser);
  outs(" svlogd \\\n");
  outs("  main/main main/auth main/cron main/daemon main/debug main/ftp \\\n");
  outs("  main/kern main/local main/mail main/news main/syslog main/user\n");
  finish();
  perm(0750);
}

void conf_inet() {
  makedir("inet");
  perm(0750);
  makedir("inet/log");
  perm(0755);

  makechdir(path);
  if (symlink(path, "inet/log/main") == -1)
    strerr_die4sys(111, FATAL, "unable to link ", path, ": ");
  makechdir("inet/log/main/main");
  mkfile("inet/log/main/main/config", "s999999\nn10");

  start("inet/run");
  outs("#!/bin/sh\n");
  outs("exec 2>&1\n");
  outs("USRID=`id -u "); outs(user); outs("`\n");
  outs("GRPID=`id -g "); outs(user); outs("`\n");
  outs("exec env UID=\"$USRID\" GID=\"$GRPID\" socklog inet 0 514\n");
  finish();
  perm(0750);

  start("inet/log/run");
  outs("#!/bin/sh\n");
  outs("exec chpst -u");
  outs(loguser);
  outs(" svlogd -t main/main\n");
  finish();
  perm(0750);
}

void conf_ucspi_tcp() {
  makedir("ucspi-tcp");
  perm(0750);
  makedir("ucspi-tcp/log");
  perm(0755);

  makechdir(path);
  if (symlink(path, "ucspi-tcp/log/main") == -1)
    strerr_die4sys(111, FATAL, "unable to link ", path, ": ");
  makechdir("ucspi-tcp/log/main/main");
  mkfile("ucspi-tcp/log/main/main/config", "s999999\nn10");

  start("ucspi-tcp/run");
  outs("#!/bin/sh\n");
  outs("PORT=10116\n");
  outs("exec 2>&1\n");
  outs("exec tcpsvd -vllogserver -u");
  outs(user);
  outs(" 0 \"$PORT\" socklog ucspi TCPREMOTEIP\n");
  finish();
  perm(0750);

  start("ucspi-tcp/log/run");
  outs("#!/bin/sh\n");
  outs("exec chpst -u");
  outs(loguser);
  outs(" svlogd -t main/main\n");
  finish();
  perm(0750);
}

void conf_klog() {
  makedir("klog");
  perm(0750);
  makedir("klog/log");
  perm(0755);

  makechdir(path);
  if (symlink(path, "klog/log/main") == -1)
    strerr_die4sys(111, FATAL, "unable to link ", path, ": ");
  makechdir("klog/log/main/main");
  mkfile("klog/log/main/main/config", "s999999\nn10");

  start("klog/run");
  outs("#!/bin/sh\n");
  outs("exec <"); outs(_PATH_KLOG); outs("\n");
  outs("exec 2>&1\n");
  outs("exec chpst -u");
  outs(user);
  outs(" socklog ucspi\n");
  finish();
  perm(0750);

  start("klog/log/run");
  outs("#!/bin/sh\n");
  outs("exec chpst -u");
  outs(loguser);
  outs(" svlogd -t main/main\n");
  finish();
  perm(0750);
}

void conf_notify() {
  makedir("notify");
  perm(0755);

  umask(007);
  if (mkfifo(path, 0620) == -1)
    strerr_die4sys(111, FATAL, "unable to create \"", path, "\": ");
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
  outs("exec <> \"$PIPE\"\n");
  outs("exec chpst -u");
  outs(user);
  outs(" uncat -s49999 -t180 \\\n");
  outs("  env MAILUSER=log MAILNAME='socklog notify' \\\n");
  outs("    mail -s socklog-notify $MAILTO\n");
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
  case 'k':
    mode =CONF_KLOG;
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
      if (path[0] != '/') usage();
    }
  }

  upw =getpwnam(user);
  if (!upw) strerr_die3x(111, FATAL, "unknown account ", user);
  if (mode != CONF_NOTIFY) {
    pw =getpwnam(loguser);
    if (!pw) strerr_die3x(111, FATAL, "unknown account ", loguser);
  } else {
    gr =getgrnam(loguser);
    if (!gr) strerr_die3x(111, FATAL, "unknown group ", loguser);
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
  case CONF_KLOG:
    if (!path) path =LOG_DIR_KLOG;
    conf_klog();
    break;
  case CONF_NOTIFY:
    if (!path) path =PATH_NOTIFY;
    conf_notify();
    break;
  }
  return(0);
}
