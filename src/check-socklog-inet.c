#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "error.h"
#include "strerr.h"
#include "byte.h"

char ip[4];
int s;
struct sockaddr_in sa;

int main() {
  ip[0] =127; ip[1] =0; ip[2] =0; ip[3] =1;
  s =socket(AF_INET,SOCK_DGRAM,0);
  if (s == -1) strerr_die1sys(111, "fatal: unable to create socket: ");
  memset(&sa, 0, sizeof(sa));
  sa.sin_family =AF_INET;
  sa.sin_port =htons(0);
  byte_copy((char *) &sa.sin_addr, 4, ip);
  if (bind(s, (struct sockaddr *) &sa, sizeof sa) == -1)
    strerr_die1sys(111, "fatal: unable to bind socket: ");
  memset(&sa, 0, sizeof(sa));
  sa.sin_family =AF_INET;
  sa.sin_port =htons(12614);
  byte_copy((char *)&sa.sin_addr, 4, ip);
  if (sendto(s, "foo\n", 4, 0, (struct sockaddr *)&sa, sizeof(sa)) != 4)
    strerr_die1sys(111, "fatal: unable to send: ");
  close(s);
  return(0);
}
