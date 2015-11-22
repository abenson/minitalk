PREFIX=/usr/local

all: minitalk

#CFLAGS=-W -Wall -ansi -pedantic
LDFLAGS=-lreadline

minitalk: minitalk.c

clean:
	rm -f minitalk

install: minitalk
	/usr/bin/install -t $(PREFIX)/bin minitalk
