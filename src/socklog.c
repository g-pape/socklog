#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "byte.h"
#include "buffer.h"
#include "error.h"
#include "strerr.h"
#include "scan.h"
#include "env.h"
#include "prot.h"
#include "sig.h"
#include "open.h"
#include "sgetopt.h"

#define SYSLOG_NAMES
#include <syslog.h>

#if defined(__sun__) && defined(__sparc__) && defined(__unix__) && defined(__svr4__)
#define SOLARIS
# include <stropts.h>
# include <sys/strlog.h>
# include <fcntl.h>
# include "syslognames.h"
#if WANT_SUN_DOOR
# include <door.h>
#endif
#endif

/* #define WARNING "socklog: warning: " */
#define FATAL "socklog: fatal: "

#ifdef SOLARIS
#define USAGE " [-rRU] [unix|inet|ucspi|sun_stream] [args]"
#else
#define USAGE " [-rRU] [unix|inet|ucspi] [args]"
#endif

#define VERSION "$Id$"
#define DEFAULTINET "0"
#define DEFAULTPORT "514"
#define DEFAULTUNIX "/dev/log"

const char *progname;

#define LINEC 1024
#define MODE_UNIX 0
#define MODE_INET 1
#define MODE_UCSPI 2
#ifdef SOLARIS
#define MODE_SUN_STREAM 3
#endif

int mode =MODE_UNIX;
char line[LINEC];
const char *address =NULL;
char *uid, *gid;
unsigned int lograw =0;
unsigned int noumask =0;

int flag_exitasap = 0;
void sig_term_catch(void) {
  flag_exitasap = 1;
}

void usage() {
  strerr_die4x(1, "usage: ", progname, USAGE, "\n");
}

void out(const char *s1, const char *s2) {
  if (s1) buffer_puts(buffer_1, s1);
  if (s2) buffer_puts(buffer_1, s2);
}
void err(const char *s1, const char *s2, const char *s3) {
  if (s1) buffer_puts(buffer_2, s1);
  if (s2) buffer_puts(buffer_2, s2);
  if (s3) buffer_puts(buffer_2, s3);
}

void setuidgid() {
  /* drop permissions */
  if ((gid = env_get("GID")) != NULL) {
    unsigned long g;

    scan_ulong(gid, &g);
    err("gid=", gid, ", ");
    if (prot_gid(g) == -1)
      strerr_die2sys(111, FATAL, "unable to setgid: ");
  }
  if ((uid = env_get("UID")) != NULL) {
    unsigned long u;

    scan_ulong(uid, &u);
    err("uid=", uid, ", ");
    if (prot_uid(u) == -1)
      strerr_die2sys(111, FATAL, "unable to setuid: ");
  }
}

int print_syslog_names(int fpr, buffer *buf) {
  int fp =LOG_FAC(fpr) <<3;
  CODE *p;
  int rc =1;
  for (p =facilitynames; p->c_name; p++) {
    if (p->c_val == fp) {
      buffer_puts(buf, p->c_name);
      buffer_puts(buf, ".");
      break;
    }
  }
  if (! p->c_name) {
    buffer_puts(buf, "unknown.");
    rc =0;
  }
  fp =LOG_PRI(fpr);
  for (p =prioritynames; p->c_name; p++) {
    if (p->c_val == fp) {
      buffer_puts(buf, p->c_name);
      buffer_puts(buf, ": ");
      break;
    }
  }
  if (! p->c_name) {
    buffer_puts(buf, "unknown: ");
    rc =0;
  }
  return(rc);
}

int scan_syslog_names (char *l, int lc, buffer *buf) {
  int i;
  int ok =0;
  int fpr =0;

  if (l[0] != '<') return(0);
  for (i =1; (i < 5) && (i < lc); i++) {
    if (l[i] == '>') {
      ok =1;
      break;
    }
    if (('0' <= l[i]) && (l[i] <= '9')) {
      fpr =10 *fpr + l[i] -'0';
    } else {
      return(0);
    }
  }
  if (!ok || !fpr) return(0);
  return(print_syslog_names(fpr, buf) ? ++i : 0);
}

void remote_info (struct sockaddr_in *sa) {
  char *host;

  host =inet_ntoa(sa->sin_addr);
  out(host, ": ");
}

int socket_unix (const char* f) {
  int s;
  struct sockaddr_un sa;
  
  if ((s =socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    strerr_die2sys(111, FATAL, "socket(): ");
  byte_zero(&sa, sizeof(sa));
  sa.sun_family =AF_UNIX;
  strncpy(sa.sun_path, f, sizeof(sa.sun_path));
  unlink(f);
  if (! noumask) umask(0);
  if (bind(s, (struct sockaddr*) &sa, sizeof sa) == -1)
    strerr_die2sys(111, FATAL, "bind(): ");

  err("listening on ", f, ", ");
  return(s);
}

int socket_inet (const char* ip, const char* port) {
  int s;
  unsigned long p;
  struct sockaddr_in sa;
  
  byte_zero(&sa, sizeof(sa));
  if (ip[0] == '0') {
    sa.sin_addr.s_addr =INADDR_ANY;
  } else {
#ifndef SOLARIS
    if (inet_aton(ip, &sa.sin_addr) == 0) {
      strerr_die2sys(111, FATAL, "inet_aton(): ");
    }
#else
    sa.sin_addr.s_addr =inet_addr(ip);
#endif
  }
  if ((s =socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    strerr_die2sys(111, FATAL, "socket(): ");
  if (scan_ulong(port, &p) == 0)
    strerr_die3x(111, FATAL, "bad port number: ", port);

  sa.sin_family =AF_INET;
  sa.sin_port =htons(p);
  if (bind(s, (struct sockaddr*) &sa, sizeof sa) == -1)
    strerr_die2sys(111, FATAL, "bind(): ");

  ip =inet_ntoa(sa.sin_addr);
  err("listening on ", ip, 0);
  err(":", port, ", ");
  return(s);
}

int read_socket (int s) {
  sig_catch(sig_term, sig_term_catch);
  sig_catch(sig_int, sig_term_catch);
  /* drop permissions */
  setuidgid();
  buffer_putsflush(buffer_2, "starting.\n");

  for(;;) {
    struct sockaddr_in saf;
    int dummy =sizeof saf;
    int linec;
    int os;
    
    linec =recvfrom(s, line, LINEC, 0, (struct sockaddr *) &saf, &dummy);
    if (linec == -1) {
      if (errno != error_intr)
	strerr_die2sys(111, FATAL, "recvfrom(): ");
      else
	linec =0;
    }
    if (flag_exitasap) break;

    while (linec && (line[linec -1] == 0)) linec--;
    if (linec == 0) continue;

    if (lograw) {
      buffer_put(buffer_1, line, linec);
      if (line[linec -1] != '\n') {
	if (linec == LINEC) out("...", 0);
	out("\n", 0);
      }
      if (lograw == 2) {
	buffer_flush(buffer_1);
	continue;
      }
    }

    if (mode == MODE_INET) remote_info(&saf);
    os =scan_syslog_names(line, linec, buffer_1);

    buffer_put(buffer_1, line +os, linec -os);
    if (line[linec -1] != '\n') {
      if (linec == LINEC) out("...", 0);
      out("\n", 0);
    }
    buffer_flush(buffer_1);
  }
  return(0);
}

int read_ucspi (int fd, const char **vars) {
  char *envs[9];
  int flageol =1;
  int i;
  
  for (i =0; *vars && (i < 8); vars++) {
    if ((envs[i] =env_get(*vars)) != NULL)
      i++;
  }
  envs[i] =NULL;
  
  for(;;) {
    int linec;
    char *l, *p;
    
    linec =buffer_get(buffer_0, line, LINEC);
    if (linec == -1)
      strerr_die2sys(111, FATAL, "read(): ");

    if (linec == 0) {
      if (! flageol) err("\n", 0, 0);
      buffer_flush(buffer_2);
      return(0);
    }
    
    for (l =p =line; l -line < linec; l++) {
      if (flageol) {
	if (! *l || (*l == '\n')) continue;
	for (i =0; envs[i]; i++) {
	  err(envs[i], ": ", 0);
	}
	/* could fail on eg <13\0>user.notice: ... */
	l += scan_syslog_names(l, line -l +linec, buffer_2);
	p =l;
	flageol =0;
      }
      if (! *l || (*l == '\n')) {
	buffer_put(buffer_2, p, l -p);
	buffer_putsflush(buffer_2, "\n");
	flageol =1;
      }
    }
    if (!flageol) buffer_putflush(buffer_2, p, l -p);
  }
}

#ifdef SOLARIS
#if WANT_SUN_DOOR
static void door_proc(void *cookie, char *argp, size_t arg_size,
		      door_desc_t *dp, uint_t ndesc) {
  door_return(NULL, 0, NULL, 0);
  return;
}

static int door_setup(const char *door) {
  int dfd;
  struct door_info di;

  if ( (dfd = open_trunc(door)) == -1)
    strerr_die2sys(111, FATAL, "open_trunc(): ");

  if (door_info(dfd, &di) == -1) {
    if (errno != EBADF)
      strerr_die2sys(111, FATAL, "door_info(): ");
  }
  else {
    /*XXX: could log the pid of the door owner. */
    if (di.di_target != -1)
      strerr_die4x(100, FATAL, "door ", door, " allready in use.");
  }
  
  close(dfd);
  fdetach(door); /* highjack the door file */

  if ((dfd =door_create(door_proc, NULL, 0)) == -1)
    strerr_die2sys(111, FATAL, "door_create(): ");

  if (fattach(dfd, door) == -1)
    strerr_die2sys(111, FATAL, "fattach(): ");

  err("door path is ", door, ", ");
  return(dfd);
}
#endif /*WANT_SUN_DOOR*/

static int stream_sun(const char *address) {
  int sfd;
  struct strioctl sc;

  if ((sfd = open(address, O_RDONLY | O_NOCTTY)) == -1)
    strerr_die2sys(111, FATAL, "open(): ");
  
  memset(&sc, 0, sizeof(sc));
  sc.ic_cmd =I_CONSLOG;
  if (ioctl(sfd, I_STR, &sc) < 0)
    strerr_die2sys(111, FATAL, "ioctl(): ");

  err("sun_stream is ", address, ", ");
  return(sfd);
}

static void read_stream_sun(int fd) {
  struct strbuf ctl, data;
  struct log_ctl logctl;
  int flags;
  
  ctl.maxlen =ctl.len =sizeof(logctl);
  ctl.buf =(char *) &logctl;
  data.maxlen =LINEC;
  data.len =0;
  data.buf =line;
  flags =0;
  
  sig_catch(sig_term, sig_term_catch);
  sig_catch(sig_int, sig_term_catch);
  setuidgid();
  buffer_putsflush(buffer_2, "starting.\n");
  
  /* read the messages */
  for (;;) {

    if ((getmsg(fd, &ctl, &data, &flags) & MORECTL) && (errno != error_intr))
      strerr_die2sys(111, FATAL, "getmsg(): ");

    if (flag_exitasap)
      return;

    if (data.len) {
      int shorten =data.len;
      if (!line[shorten-1])
        shorten--;
      while (line[shorten-1] == '\n')
        shorten--;

      (void) print_syslog_names(logctl.pri, buffer_1);

      buffer_put(buffer_1, line, shorten);
      if (data.len == LINEC) out("...", 0);
      out("\n", 0);

      buffer_flush(buffer_1);
    }
  }
}

#endif

int main(int argc, const char **argv, const char *const *envp) {
  int opt;
  int s =0;
  
  progname =*argv;

  while ((opt =getopt(argc, argv, "rRUV")) != opteof) {
    switch(opt) {
    case 'r': lograw =1; break;
    case 'R': lograw =2; break;
    case 'U': noumask =1; break;
    case 'V':
      err(VERSION, 0, 0);
      buffer_putsflush(buffer_2, "\n\n");
    case '?': usage();
    }
  }
  argv +=optind;

  if (*argv) {
    switch (**argv) {
    case 'u':
      if (! *(++*argv)) usage();
      switch (**argv) {
      case 'n':
	mode =MODE_UNIX;
	break;
      case 'c':
	mode =MODE_UCSPI;
	argv--;
	break;
      default:
	usage();
      }
      break;
    case 'i':
      mode =MODE_INET;
      break;
#ifdef SOLARIS
    case 's':
      mode =MODE_SUN_STREAM;
      break;
#endif
    default:
      usage();
    }
    argv++;
  }

  if (*argv) address =*argv++;

  switch (mode) {
  case MODE_INET: {
    const char* port =NULL;

    if (*argv) port =*argv++;
    if (*argv) usage();
    if (!address) address =DEFAULTINET;
    if (!port) port =DEFAULTPORT;
    s =socket_inet(address, port);
    return(read_socket(s));
  }
  case MODE_UNIX: {
    if (*argv) usage();
    if (!address) address =DEFAULTUNIX;
    s =socket_unix(address);
    return(read_socket(s));
  }
#ifdef SOLARIS
  case MODE_SUN_STREAM: {
#if WANT_SUN_DOOR
    const char *door =NULL;
    int dfd =-1;
    if (*argv) door =*argv++;
#endif
    if (!address) address =DEFAULTUNIX;
    if (*argv) usage();

    s =stream_sun(address);

#if WANT_SUN_DOOR
    if (door)
      dfd = door_setup(door);
#endif

    read_stream_sun(s);

#if WANT_SUN_DOOR
    if (dfd != -1)
      door_revoke(dfd);
    /*
    ** syslogd does unlink() the door file, but we can't, since we droped
    ** all privs before.
    */
#endif
    return 0;
  }
#endif /*SOLARIS*/
  case MODE_UCSPI:
    s =0;
    return(read_ucspi(0, argv));
  }
  /* not reached */
  return(1);
}
