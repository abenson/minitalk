/*

Copyright (c) 2019 Andrew Benson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* minichat - small chat system for multiple users on a UNIX-like host */

#define VERSION "0.3.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>

#include <readline/readline.h>
#include <readline/history.h>

/* The maximum length of a nick string (will be truncated) */
#define MAX_NICK_SIZE 15

/* Time format will be fixed format [YYYY-MM-DD HH:MM:SS] */
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define TIME_STRSIZE 20

/* Maximum supported message size, including datetime/username */
#define MSG_LEN 1025

#define PROMPT "> "

/* GLOBALS */

/* File used for chat */
static FILE *ctrl = NULL;

static char nick[MAX_NICK_SIZE+1];

/* Keep track of the last position we read from. */
static long last_read_pos = 0;

/* BYO...Boolean. */
typedef enum { NO, YES} BOOL;


/* The program will run as long as this is set to YES. */
BOOL cont = YES;

/* Build the text string for the time. */
static void get_time(char *dt)
{
	struct tm tm = {0};
	time_t t = 0;

	t = time(NULL);
	gmtime_r(&t, &tm); /* UTC */

	strftime(dt, TIME_STRSIZE, TIME_FORMAT, &tm);
}

/* This is a "message", i.e. a chat from one user to another. */
static void write_msg(const char *msg)
{
	char dt[TIME_STRSIZE] = {0};

	get_time(dt);

	fprintf(ctrl, "[%s] <%s> %s\n", dt, nick, msg);

	fflush(ctrl);
}

/* This is a "status", i.e. user joined */
static void write_status(const char *status)
{
	char dt[TIME_STRSIZE] = {0};

	get_time(dt);

	fprintf(ctrl, "[%s] *** %s %s\n", dt, nick, status);

	fflush(ctrl);
}

/* Handle readline stuff and print a message to the screen. */

static void print_line(char *line)
{

	char *saved_line = NULL;
	int saved_point = 0;

	/* This is readline stuff.
	   - save the cursor position
	   - save the current line contents
	   - set the line to blank
	   - tell readline we're done mucking
	   - print the message
	   - restore the standard prompt
	   - restore the line contents
	   - restore the cursor position
	   - tell readline we're done mucking (again)
	*/
	saved_point = rl_point;
	saved_line = rl_copy_text(0, rl_end);
	rl_set_prompt("");
	rl_clear_message();
	rl_replace_line("", 0);
	rl_redisplay();
	fprintf(stdout, "%s", line);
	rl_set_prompt(PROMPT);
	rl_replace_line(saved_line, 0);
	rl_point = saved_point;
	rl_redisplay();
	free(saved_line);
}

/* Read all of the messages from the transcript since the last read */
static void check_msgs(void)
{
	char msg[MSG_LEN];
	int count;
	fd_set fds;
	int fd;
	struct timeval t;

	/* Timeout of 0. */
	t.tv_sec = 0;
	t.tv_usec = 0;

	/* Does ctrl have data available for reading? */
	FD_ZERO(&fds);
	fd = fileno(ctrl);
	FD_SET(fd, &fds);
	count = select(FD_SETSIZE, &fds, NULL, NULL, &t);

	if(count > 0) {
		/* Go back to the last read point. */
		fseek(ctrl, last_read_pos, SEEK_SET);

		while(fgets(msg, MSG_LEN, ctrl) != NULL) {
			char *str;
			if(str = strstr(msg, ">")) {
				if(strstr(str, nick)) {
					rl_ding();
				}
			}
			print_line(msg);
		}

		/* Update the last read position */
		last_read_pos = ftell(ctrl);

		/* Set the file position at the end of the file */
		fseek(ctrl, 0L, SEEK_END);
	}
}

static BOOL is_nick_allowed(char *nick)
{
	/* Verify there are only alpha-numeric characters in the nick,
	   it is at least one character, and is no more than the maximum
	   buffer size */
	if(nick == NULL || strlen(nick) < 1 || strlen(nick) > MAX_NICK_SIZE) {
		return NO;
	}

	while(*nick) {
		if(!isalnum(*nick)) {
			return NO;
		}
		nick++;
	}
	return YES;
}

static void handle_msg(char *msg)
{
	char lmsg[MSG_LEN] = {0};
	char nnick[MAX_NICK_SIZE+1] = {0};
	/* Only care if we're called on a real message. */
	if(msg) {
		/* readline keeps history, let's use */
		add_history(msg);

		/* Quit if told to quit. Only write the message if there's something to say. */
		if(strncmp(msg, "/quit", 5) == 0)  {
			cont = NO;
		} else if(strncmp(msg, "/nick", 5) == 0) {
			if(is_nick_allowed(msg+6)) {
				strncpy(nnick, msg+6, MAX_NICK_SIZE);
				snprintf(lmsg, MSG_LEN, "is now known as %s", nnick);
				write_status(lmsg);
				strncpy(nick, nnick, MAX_NICK_SIZE);
			} else {
				print_line("Please specify a valid nick. Nicks must 1-15 characters and only\n"
					"include numbers and letters.\n");
			}
		} else if(strncmp(msg, "/", 1) == 0) {
			snprintf(lmsg, MSG_LEN, "Unknown command: %s\n", msg+1);
			print_line(lmsg);
		} else if(strlen(msg) > 0) {
			write_msg(msg);
		}
	}
}

static void handle_line_fake(char *line)
{
	/* We want to ignore when readline says it has the end of a message, as we'll
	   only care if the user presses ENTER.
	*/
	if (line != NULL) {
		rl_set_prompt(PROMPT);
		rl_already_prompted = 1;
	} else {
		rl_set_prompt("");
		rl_replace_line("", 0);
		rl_redisplay();
		rl_set_prompt(PROMPT);
	}
}

static int handle_enter(int x, int y)
{
	char *line = NULL;

	/* Handle when a user presses enter.
	   - Save the contents of the line.
	   - Set the prompt to nothing.
	   - Blank the line.
	   - pass the message to the message handler
	   - rl_copy_text returns malloc'd mem, so free it
	   - restore the prompt
	   - tell readline we're done mucking
	*/
	line = rl_copy_text(0, rl_end);
	rl_set_prompt("");
	rl_replace_line("", 1);
	rl_redisplay();

	handle_msg(line);

	free(line);

	rl_set_prompt(PROMPT);
	rl_redisplay();

	rl_done = 1;
	return 0;
}


/* Don't block on I/O. */
static void get_input(void) {
	int count;
	fd_set fds;
	struct timeval t;

	/* Timeout of 0. */
	t.tv_sec = 0;
	t.tv_usec = 0;

	/* Does stdin have data available for reading? */
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	count = select(FD_SETSIZE, &fds, NULL, NULL, &t);

	/* If data is avaiable, read it. Else carry on */
	if (count > 0) {
		rl_callback_read_char();
	}
}

static void handle_exit_signal(int signal)
{
	fprintf(stderr, "Received signal %d, exiting.\n", signal);
	cont = NO;
}

int main(int argc, char *argv[])
{
	struct sigaction sa_exit;

	/* We always require a file, will guess a nick if none provided.*/
	if(argc < 2) {
		fprintf(stderr, "%s v%s - a small chat system for multiple users\n", argv[0], VERSION);
		fprintf(stderr, "usage: %s file [nick]\n", argv[0]);
		return 1;
	}

	if(argv[2] == NULL) {
		/* If no nick is provided, pick based on the username */
		if(getenv("LOGNAME") == NULL) {
			fprintf(stderr, "Can't determine username. Fix your environment or specify\n"
				"a name on the command line.\n");
			return 1;
		}
		if(!is_nick_allowed(getenv("LOGNAME"))) {
			fprintf(stderr, "Nicks must be 1-15 characters and only include numbers\n"
				"and letters. Your username does not meet these requirements; please\n"
				"specify a nick on the command line.\n");
			return 1;
		}
		strncpy(nick, getenv("LOGNAME"), MAX_NICK_SIZE);
	} else {
		/* Copy from command line. */
		if(!is_nick_allowed(argv[2])) {
			fprintf(stderr, "Nicks must be 1-15 characters and only include numbers\n"
				"and letters. The name you have provided does not meet these\n"
				"requirements.\n");
			return 1;
		}
		strncpy(nick, argv[2], MAX_NICK_SIZE);
	}

	/* Setup signal handlers */
	sa_exit.sa_handler = handle_exit_signal;
	sigemptyset(&sa_exit.sa_mask);
	sigaction(SIGINT, &sa_exit, NULL);
	sigaction(SIGTERM, &sa_exit, NULL);

	/* Open the file. Always write at the end. */
	ctrl = fopen(argv[1], "a+");
	if(ctrl == NULL) {
		fprintf(stderr, "Unable to open %s!\n", argv[1]);
		return 1;
	}

	/* We don't want to see past messages, as they could be loooooong.*/
	fseek(ctrl, 0L, SEEK_END);
	/* This is now our "last read" position. */
	last_read_pos = ftell(ctrl);

	/* Tell readline to let us know when the user hits enter. */
	rl_bind_key(RETURN, handle_enter);

	/* Setup the fake handler for when readline thinks we're done. */
	rl_callback_handler_install(PROMPT, handle_line_fake);

	/* Done with setup!
	   Now let everyone know the user has arrived. */
	write_status("joined");

	/* Until we decide to quit, tell readline to grab characters if they're available. */
	while(cont) {
		get_input();
		usleep(10000);
		check_msgs();
		usleep(10000);
	}

	/* We're quitting now. Say goodbye. */
	write_status("left");

	/* Clean up for readline now */
	rl_unbind_key(RETURN);
	rl_callback_handler_remove();

	/* Being a good citizen. Closing file handles. */
	fclose(ctrl);

	/* Clean up screen. */
	rl_set_prompt("");
	rl_clear_message();
	rl_redisplay();
	return 0;
}
