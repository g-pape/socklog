DESTDIR=

CC=gcc

CFLAGS=-Wall -O3 -Isupport
LDFLAGS=-Wall -O3 -s

# use this on solaris
#CFLAGS=-Wall -O3 -Isupport -DSOLARIS
#LDFLAGS=-Wall -O3 -s -lsocket -lnsl -ldoor -lthread

all: socklog-conf socklog tryto uncat

socklog-conf: djb-publicdomain socklog-conf.o
	$(CC) $(LDFLAGS) -o socklog-conf \
		socklog-conf.o support/unix.a support/byte.a

socklog: socklog.o fdbuffer.o
	$(CC) $(LDFLAGS) -o socklog socklog.o fdbuffer.o

fdbuffer.o: fdbuffer.c fdbuffer.h

tryto:	djb-publicdomain tryto.o
	$(CC) $(LDFLAGS) -o tryto \
		tryto.o support/unix.a support/byte.a support/time.a

uncat:	djb-publicdomain uncat.o
	$(CC) $(LDFLAGS) -o uncat \
		uncat.o support/unix.a support/byte.a support/time.a

djb-publicdomain:
	( cd support ; make )

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;
	if test -h support; \
		then ( cd support ; $(MAKE) clean ); rm support; fi
	rm -f `cat TARGETS`
