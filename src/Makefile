DESTDIR=

CC=gcc

CFLAGS=-Wall -O3
LDFLAGS=-Wall -O3 -s

# use this on solaris
#CFLAGS=-Wall -O3 -DSOLARIS
#LDFLAGS=-Wall -O3 -s -lsocket -lnsl -ldoor -lthread

TARGET=socklog
OBJECTS=$(TARGET).o fdbuffer.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

$(TARGET).o: $(TARGET).c

fdbuffer.o: fdbuffer.c fdbuffer.h

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;
	rm -f $(OBJECTS) $(TARGET)
