#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include "error.h"
#include "strerr.h"
#include "byte.h"

char ip[4];
int s;
struct sockaddr_un sa;

int main() {
  ip[0] =127; ip[1] =0; ip[2] =0; ip[3] =1;
  s =socket(AF_UNIX, SOCK_DGRAM, 0);
  if (s == -1) strerr_die1sys(111, "fatal: unable to create socket: ");
//  memset(&sa, 0, sizeof(sa));
  sa.sun_family =AF_UNIX;
  strcpy(sa.sun_path, "socklog.check.socket");
  if (connect(s, (struct sockaddr *) &sa, sizeof sa) == -1)
    strerr_die1sys(111, "fatal: unable to connect socket: ");
  if (write(s, "foo\n", 4) != 4)
    strerr_die1sys(111, "fatal: unable to write to socket: ");
  close(s);
  return(0);
}
