PREFIX=/usr/local

all: minitalk

CFLAGS=-D_REENTRANT -D_POSIX_C_SOURCE -ansi -pedantic -W -Wall
LDFLAGS=-lreadline

minitalk: minitalk.c

clean:
	rm -f minitalk

install: minitalk
	/usr/bin/install -t $(PREFIX)/bin minitalk
