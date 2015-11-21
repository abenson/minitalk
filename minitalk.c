/*

Copyright (c) 2015 Andrew Benson

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

/* Time format will be fixed format [YYYY-MM-DD HH:MM] */
#define TIME_FORMAT "%Y-%m-%d %H:%M"
#define TIME_STRSIZE 20

#define PROMPT "> "

/* GLOBALS */

/* File used for chat */
static FILE *ctrl = NULL;

static char nick[MAX_NICK_SIZE] = {0};
static char *msg;

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

/* Read all of the messages from the transcript since the last read */
static void check_msgs(void)
{
	char msg[120];
	char *saved_line = NULL;
	int saved_point = 0;

	/* Go back to the last read point. */
	fseek(ctrl, last_read_pos, SEEK_SET);

	while(fgets(msg, 255, ctrl) != NULL) {

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
		rl_replace_line("",0);
		rl_redisplay();
		fprintf(stdout, "%s", msg);
		rl_set_prompt(PROMPT);
		rl_replace_line(saved_line, 0);
		rl_point = saved_point;
		rl_redisplay();
		free(saved_line);
	}


	/* Update the last read position */
	last_read_pos = ftell(ctrl);

	/* Set the file position at the end of the file */
	fseek(ctrl, 0L, SEEK_END);
}

void sig_handler(int signal)
{
	/* We could have the signal handler call check_msgs directly, I guess */
	check_msgs();
}

void handle_msg(char *msg)
{
	/* Only care if we're called on a real message. */
	if(msg) {
		/* readline keeps history, let's use */
		add_history(msg);

		/* Quit if told to quit. Only write the message if there's something to say. */
		if(strncmp(msg, "/quit", 5) == 0)  {
			cont = NO;
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

int handle_enter(int x, int y)
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
void get_input(void) {
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

int main(int argc, char *argv[])
{

	struct sigaction sigact = {0};
	struct itimerval timer = {0};

	/* We always require a file, will guess a nick if none provided.*/
	if(argc < 2) {
		fprintf(stderr, "usage: %s file [nick]\n", argv[0]);
		return 1;
	}

	if(argv[2] == NULL) {
		/* If no nick is provided, pick based on the username. */
		getlogin_r(nick, MAX_NICK_SIZE);
	} else {
		/* Copy from argv and make sure its null-terminated.*/
		strncpy(nick, argv[2], MAX_NICK_SIZE);
		nick[MAX_NICK_SIZE-1] = '\0';
	}

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

	/* We're using setitimer, which sends a SIGALRM.
	   - sig_handler will handle when we get the alarm
	   - don't mask any signals
	   - no special behaviors
	   - perform action when we get SIGALRM
	*/
	sigact.sa_handler = sig_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	sigaction(SIGALRM, &sigact, NULL);

	/* This may be tweaked later, but for now fire every second*/
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &timer, NULL);

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
	}

	/* We're quitting now. Say goodbye. */
	write_status("left");

	/* Clean up for readline now */
	rl_unbind_key(RETURN);
	rl_callback_handler_remove();

	/* Being a good citizen. Closing file handles. */
	fclose(ctrl);

	return 0;
}
