PREFIX=/usr/local

all: minitalk

LDFLAGS=-lreadline

minitalk: minitalk.c
	$(CC) $(CFLAGS) -o minitalk minitalk.c $(CFLAGS) $(LDFLAGS)

minitalk.1: minitalk.1.md
	pandoc -s -t man -o minitalk.1 minitalk.1.md

clean:
	rm -f minitalk

install: minitalk
	/usr/bin/install -t $(PREFIX)/bin minitalk
	/usr/bin/install -t $(PREFIX)/share/man/man1 minitalk.1
