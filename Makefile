all: chat

#CFLAGS=-W -Wall -ansi -pedantic
LDFLAGS=-lreadline

chat: minitalk.c

clean:
	rm -f minitalk
