/* fdbuffer.c
   buffered write to file descriptor
   
   thanks to djb for the opportunity to read the code of his buffer
   library.
*/   
            
#include "fdbuffer.h"
#include <unistd.h>

void char_copy(char *d, const char *s, unsigned int len) {
  while (len) {
    if (! (*d++ = *s++)) return;
    len--;
  }
}

int fd_write(int fd, const char *s, unsigned int len) {
  int i;

  while (len) {
    i =write(fd, s, len);
    if (i <= 0) return(-1);
    s +=i;
    len -=i;
  }
  return(0);
}

int fdbuffer_write(fdbuffer *b, const char *s, unsigned int len) {
  if (len > b->size - b->pos) {
    unsigned int max;
    
    max =b->size;
    fdbuffer_flush(b); // fdbuffer reset: b->pos =0
    if (max < FDBUFFER_MAX) max =FDBUFFER_MAX;
    while (len > b->size) {
      if (max > len) max =len;
      if (fd_write(b->fd, s, max) == -1) return(-1);
      s += max;
      len -= max;
    }
  }
  char_copy(b->s + b->pos, s, len);
  b->pos += len;
  return(0);
}

int fdbuffer_flush(fdbuffer *b) {
  int i;

  if (! b->pos) return(0); // buffer empty
  i =fd_write(b->fd, b->s, b->pos);
  fdbuffer_reset(b);
  return(i);
}

void fdbuffer_reset(fdbuffer *b) {
  b->pos =0;
}

void fdbuffer_init(fdbuffer *b, char *s, unsigned int size, int fd) {
  b->s =s;
  b->size =size;
  b->fd =fd;
  b->pos =0;
}
