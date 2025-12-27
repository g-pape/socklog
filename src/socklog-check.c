#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "strerr.h"
#include "str.h"
#include "byte.h"
#include "sgetopt.h"

#define FATAL "socklog-check: fatal: "
#define WARNING "socklog-check: warning: "
#define INFO "socklog-check: info: "

#define VERSION "$Id$"
#define USAGE " [-v] [unix [address]]"

const char *progname;

void usage() { strerr_die4x(1, "usage: ", progname, USAGE, "\n"); }

int s;
struct sockaddr_un sa;
const char *address ="/dev/log";
unsigned int verbose =0;

int main(int argc, char **argv) {
  int opt;

  progname =*argv;

  while ((opt =getopt(argc, argv, "vV")) != opteof) {
    switch(opt) {
    case 'v': verbose =1; break;
    case 'V': strerr_warn1(VERSION, 0);
    case '?': usage();
    }
  }
  argv +=optind;

  if (argv && *argv) {
    if (**argv != 'u') usage();
    if (++argv && *argv) address =*argv;
  }
  if ((s =str_len(address)) >= sizeof(sa.sun_path))
    strerr_die4x(111, FATAL, "unable to create socket: ", address,
                 ": name too long");
  byte_zero((char *)&sa, sizeof(sa));
  sa.sun_family =AF_UNIX;
  byte_copy(sa.sun_path, s, address);
  if ((s =socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    strerr_die4sys(111, FATAL, "unable to create socket: ", address, ": ");
  if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    close(s);
#ifdef EDESTADDRREQ
    if (errno == EDESTADDRREQ) errno =error_connrefused;
#endif
    strerr_die4sys(111, WARNING, "unable to connect socket: ", address, ": ");
  }
  close(s);
  if (verbose) strerr_die3x(0, INFO, "successfully connected to ", address);
  return(0);
}
