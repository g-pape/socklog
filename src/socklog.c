#include "fdbuffer.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>

#define SYSLOG_NAMES
#include <syslog.h>

#ifdef SOLARIS
# include <stropts.h>
# include <sys/strlog.h>
# include <fcntl.h>
# include <door.h>
# include "syslognames.h"
#endif

#define LINEC 1024
#define USAGE " [unix|inet|ucspi] [args]"
#define VERSION "$Id$"
#define DEFAULTINET "0"
#define DEFAULTPORT "514"
#define DEFAULTUNIX "/dev/log"

char *progname;

#define MODE_UNIX 0
#define MODE_INET 1
#define MODE_UCSPI 2

int mode =MODE_UNIX;
char line[LINEC];
char *address =NULL;
char *uid, *gid;
char buf[1024];
fdbuffer fdbuf;

void usage() {
  write(2, "usage: ", 7);
  write(2, progname, strlen(progname));
  write(2, USAGE, strlen(USAGE));
  write(2, "\n\n", 2);
  exit(1);
}

void setuidgid() {
  /* drop permissions */
  if ((gid = getenv("GID")) != NULL) {
    int g =atoi(gid);
    
    write(2, "gid=", 4);
    write(2, gid, strlen(gid));
    write(2, ", ", 2);
    if (setgroups(1, &g)) {
      perror("setgroups");
      usage();
    }
    if (setgid(g) == -1) {
      perror("setgid");
      usage();
    }
  }
  if ((uid = getenv("UID")) != NULL) {
    write(2, "uid=", 4);
    write(2, uid, strlen(uid));
    write(2, ", ", 2);
    if (setuid(atoi(uid)) == -1) {
      perror("setuid");
      usage();
    }
  }
}

int syslog_names (char *line, int linec) {
  int i, fp;
  int ok =0;
  int fpr =0;
  CODE *p;

  if (line[0] != '<') return(0);
  for (i =1; (i < 5) && (i < linec); i++) {
    if (line[i] == '>') {
      ok =1;
      break;
    }
    if (('0' <= line[i]) && (line[i] <= '9')) {
      fpr =10 *fpr + line[i] -'0';
    } else {
      return(0);
    }
  }
  if (!ok || !fpr) return(0);
  i++;
  
  fp =LOG_FAC(fpr) <<3;
  for (p =facilitynames; p->c_name; p++) {
    if (p->c_val == fp) {
      fdbuffer_write(&fdbuf, p->c_name, strlen(p->c_name));
      fdbuffer_write(&fdbuf, ".", 1);
      break;
    }
  }
  if (! p->c_name) {
    fdbuffer_write(&fdbuf, "unknown.", 8);
    i =0;
  }
  fp =LOG_PRI(fpr);
  for (p =prioritynames; p->c_name; p++) {
    if (p->c_val == fp) {
      fdbuffer_write(&fdbuf, p->c_name, strlen(p->c_name));
      fdbuffer_write(&fdbuf, ": ", 2);
      break;
    }
  }
  if (! p->c_name) {
    fdbuffer_write(&fdbuf, "unknown: ", 9);
    i =0;
  }
  return(i);
}

void remote_info (struct sockaddr_in *sa) {
  char *host;

  host =inet_ntoa(sa->sin_addr);
  fdbuffer_write(&fdbuf, host, strlen(host));
  fdbuffer_write(&fdbuf, ": ", 2);
}

#ifndef SOLARIS
int socket_unix (char* f) {
  int s;
  struct sockaddr_un sa;
  
  if ((s =socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  sa.sun_family =AF_UNIX;
  strncpy(sa.sun_path, f, sizeof(sa.sun_path));
  unlink(f);
  umask(0);
  if (bind(s, (struct sockaddr*) &sa, sizeof sa) == -1) {
    perror("bind");
    exit(1);
  }
  write(2, "listening on ", 13);
  write(2, f, strlen(f));
  write(2, ", ", 2);
  return(s);
}
#endif

int socket_inet (char* ip, char* port) {
  int s;
  struct sockaddr_in sa;
  
  if (ip[0] == '0') {
    sa.sin_addr.s_addr =INADDR_ANY;
  } else {
#ifndef SOLARIS
    if (inet_aton(ip, &sa.sin_addr) == 0) {
      perror("inet_aton");
      usage();
    }
#else
    sa.sin_addr.s_addr =inet_addr(ip);
#endif
  }
  if ((s =socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  sa.sin_family =AF_INET;
  sa.sin_port =htons(atoi(port));
  if (bind(s, (struct sockaddr*) &sa, sizeof sa) == -1) {
    perror("bind");
    exit(1);
  }
  ip =inet_ntoa(sa.sin_addr);
  write(2, "listening on ", 13);
  write(2, ip, strlen(ip));
  write(2, ":", 1);
  write(2, port, strlen(port));
  write(2, ", ", 2);
  return(s);
}

int read_socket (int s) {
  fdbuffer_init(&fdbuf, buf, 1024, 1);

  /* drop permissions */
  setuidgid();

  write(2, "starting.\n", 10);

  for(;;) {
    struct sockaddr_in saf;
    int dummy =sizeof saf;
    int linec;
    int os;
    
    linec =recvfrom(s, line, LINEC, 0, (struct sockaddr *) &saf, &dummy);
    if (! (0 <= linec <= LINEC)) {
      perror("recvfrom");
      exit(1);
    }
    if (linec == 0) continue;

    if (mode == MODE_INET) remote_info(&saf);
    os =syslog_names(line, linec);

    fdbuffer_write(&fdbuf, line +os, linec -os);
    if (linec == LINEC) fdbuffer_write(&fdbuf, "...", 3);
    if (line[linec -1] != '\n') fdbuffer_write(&fdbuf, "\n", 1);
    fdbuffer_flush(&fdbuf);
  }
}

int read_ucspi (int fd, char** vars) {
  char *envs[9];
  int flageol =1;
  int i;
  
  fdbuffer_init(&fdbuf, buf, 1024, 2);
  
  for (i =0; *vars && (i < 8); vars++) {
    if ((envs[i] =getenv(*vars)) != NULL) {
      i++;
    }
  }
  envs[i] =NULL;
  
  for(;;) {
    int linec;
    char *l, *p;
    
    linec =read(fd, line, LINEC);
    if (linec == -1) {
      fdbuffer_flush(&fdbuf);
      perror("read");
      return(1);
    }
    if (linec == 0) {
      if (! flageol) fdbuffer_write(&fdbuf, "\n", 1);
      fdbuffer_flush(&fdbuf);
      return(0);
    }
    
    for (l =p =line; l -line < linec; l++) {
      if (flageol) {
	if (! *l || (*l == '\n')) continue;
	for (i =0; envs[i]; i++) {
	  fdbuffer_write(&fdbuf, envs[i], strlen(envs[i]));
	  fdbuffer_write(&fdbuf, ": ", 2);
	}
	/* could fail on eg <13\0>user.notice: ... */
	l += syslog_names(l, line -l +linec);
	p =l;
	flageol =0;
      }
      if (! *l || (*l == '\n')) {
	fdbuffer_write(&fdbuf, p, l -p);
	fdbuffer_write(&fdbuf, "\n", 1);
	fdbuffer_flush(&fdbuf);
	flageol =1;
      }
    }
    if (!flageol) fdbuffer_write(&fdbuf, p, l -p);
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
  if ((fd = open(address, O_RDONLY | O_NOCTTY)) == -1) {
    perror("open");
    exit(1);
  }
  
  memset(&sc, 0, sizeof(sc));
  sc.ic_cmd =I_CONSLOG;
  if (ioctl(fd, I_STR, &sc) < 0) {
    perror("ioctl");
    exit(1);
  }
  if (door) {
    if (stat(door, &st) == -1) {
      /* The door file doesn't exist, create a new one */
      if ((*dfd =creat(door, 0666)) == -1) {
	perror("creat");
	exit(1);
      }
      close(*dfd);
    }
    fdetach(door);
    if ((*dfd =door_create(door_proc, NULL, 0)) == -1) {
      perror("door_create");
      exit(1);
    }
    if (fattach(*dfd, door) == -1) {
      perror("fattach");
      exit(1);
    }
  }
  else *dfd = -1;
  write(2, "listening on ", 13);
  write(2, address, strlen(address));
  write(2, ", ", 2);
  return fd;
}

static void read_stream_sun(int fd) {
  struct strbuf ctl, data;
  struct log_ctl logctl;
  int flags;
  
  /* Initialize buffer */
  fdbuffer_init(&fdbuf, buf, LINEC, 1);
  
  ctl.maxlen =ctl.len =sizeof(logctl);
  ctl.buf =(char *) &logctl;
  data.maxlen =LINEC;
  data.len =0;
  data.buf =line;
  flags =0;
  
  setuidgid();
  
  write(2, "starting.\n", 10);
  
  /* read the messages */
  for (;;) {
    if ((getmsg(fd, &ctl, &data, &flags) & MORECTL)) {
      perror("getmsg");
      return;
    }
    if (data.len) {
      int shorten =data.len;
      if (!line[shorten-1]) shorten--;
      
      fdbuffer_write(&fdbuf, line, shorten);
      if (data.len == LINEC) fdbuffer_write(&fdbuf, "...", 3);
      if (line[shorten-1] != '\n') fdbuffer_write(&fdbuf, "\n", 1);
      fdbuffer_flush(&fdbuf);
    }
  }
}
#endif

int main(int argc, char** argv) {
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
	write(2, VERSION, strlen(VERSION));
	write(2, "\n\n", 2);
      }
    default:
      usage();
    }
    argv++;
  }
  if (*argv) address =*argv++;

  switch (mode) {
  case MODE_INET: {
    char* port =NULL;

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
