PREFIX=/usr/local

all: minitalk

LDFLAGS=-lreadline

minitalk: minitalk.c

minitalk.1: minitalk.1.md
	pandoc -t man -o minitalk.1 minitalk.1.md

clean:
	rm -f minitalk

install: minitalk
	/usr/bin/install -t $(PREFIX)/bin minitalk
	/usr/bin/install -t $(PREFIX)/man/man1 minitalk.1
