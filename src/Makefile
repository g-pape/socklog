DESTDIR=

CC=gcc

CFLAGS=-Wall -O3 -Isupport
LDFLAGS=-Wall -O3 -s

# use this on solaris
#CFLAGS=-Wall -O3 -Isupport -DSOLARIS
#LDFLAGS=-Wall -O3 -s -lsocket -lnsl -ldoor -lthread

all: socklog tryto uncat

socklog: socklog.o fdbuffer.o
	$(CC) $(LDFLAGS) -o socklog socklog.o fdbuffer.o

socklog.o: socklog.c

fdbuffer.o: fdbuffer.c fdbuffer.h

tryto:	djb-publicdomain tryto.o
	$(CC) $(LDFLAGS) -o tryto \
		tryto.o support/unix.a support/byte.a support/time.a

#tryto.o:	tryto.c

uncat:	djb-publicdomain uncat.o
	$(CC) $(LDFLAGS) -o uncat \
		uncat.o support/unix.a support/byte.a support/time.a

#uncat.o:	uncat.c

djb-publicdomain:
	$(MAKE) -C support

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;
	if test -h support; then $(MAKE) -C support clean ; fi
	rm -f `cat TARGETS`
