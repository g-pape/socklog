/* fdbuffer.h
   buffered write to file descriptor

   thanks to djb for the opportunity to read the code of his buffer
   library.
*/
   
#ifndef FDBUFFER_H
#define FDBUFFER_H

typedef struct fdbuffer {
  char *s;
  unsigned int pos;
  unsigned int size;
  int fd;
} fdbuffer;

#define FDBUFFER_MAX 8192

void fdbuffer_init(fdbuffer *, char *, unsigned int, int);
/* fdbuffer_init(b, buf, size, fd)
   initializes the buffer b
   buf must point to a preallocated space
   size is the size of buf
   fd is the file desciptor to write the buffer to
*/

int fdbuffer_write(fdbuffer *, const char *, unsigned int);
/* fdbuffer_write(b, s, len)
   appends s[0], s[1], ..., s[len -1] to the buffer b->s
   calls fdbuffer_flush(b) automatically if it runs out of space

   returns 0 on success, -1 on error
*/

int fdbuffer_flush(fdbuffer *);
/* fdbuffer_flush(b)
   uses write() to write the buffer b->s to file descriptor b->fd
   calls fdbuffer_reset(b) to empty the buffer

   returns 0 on success, -1 on error
*/

void fdbuffer_reset(fdbuffer *);
/* fdbuffer_reset(b)
   resets the buffer to empty by setting b->pos =0
*/

#endif
