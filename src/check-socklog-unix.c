#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include "error.h"
#include "strerr.h"
#include "byte.h"

int s;
struct sockaddr_un sa;

int main() {
  s =socket(AF_UNIX, SOCK_DGRAM, 0);
  if (s == -1) strerr_die1sys(111, "fatal: unable to create socket: ");
  byte_zero(&sa, sizeof(sa));
  sa.sun_family =AF_UNIX;
  strcpy(sa.sun_path, "socklog.check.socket");
  if (connect(s, (struct sockaddr *) &sa, sizeof(sa)) == -1)
    strerr_die1sys(111, "fatal: unable to connect socket: ");
  if (write(s, "foo\n", 4) != 4)
    strerr_die1sys(111, "fatal: unable to write to socket: ");
  close(s);
  return(0);
}
