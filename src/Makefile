DESTDIR=

CC=gcc

CFLAGS=-Wall -O3 # -DSOLARIS
LDFLAGS=-Wall -O3 -s

# use this on solaris
#CFLAGS=-Wall -O3 -DSOLARIS
#LDFLAGS=-Wall -O3 -s -lsocket -lnsl -ldoor -lthread

TARGET=socklog
OBJECTS=$(TARGET).o fdbuffer.o
DOCS=README INSTALL Configuration Examples
SERVICES=unix inet klog

all: docs $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

$(TARGET).o: $(TARGET).c

fdbuffer.o: fdbuffer.c fdbuffer.h

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;
	rm -f $(OBJECTS) $(TARGET)

installdirs:
	for i in $(SERVICES); do \
		install -d $(DESTDIR)/etc/socklog/$$i/log ; \
		chmod +t $(DESTDIR)/etc/socklog/$$i ; \
	done

install: installdirs
	install -m 0755 $(TARGET) $(DESTDIR)/usr/sbin/$(TARGET)
	for i in $(SERVICES); do \
		EXT="" ; \
		if [ -e $(DESTDIR)/etc/socklog/$$i/run ]; then \
			EXT=.distrib ; \
		fi ; \
		install -m 0755 $$i/run \
			$(DESTDIR)/etc/socklog/$$i/run$$EXT ; \
		EXT="" ; \
		if [ -e $(DESTDIR)/etc/socklog/$$i/log/run ]; then \
			EXT=.distrib ; \
		fi ; \
		install -m 0755 $$i/runlog \
			$(DESTDIR)/etc/socklog/$$i/log/run$$EXT ; \
	done

docs-clean:
	rm -f $(DOCS)

docs: $(DOCS)

README: doc/index.html
	w3m -dump doc/index.html > README

INSTALL: doc/install.html
	w3m -dump doc/install.html > INSTALL

Configuration: doc/configuration.html
	w3m -dump doc/configuration.html > Configuration

Examples: doc/examples.html
	w3m -dump doc/examples.html > Examples
