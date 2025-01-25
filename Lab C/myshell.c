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
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 10   // Part 4 - history

typedef struct commandLineString{
	char* commandString;
	struct commandLineString *next;
} commandLineString;

typedef struct commandLineStringsList{
	int size;
	struct commandLineString *head;  // we delete from the head
	struct commandLineString *tail;  // we add to the tail
} commandLineStringsList;

commandLineStringsList *UnparsedCommandHistory = NULL;

typedef struct process{
	cmdLine* cmd;                         /* the parsed command line*/
	pid_t pid; 		                  /* the process id that is running the command*/
	int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
	struct process *next;	                  /* next process in chain */
} process;

process *globalProcessList = NULL;  // pointer to a pointer so that we can access the first element added, like said in the assignment.


void handler(int sig);  // <<<====================   from lab 2
void DebugModeHandling(int argc, char **argv);  // <<<====================   from lab 2
bool isDebugMode = false;  // default = false
char cwd[PATH_MAX];
char UserInput[MAX_INPUT];

void execute(cmdLine *pCmdLine);
void createPipe(cmdLine *pCmdLine, cmdLine *nextpCmdLine);

void addProcess(process** process_list, cmdLine* cmd, pid_t pid); // 3a
void printProcessList(process** process_list); // 3a
void printProcess(process* proc, int index, char *status); //3a
void freeProcessList(process* process_list); // 3b
void updateProcessList(process **process_list); // 3b
void updateProcessStatus(process* process_list, int pid, int status); // 3b
void deleteTerminatedFromList(process** process_list);

void CMDHistoryPrinter(); // part 4
void exclamationExclamation(); // part 4
void exclamationN(int n); // part 4

void addToStringHistory(char *commandString);  //  Part 4
void deleteFromStringHistoryHead(); // Part 4

void freeAll();   //  handling of overall memory freeing
void deleteALLStringHistory();


int main(int argc, char **argv)
{

	printf("Starting the program\n");  // <<<====================      from lab 2
	signal(SIGINT, handler);  // <<<====================      from lab 2
	signal(SIGTSTP, handler);  // <<<====================      from lab 2
	signal(SIGCONT, handler);  // <<<====================   from lab 2

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
			addToStringHistory(UserInput);
			execute(parsed);
			freeCmdLines(parsed);
		}
		else{
			printf("ctrl+d was pressed/fgets gets NULL\n");
			freeAll();
			exit(0);
		}

	}

	freeAll();
	return 0;
}



 
void execute(cmdLine *pCmdLine){

	if(strcmp((*pCmdLine).arguments[0], "quit") == 0){
		freeCmdLines(pCmdLine);
		freeAll();
		exit(0);
	}else if(strcmp(pCmdLine -> arguments[0], "cd") == 0){
		// Move to other dir
		int res = chdir(pCmdLine -> arguments[1]); //  https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
		if(res == -1){
			fprintf(stderr, "Failed to change the current working directory\n");
		}

	}
	else if(strcmp(pCmdLine -> arguments[0], "stop") == 0){
		int sstatus = kill(atoi(pCmdLine -> arguments[1]), SIGSTOP); // https://pubs.opengroup.org/onlinepubs/009695399/functions/kill.html
		if(sstatus == 0){
			printf("Running proccess was stopped\n");
		}else if (sstatus == -1)
		{
			perror("There was an error while using the \"stop\" command\n");
		}
		updateProcessStatus(globalProcessList, atoi(pCmdLine->arguments[1]), SUSPENDED);
		
	}
	else if(strcmp(pCmdLine -> arguments[0], "wake") == 0){
		int wstatus = kill(atoi(pCmdLine -> arguments[1]), SIGCONT); // https://pubs.opengroup.org/onlinepubs/009695399/functions/kill.html
		if(wstatus == 0){
			printf("Running proccess was woked up\n");
		}else if (wstatus == -1)
		{
			perror("There was an error while using the \"wake\" command\n");
		}
		updateProcessStatus(globalProcessList, atoi(pCmdLine->arguments[1]), RUNNING);
	}
	else if(strcmp(pCmdLine -> arguments[0], "term") == 0){
		int tstatus = kill(atoi(pCmdLine -> arguments[1]), SIGINT); // https://pubs.opengroup.org/onlinepubs/009695399/functions/kill.html
		if(tstatus == 0){
			printf("Running proccess was terminated\n");
		}else if (tstatus == -1)
		{
			perror("There was an error while using the \"term\" command\n");
		}
		updateProcessStatus(globalProcessList, atoi(pCmdLine->arguments[1]), TERMINATED);
		
	}
	else if(strcmp(pCmdLine -> arguments[0], "procs") == 0){  // Part 3a
		printProcessList(&globalProcessList);
	}
	else if(strcmp(pCmdLine -> arguments[0], "history") == 0){  // Part 4
		CMDHistoryPrinter();

	}
	else if(strcmp(pCmdLine -> arguments[0], "!!") == 0){  // Part 4
		exclamationExclamation();

	}
	else if(pCmdLine -> arguments[0][0] == '!'){  // Part 4
		exclamationN(atoi(&(pCmdLine -> arguments[0][1])));

	}
	else if(pCmdLine->next != NULL){     //*    <<<-------------------  piping case

		if (pCmdLine->outputRedirect != NULL){   //  when we're in the left hand side command     <<<-----------------------
			fprintf(stderr, "Left hand side's output cannot be redirected\n");
		}
		else if (pCmdLine -> next -> inputRedirect != NULL){   //  when we've gotten to the right hand side command     <<<-----------------------
			fprintf(stderr, "Right hand side's input cannot be redirected\n");
		}
		else{
			createPipe(pCmdLine, pCmdLine -> next);
		}
		
	}



	else{

		int i;
		int child_ID = fork(); // https://www.geeksforgeeks.org/fork-system-call/
		if(child_ID > 0){
			fprintf(stderr, "Child PID: %d\tExecuting Command: %s\n", child_ID, pCmdLine->arguments[0]);
		}
		int status =0;

		if (child_ID < 0) {
			perror("fork() unsuccessful\n");
			freeAll();
			exit(1);

		} else if(child_ID == 0){
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
                OutputFileDesc = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC);
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

			if(inputFile != NULL){
                close(inputFileDesc); // https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
			}
            if(outputFile != NULL){
				close(OutputFileDesc); // https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
			}

			if (i == -1)
			{
				freeCmdLines(pCmdLine);
				perror("execvp failed\n");
				exit(1);
			}

			exit(1);
		}                //*  CHILD PROCESS doesn't go beyond here, it executes execvp    <<<=====================

		if(child_ID != 0){
			addProcess(&globalProcessList, pCmdLine, child_ID);
			freeCmdLines(pCmdLine);
		}
		
		
		if(pCmdLine->blocking != 0){
			waitpid(child_ID, &status, 0); // https://stackoverflow.com/questions/21248840/example-of-waitpid-in-use
			updateProcessStatus(globalProcessList, child_ID, TERMINATED);
		}
	}
	
}







void freeAll(){            //*   function to free the lists
	deleteALLStringHistory();
	freeProcessList(globalProcessList);
}






void CMDHistoryPrinter(){  // part 4
	if (UnparsedCommandHistory != NULL)
	{
		commandLineString *currCMD = UnparsedCommandHistory->head;
		int i = 1;
		while(currCMD != NULL){
			printf("%d) %s\n", i, currCMD -> commandString);
			currCMD = currCMD -> next;
			i++;
		}
	}
}



void exclamationExclamation(){  // part 4
	if(UnparsedCommandHistory == NULL){
		fprintf(stderr, "Current Command History is empty, cannot execute command: \"!!\"\n");
	}else{
		char *commandToExecute = UnparsedCommandHistory->tail->commandString;
		addToStringHistory(commandToExecute);
		cmdLine *newParsed = parseCmdLines(commandToExecute);
		execute(newParsed);
		freeCmdLines(newParsed);
	}
}




void exclamationN(int n){  // part 4
	if(UnparsedCommandHistory == NULL){
		fprintf(stderr, "Current Command History is empty, cannot execute command: !n\n");
	}
	else{
		commandLineString *currCMD = UnparsedCommandHistory->head;

		if(n > 10 || n < 1 || n > UnparsedCommandHistory -> size){
			fprintf(stderr, "n is out of history bounds in !n command\n");
			return;
		}

		int i = 1;
		while(i <= n-1){
			if(currCMD == NULL){
				fprintf(stderr, "n is out of history bounds in !n command\n");
				return;
			}
			i++;
			currCMD = currCMD -> next;
		}
		char *commandToExecute = currCMD->commandString;
		addToStringHistory(commandToExecute);
		cmdLine *newParsed = parseCmdLines(commandToExecute);
		execute(newParsed);
		freeCmdLines(newParsed);
	}
}






void addToStringHistory(char *commandString){   // add in the main
	if (commandString[0] == '!')
	{
		return;
	}
	
	commandLineString *newCommand = (commandLineString*)malloc(sizeof(commandLineString));
	// newCommand->commandString = commandString;
	newCommand->commandString = strdup(commandString);  // works
	newCommand->next = NULL;

	if(UnparsedCommandHistory == NULL){
		UnparsedCommandHistory = (commandLineStringsList*)malloc(sizeof(commandLineStringsList));
		UnparsedCommandHistory->head = newCommand;
		UnparsedCommandHistory->size = 1;
		UnparsedCommandHistory->tail = newCommand;
	}else{
		if(UnparsedCommandHistory->size == HISTLEN){
			deleteFromStringHistoryHead();
		}
		UnparsedCommandHistory->tail->next = newCommand;
		UnparsedCommandHistory->tail = newCommand;
		UnparsedCommandHistory->size = UnparsedCommandHistory->size + 1;
	}

}



void deleteFromStringHistoryHead(){
	if(UnparsedCommandHistory != NULL){
		commandLineString *tmpCMD = UnparsedCommandHistory -> head;
		UnparsedCommandHistory -> head = UnparsedCommandHistory -> head -> next;
		UnparsedCommandHistory->size = UnparsedCommandHistory->size - 1;

		if(UnparsedCommandHistory->head == NULL){  //  if there is nothing in the history, let's delete everything inside and free the history list
			if (UnparsedCommandHistory->tail != NULL)
			{
				free(UnparsedCommandHistory->tail);
			}
			if (UnparsedCommandHistory->head != NULL)
			{
				free(UnparsedCommandHistory->head);
			}
			free(UnparsedCommandHistory);
			UnparsedCommandHistory = NULL;
		}

		tmpCMD->next = NULL;   // freeing the temp
		free(tmpCMD -> commandString);
		free(tmpCMD);
	}
}


void deleteALLStringHistory(){
	while(UnparsedCommandHistory != NULL){
		deleteFromStringHistoryHead();
	}
}
















void freeProcessList(process *process_list) {  // Part 3b
    if (process_list == NULL) {
        return;
    }

    process *current = process_list;
    process *next = NULL;

    while (current != NULL) {
        next = current->next;
        if (current->cmd != NULL) {
            freeCmdLines(current->cmd);
        }
        free(current);
        current = next;
    }
    process_list = NULL;
}





void updateProcessList(process **process_list){  // Part 3b
	if(*process_list != NULL){
		process *currProcess = *process_list;
		int status = 0;
		while (currProcess != NULL)
		{
			if(waitpid(currProcess->pid, &status, WNOHANG | WUNTRACED | __W_CONTINUED) > 0){
				if(WIFEXITED(status) || WIFSIGNALED(status)){
					updateProcessStatus(currProcess, currProcess -> pid, TERMINATED);
				}
				if(WIFSTOPPED(status)){
					updateProcessStatus(currProcess, currProcess -> pid, SUSPENDED);
				}
				if(WIFCONTINUED(status)){
					updateProcessStatus(currProcess, currProcess -> pid, RUNNING);
				}
			}
			currProcess = currProcess -> next;
		}
	}
}




void updateProcessStatus(process* process_list, int pid, int status){  // Part 3b
	if(process_list != NULL){
		process *currProcess = process_list;
		while (currProcess != NULL)
		{
			if(currProcess->pid == pid){
				currProcess->status = status;
			}
			currProcess = currProcess->next;
		}
	}
}












void addProcess(process** process_list, cmdLine* cmd, pid_t pid){  // Part 3a
	process* newProcess = (process*)malloc(sizeof(process));
	newProcess -> cmd = cmd;
	newProcess -> pid = pid;
	newProcess -> status = RUNNING;
	newProcess -> next = NULL;
	
	if (*process_list == NULL){
		*process_list = newProcess;
	}
	else
	{
		process *currProc = *process_list;
		while(currProc -> next != NULL){
			currProc = currProc -> next;
		}
		currProc -> next = newProcess;
	}
}


					
          
void printProcessList(process** process_list){  // Part 3a
	updateProcessList(process_list);
	printf("index \t\t PID \t\t Command \t STATUS\n");
	fflush(stdout);
	int index = 0;
	if (process_list != NULL){
		process *currProc = *process_list;
		while(currProc != NULL){

			char *status = NULL;
			if(currProc -> status == TERMINATED){
				status = "TERMINATED";
			}
			else if(currProc -> status == SUSPENDED){
				status = "SUSPENDED";
			}
			else{
				status = "RUNNING";
			}

			printProcess(currProc, index, status);
			currProc = currProc -> next;
			index++;
		}
	}

	deleteTerminatedFromList(process_list);
}



void deleteTerminatedFromList(process **process_list){
	process *prevProcess = NULL;
	process *currProcess = *process_list;

	while(currProcess != NULL){
		if(currProcess -> status != TERMINATED){
			prevProcess = currProcess;
			currProcess = currProcess -> next;
		}
		else{
			if(prevProcess == NULL){
				if(currProcess -> next != NULL){
					*process_list = currProcess -> next;
					freeCmdLines(currProcess -> cmd);
					free(currProcess);
					currProcess = *process_list;
				}
				else{
					freeCmdLines(currProcess -> cmd);
					free(currProcess);
					*process_list = NULL;
					currProcess = NULL;
				}
			}
			else{
				if(currProcess -> next != NULL){
					prevProcess -> next = currProcess -> next;
					freeCmdLines(currProcess -> cmd);
					free(currProcess);
					currProcess = prevProcess -> next;
				}
				else{
					freeCmdLines(currProcess -> cmd);
					free(currProcess);
					prevProcess -> next = NULL;
					currProcess = NULL;
				}
			}	
		}
	}	
}







void printProcess(process* proc, int index, char *status)
{
	printf("%d) \t\t %d \t\t", index, proc->pid);
	for (int i = 0; i < proc->cmd->argCount; i++)
	{
		printf(" %s", proc->cmd->arguments[i]);
	}
    printf(" \t\t %s\n", status);
}






















void createPipe(cmdLine *pCmdLine, cmdLine *nextpCmdLine){
    int status = 0;
    int pipeFD[2];

    if(pipe(pipeFD) == -1){
        fprintf(stderr, "Failed to pipe\n");
		freeCmdLines(pCmdLine);
		freeAll();
        exit(1);
    }

    int ProcId1 = fork();  // child 1 creation
    if(ProcId1 == -1){
        fprintf(stderr, "Error during fork\n");
		freeCmdLines(pCmdLine);
		freeAll();
        exit(1);
    }else if (ProcId1 == 0)
    {
        close(STDOUT_FILENO);
        int writeEndFD = dup(pipeFD[1]);
        if(writeEndFD == -1){
            fprintf(stderr, "error in dup in first child when pipelining");
            close(pipeFD[1]);
            close(pipeFD[0]);
			freeCmdLines(pCmdLine);
			//freeAll();
            exit(1);
        }
        close(pipeFD[1]);

		char *inputFile = pCmdLine -> inputRedirect;    //*  input redirect of the first/left process if there is redirection
		int inputFileDesc;
		if(inputFile != NULL){
			inputFileDesc = open(inputFile, O_RDONLY); // https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
			if(inputFileDesc == -1){
				perror("Input file Error\n");
				freeCmdLines(pCmdLine);
				//freeAll();
				exit(1);
			}

			if(dup2(inputFileDesc, 0) == -1){  // https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
				perror("dup 2 at input failed\n");
				freeCmdLines(pCmdLine);
				if(inputFileDesc != NULL){
					close(inputFileDesc);
				}
				//freeAll();
				exit(1);
			}
		}                           //*  input redirect of the first/left process if there is redirection

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            fprintf(stderr, "error in execvp in first child when pipelining");
            close(pipeFD[1]);
            close(pipeFD[0]);
			if(inputFileDesc != NULL){
				close(inputFileDesc);
			}
			freeCmdLines(pCmdLine);
            exit(1);
        }
        
        close(pipeFD[0]);
        close(writeEndFD);
		if(inputFileDesc != NULL){
			close(inputFileDesc);
		}
		
    }
    else{
        close(pipeFD[1]); 
		addProcess(&globalProcessList, pCmdLine, ProcId1);

        int ProcId2 = fork();  // child 2

        if(ProcId2 == -1){
            fprintf(stderr, "Error during fork2\n");
			freeCmdLines(pCmdLine);
			freeAll();
            exit(1);
        }else if (ProcId2 == 0){
            close(STDIN_FILENO);
            int readEndFD = dup(pipeFD[0]);
            if(readEndFD == -1){
                fprintf(stderr, "error in dup in second child when pipelining");
                close(pipeFD[1]);
                close(pipeFD[0]);
				freeCmdLines(pCmdLine);
				//freeAll();
                exit(1);
            }
            close(pipeFD[0]);


			char *outputFile = nextpCmdLine -> outputRedirect;       //*  output redirect of the second/right process if there is redirection
            int OutputFileDesc;
			if(outputFile != NULL){
                OutputFileDesc = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC);
                if(OutputFileDesc == -1){
                    perror("Output file Error\n");
                    freeCmdLines(pCmdLine);
					//freeAll();
                    exit(1);
                }

				// https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
                if(dup2(OutputFileDesc, 1) == -1){
                    perror("dup2 at output failed\n");
                    freeCmdLines(pCmdLine);
					if (OutputFileDesc != NULL)
					{
						close(OutputFileDesc);
					}
					//freeAll();
                    exit(1);
                }
			}                                     //*  output redirect of the second/right process if there is redirection


            if (execvp(nextpCmdLine -> arguments[0], nextpCmdLine -> arguments) == -1)
            {
                fprintf(stderr, "error in execvp in second child when pipelining");
                close(pipeFD[1]);
                close(pipeFD[0]);
				if (OutputFileDesc != NULL)
				{
					close(OutputFileDesc);
				}
				freeCmdLines(pCmdLine);
                exit(1);
            }
            close(pipeFD[1]);
            close(readEndFD);
			if (OutputFileDesc != NULL)
			{
				close(OutputFileDesc);
			}
        }
        else{
            close(pipeFD[0]);
			addProcess(&globalProcessList, nextpCmdLine, ProcId2);

            if(waitpid(ProcId1, &status, 0) == -1){
                fprintf(stderr, "Error in the child1 proccess wait\n");
				freeCmdLines(pCmdLine);
				//freeAll();
                exit(1);
            }
			updateProcessStatus(globalProcessList, ProcId1, TERMINATED);

            if(waitpid(ProcId2, &status, 0) == -1){
                fprintf(stderr, "Error in the child2 proccess wait\n");
				freeCmdLines(pCmdLine);
				//freeAll();
                exit(1);
            }
			updateProcessStatus(globalProcessList, ProcId2, TERMINATED);

        }
    }
}






















void DebugModeHandling(int argc, char **argv){   //  my line
    isDebugMode = false;  // default = true as specified in the assignment   //  my line
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i], "-d") == 0)
        {
			isDebugMode = true;
			break;
        }
    }

	if(isDebugMode){
		for(int i=0; i<argc; i++){
			fprintf(stderr, "myshell argument: %s\n",argv[i]);
			fflush(stderr);
		}
	}

}


void handler(int sig)  // <<<====================   from lab 2
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
	else if (sig == SIGINT)
	{
		freeAll();
		signal(SIGINT, SIG_DFL);
		signal(SIGCONT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		exit(0);
	}
	
	signal(sig, SIG_DFL);
	raise(sig);
}




