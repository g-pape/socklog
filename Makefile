DESTDIR=

CC=gcc
CFLAGS=-Wall -O3
#LDFLAGS=-s

TARGET=socklog
OBJECTS=$(TARGET).o

SERVICES=unix inet klog

all: $(TARGET)

$(TARGET): $(OBJECTS)

$(TARGET).o: $(TARGET).c

clean:
	find . -name *~ -exec rm -f {} \;
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
		install -m 0755 $$i/run \
			$(DESTDIR)/etc/socklog/$$i/run ; \
		install -m 0755 $$i/runlog \
			$(DESTDIR)/etc/socklog/$$i/log/run ; \
	done
