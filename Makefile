all: minitalk

CFLAGS?=
LDFLAGS?=
DESTDIR?=
PREFIX?=/usr/local

ifeq ($(shell uname -s), OpenBSD)
LDFLAGS+=-ltermcap
endif

minitalk: minitalk.c
	$(CC) $(CFLAGS) -o minitalk minitalk.c -lreadline $(LDFLAGS)

minitalk.1: minitalk.1.md
	pandoc -s -t man -o minitalk.1 minitalk.1.md

clean:
	rm -f minitalk

install: minitalk
	/usr/bin/install -Dm0755 -t $(DESTDIR)$(PREFIX)/bin minitalk
	/usr/bin/install -Dm0644 -t $(DESTDIR)$(PREFIX)/share/man/man1 minitalk.1
