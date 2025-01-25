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
#include <fcntl.h> 
#include <sys/types.h>

#define MAX_INPUT 2048

bool isDebugMode = false;  // default = false //  my line
char cwd[PATH_MAX];
char UserInput[MAX_INPUT];

void handler(int sig)  // <<<====================   from looper.c
{
	printf("\nRecieved Signal : %s\n", strsignal(sig));
	if (sig == SIGTSTP)
	{ 
		signal(SIGTSTP, SIG_DFL);
		signal(SIGCONT, SIG_DFL);  
	}
	else if (sig == SIGCONT)
	{
		signal(SIGCONT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
	}
	
	signal(sig, SIG_DFL);
	raise(sig);
}


void execute(cmdLine *pCmdLine);
void DebugModeHandling(int argc, char **argv);

int main(int argc, char **argv)
{

	printf("Starting the program\n");
	signal(SIGINT, handler);  // <<<====================      from looper.c
	signal(SIGTSTP, handler);  // <<<====================      from looper.c
	signal(SIGCONT, handler);  // <<<====================   from looper.c

	DebugModeHandling(argc, argv);

	while (1)   //*    The Infinite Loop    <<<--------------------
	{
		getcwd(cwd, PATH_MAX); // https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
		printf("The current working directory is: %s>\t", cwd);
		fflush(stdout);
		
		// Our code
		if(fgets(UserInput, MAX_INPUT, stdin) != NULL){
			printf("You typed in: %s\n", UserInput);
			cmdLine *parsed = parseCmdLines(UserInput);
			execute(parsed);
			freeCmdLines(parsed);
		}
		else{
			printf("ctrl+d was pressed/fgets gets NULL\n");
			exit(0);
		}


	}

	return 0;
}



 
void execute(cmdLine *pCmdLine){

	if(strcmp((*pCmdLine).arguments[0], "quit") == 0){
		freeCmdLines(pCmdLine);
		exit(0);
	} else if(strcmp(pCmdLine -> arguments[0], "cd") == 0){
		// Move to other dir
		int res = chdir(pCmdLine -> arguments[1]); //  https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
		if(res == -1){
			fprintf(stderr, "Failed to change the current working directory\n");
		}



	}else if(strcmp(pCmdLine -> arguments[0], "stop") == 0){
		int sstatus = kill(atoi(pCmdLine -> arguments[1]), SIGSTOP); // https://pubs.opengroup.org/onlinepubs/009695399/functions/kill.html
		if(sstatus == 0){
			printf("Running proccess was stopped\n");
		}else if (sstatus == -1)
		{
			perror("There was an error while using the \"stop\" command\n");
		}
		
	}else if(strcmp(pCmdLine -> arguments[0], "wake") == 0){
		int wstatus = kill(atoi(pCmdLine -> arguments[1]), SIGCONT); // https://pubs.opengroup.org/onlinepubs/009695399/functions/kill.html
		if(wstatus == 0){
			printf("Running proccess was woked up\n");
		}else if (wstatus == -1)
		{
			perror("There was an error while using the \"wake\" command\n");
		}
	}else if(strcmp(pCmdLine -> arguments[0], "term") == 0){
		int tstatus = kill(atoi(pCmdLine -> arguments[1]), SIGINT); // https://pubs.opengroup.org/onlinepubs/009695399/functions/kill.html
		printf("sssttaattuuss   ---->>>>  %d", tstatus);
		if(tstatus == 0){
			printf("Running proccess was terminated\n");
		}else if (tstatus == -1)
		{
			perror("There was an error while using the \"term\" command\n");
		}
	}



	else{
		int child_ID = fork(); // https://www.geeksforgeeks.org/fork-system-call/
		if(child_ID > 0){
			fprintf(stderr, "Child PID: %d\tExecuting Command: %s\n", child_ID, pCmdLine->arguments[0]);
		}
		int status;


		if (child_ID < 0) {
			perror("fork() unsuccessful\n");
			exit(1);

		} else if(child_ID == 0){
			int i;
			


            char *inputFile = pCmdLine -> inputRedirect;
            int inputFileDesc;
			if(inputFile != NULL){
                inputFileDesc = open(inputFile, O_RDONLY); // https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
                if(inputFileDesc == -1){
                    perror("Input file Error\n");
                    freeCmdLines(pCmdLine);
                    exit(1);
                }

				if(dup2(inputFileDesc, 0) == -1){  // https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
					perror("dup 2 at input failed\n");
					freeCmdLines(pCmdLine);
					exit(1);
				}
			}



			char *outputFile = pCmdLine -> outputRedirect;
            int OutputFileDesc;
			if(outputFile != NULL){
                OutputFileDesc = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC);    //? Ask Matan if we also want to append or not and create if not exist    <<<------------
                if(OutputFileDesc == -1){
                    perror("Output file Error\n");
                    freeCmdLines(pCmdLine);
                    exit(1);
                }

				// https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
                if(dup2(OutputFileDesc, 1) == -1){
                    perror("dup2 at output failed\n");
                    freeCmdLines(pCmdLine);
                    exit(1);
                }
			}




			i = execvp(pCmdLine->arguments[0], pCmdLine->arguments); // https://www.geeksforgeeks.org/exec-family-of-functions-in-c/
			printf("\n");
			if (i== -1)
			{
				freeCmdLines(pCmdLine);
				perror("execvp failed\n");
				exit(1);
			}

            if(inputFile != NULL){
                close(inputFileDesc); // https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
			}
            if(outputFile != NULL){
				close(OutputFileDesc); // https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
			}

			exit(1);
		}

		if(pCmdLine->blocking != 0){
			waitpid(child_ID, &status, 0); // https://stackoverflow.com/questions/21248840/example-of-waitpid-in-use
		}
	}
	
}








void DebugModeHandling(int argc, char **argv){   //  my line
    isDebugMode = false;  // default = true as specified in the assignment   //  my line
    for(int i=0; i<argc; i++){   //  my line
        if(strcmp(argv[i], "-d") == 0)   //  my line
        {
			isDebugMode = true;
			break;
        }
    }

	if(isDebugMode){
		for(int i=0; i<argc; i++){   //  my line
			fprintf(stderr, "myshell argument: %s\n",argv[i]);
			fflush(stderr);   //  my line
		}
	}

}

