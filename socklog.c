#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#define SYSLOG_NAMES
#include <syslog.h>

#define LINEC 1024
#define USAGE " [unix|inet] [address]"
#define VERSION "$Id$"
#define DEFAULTINET "0"
#define DEFAULTPORT "514"
#define DEFAULTUNIX "/dev/log"

char *progname;
int inet =0;

void usage() {
  write(2, "usage: ", 7);
  write(2, progname, strlen(progname));
  write(2, USAGE, strlen(USAGE));
  write(2, "\n\n", 2);
  exit(1);
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
    if ('0' <= line[i] <= '9') {
      fpr =10 *fpr + line[i] -'0';
    }
  }
  if (!ok || !fpr) return(0);
  i++;

  fp =LOG_FAC(fpr) <<3;
  for (p =facilitynames; p->c_name; p++) {
    if (p->c_val == fp) {
      write(2, p->c_name, strlen(p->c_name));
      write(2, ".", 1);
      break;
    }
  }
  if (! p->c_name) {
    write (2, "unknown.", 8);
    i =0;
  }
  fp =LOG_PRI(fpr);
  for (p =prioritynames; p->c_name; p++) {
    if (p->c_val == fp) {
      write(2, p->c_name, strlen(p->c_name));
      write(2, ": ", 2);
      break;
    }
  }
  if (! p->c_name) {
    write (2, "unknown: ", 9);
    i =0;
  }
  return(i);
}

void remote_info (struct sockaddr_in *sa) {
  char *host;

  host =inet_ntoa(sa->sin_addr);
  write(2, host, strlen(host));
  write(2, ": ", 2);
}

int socket_unix (char* f) {
  int s;
  struct sockaddr_un sa;

  if ((s =socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  sa.sun_family =AF_UNIX;
  strcpy(sa.sun_path, f);
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
int socket_inet (char* ip, char* port) {
  int s;
  struct sockaddr_in sa;

  if (ip[0] == '0') {
    sa.sin_addr.s_addr =INADDR_ANY;
  } else {
    if (inet_aton(ip, &sa.sin_addr) == 0) {
      perror("inet_aton");
      usage();
    }
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

int main(int argc, char** argv) {
  int s;
  char *address =NULL;
  char *uid, *gid;

  progname =*argv++;
  if (*argv) {
    switch (**argv) {
    case 'u':
      inet =0;
      break;
    case 'i':
      inet =1;
      break;
    case '-':
      if ((*argv)[1] && (*argv)[1] == 'v') {
	write(2, VERSION, strlen(VERSION));
	write(2, "\n", 1);
      }
    default:
      usage();
    }
    argv++;
  }
  if (*argv) address =*argv++;

  if (inet) {
    char* port =NULL;

    if (*argv) port =*argv++;
    if (!address) address =DEFAULTINET;
    if (!port) port =DEFAULTPORT;
    s =socket_inet(address, port);
  } else {
    if (!address) address =DEFAULTUNIX;
    s =socket_unix(address);
  }

  // drop permissions
  if ((gid = getenv("GID")) != NULL) {
    write(2, "gid=", 4);
    write(2, gid, strlen(gid));
    write(2, ", ", 2);
    if (setgid(atoi(gid)) == -1) {
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
  write(2, "starting.\n", 10);
  if (*argv) usage();

  for(;;) {
    struct sockaddr_in saf;
    int dummy =sizeof saf;
    int linec;
    char line[LINEC];
    int os;

    linec =recvfrom(s, line, LINEC, 0, (struct sockaddr *) &saf, &dummy);
    if (! (0 <= linec <= LINEC)) {
      perror("recvfrom");
      exit(1);
    }
    if (linec == 0) continue;

    if (inet) remote_info(&saf);
    os =syslog_names(line, linec);

    write(2, line +os, linec -os);
    if (linec == LINEC) write(2, "...", 3);
    if (line[linec -1] != '\n') write(2, "\n", 1);
  }
}
