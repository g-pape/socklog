DESTDIR=

CC=./support/compile
LD=./support/load

CFLAGS=-Isupport
LDFLAGS=

# use this on solaris
#CFLAGS=--Isupport -DSOLARIS
#LDFLAGS=-lsocket -lnsl -ldoor -lthread

all: socklog-conf socklog tryto uncat

socklog-conf: djb-publicdomain socklog-conf.o
	$(LD) socklog-conf support/unix.a support/byte.a

socklog: socklog.o fdbuffer.o
	$(LD) socklog fdbuffer.o $(LDFLAGS)

tryto:	djb-publicdomain tryto.o
	$(LD) tryto support/unix.a support/byte.a support/time.a

uncat:	djb-publicdomain uncat.o
	$(LD) uncat support/unix.a support/byte.a support/time.a

djb-publicdomain:
	( cd support ; make )

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;
	if test -h support; \
		then ( cd support ; $(MAKE) clean ); rm support; fi
	rm -f `cat TARGETS`
