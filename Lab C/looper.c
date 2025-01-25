#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include "LineParser.h"
#include <linux/limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
//#include <bits/signum-generic.h>
#define MAX_INPUT 2048

bool isDebugMode = false;  // default = false //  my line
char cwd[PATH_MAX];
char UserInput[MAX_INPUT];

void handler(int sig)
{
	printf("\nRecieved Signal : %s\n", strsignal(sig));
	if (sig == SIGTSTP)
	{
		signal(SIGTSTP, SIG_DFL);
		signal(SIGCONT, SIG_DFL);  //
	}
	else if (sig == SIGCONT)
	{
		signal(SIGCONT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);  //
	}
	
	signal(sig, SIG_DFL);
	raise(sig);
}


int main(int argc, char **argv)
{

	printf("Starting the program\n");
	signal(SIGINT, handler);
	signal(SIGTSTP, handler);
	signal(SIGCONT, handler);
	// signal(SIGINT, SIG_DFL);


	while (1)   //*    The Infinite Loop    <<<--------------------
	{
		//printf("looper running");
		//fflush(stdout);
		sleep(1);
	}

	return 0;
}


