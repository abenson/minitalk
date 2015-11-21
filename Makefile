all: minitalk

#CFLAGS=-W -Wall -ansi -pedantic
LDFLAGS=-lreadline

minitalk: minitalk.c

clean:
	rm -f minitalk
