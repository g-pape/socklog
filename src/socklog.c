#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#include "buffer.h"
#include "strerr.h"
#include "scan.h"
#include "env.h"
#include "prot.h"

#define SYSLOG_NAMES
#include <syslog.h>

#ifdef SOLARIS
# include <stropts.h>
# include <sys/strlog.h>
# include <fcntl.h>
# include <door.h>
# include "syslognames.h"
#endif

/* #define WARNING "socklog: warning: " */
#define FATAL "socklog: fatal: "
#define USAGE " [unix|inet|ucspi] [args]"

#define VERSION "$Id$"
#define DEFAULTINET "0"
#define DEFAULTPORT "514"
#define DEFAULTUNIX "/dev/log"

const char *progname;

#define LINEC 1024
#define MODE_UNIX 0
#define MODE_INET 1
#define MODE_UCSPI 2

int mode =MODE_UNIX;
char line[LINEC];
const char *address =NULL;
char *uid, *gid;

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

int syslog_names (char *l, int lc, buffer *buf) {
  int i, fp;
  int ok =0;
  int fpr =0;
  CODE *p;

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
  i++;
  
  fp =LOG_FAC(fpr) <<3;
  for (p =facilitynames; p->c_name; p++) {
    if (p->c_val == fp) {
      buffer_puts(buf, p->c_name);
      buffer_puts(buf, ".");
      break;
    }
  }
  if (! p->c_name) {
    buffer_puts(buf, "unknown.");
    i =0;
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
    i =0;
  }
  return(i);
}

void remote_info (struct sockaddr_in *sa) {
  char *host;

  host =inet_ntoa(sa->sin_addr);
  out(host, ": ");
}

#ifndef SOLARIS
int socket_unix (const char* f) {
  int s;
  struct sockaddr_un sa;
  
  if ((s =socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    strerr_die2sys(111, FATAL, "socket(): ");

  sa.sun_family =AF_UNIX;
  strncpy(sa.sun_path, f, sizeof(sa.sun_path));
  unlink(f);
  umask(0);
  if (bind(s, (struct sockaddr*) &sa, sizeof sa) == -1)
    strerr_die2sys(111, FATAL, "bind(): ");

  err("listening on ", f, ", ");
  return(s);
}
#endif

int socket_inet (const char* ip, const char* port) {
  int s;
  unsigned long p;
  struct sockaddr_in sa;
  
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
  /* drop permissions */
  setuidgid();

  buffer_putsflush(buffer_2, "starting.\n");

  for(;;) {
    struct sockaddr_in saf;
    int dummy =sizeof saf;
    int linec;
    int os;
    
    linec =recvfrom(s, line, LINEC, 0, (struct sockaddr *) &saf, &dummy);
    if (linec == -1)
      strerr_die2sys(111, FATAL, "recvfrom(): ");

    while (linec && (line[linec -1] == 0)) linec--;
    if (linec == 0) continue;

    if (mode == MODE_INET) remote_info(&saf);
    os =syslog_names(line, linec, buffer_1);

    buffer_put(buffer_1, line +os, linec -os);
    if (linec == LINEC) out("...", 0);
    if (line[linec -1] != '\n') out("\n", 0);
    buffer_flush(buffer_1);
  }
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
	l += syslog_names(l, line -l +linec, buffer_2);
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
static void door_proc(void *cookie, char *argp, size_t arg_size,
		      door_desc_t *dp, uint_t ndesc) {
  door_return(NULL, 0, NULL, 0);
}

static int stream_sun(char *address, char *door, int *dfd) {
  int fd;
  struct strioctl sc;
  struct stat st;
  if ((fd = open(address, O_RDONLY | O_NOCTTY)) == -1)
    strerr_die2sys(111, FATAL, "open(): ");
  
  memset(&sc, 0, sizeof(sc));
  sc.ic_cmd =I_CONSLOG;
  if (ioctl(fd, I_STR, &sc) < 0)
    strerr_die2sys(111, FATAL, "ioctl(): ");

  if (door) {
    if (stat(door, &st) == -1) {
      /* The door file doesn't exist, create a new one */
      if ((*dfd =creat(door, 0666)) == -1)
	strerr_die2sys(111, FATAL, "creat(): ");

      close(*dfd);
    }
    fdetach(door);
    if ((*dfd =door_create(door_proc, NULL, 0)) == -1)
      strerr_die2sys(111, FATAL, "door_create(): ");

    if (fattach(*dfd, door) == -1)
      strerr_die2sys(111, FATAL, "fattach(): ");
  }
  else *dfd = -1;

  err("listening on ", address, ", ");
  return fd;
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
  
  setuidgid();
  
  buffer_putsflush(buffer_2, "starting.\n");
  
  /* read the messages */
  for (;;) {
    if ((getmsg(fd, &ctl, &data, &flags) & MORECTL))
      strerr_die2sys(111, FATAL, "getmsg(): ");

    if (data.len) {
      int shorten =data.len;
      if (!line[shorten-1]) shorten--;
      
      buffer_put(buffer_1, line, shorten);
      if (data.len == LINEC) out("...", 0);
      if (line[shorten-1] != '\n') out("\n", 0);
      buffer_flush(buffer_1);
    }
  }
}
#endif

int main(int argc, const char **argv, const char *const *envp) {
  int s =0;
#ifdef SOLARIS
  int dfd;
#endif
  
  progname =*argv++;
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
    case '-':
      if ((*argv)[1] && (*argv)[1] == 'V') {
	err(VERSION, 0, 0);
	buffer_putsflush(buffer_2, "\n\n");
      }
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
  case MODE_UNIX:
    if (*argv) usage();
    if (!address) address =DEFAULTUNIX;
#ifndef SOLARIS
    s =socket_unix(address);
    return(read_socket(s));
#else
    s =stream_sun(address, "/etc/.syslog_door", &dfd);
    read_stream_sun(s);
    if (dfd != -1) door_revoke(dfd);
    return 0;
#endif
  case MODE_UCSPI:
    s =0;
    return(read_ucspi(0, argv));
  }
  /* not reached */
  return(1);
}
