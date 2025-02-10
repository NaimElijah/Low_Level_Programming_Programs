#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#define SIZEMSG 255

void myPipe();

int main(int argc, char **argv)
{
    myPipe();
	return 0;
}


void myPipe(){
    int status;
    int pipeFD[2];

    if(pipe(pipeFD) == -1){
        fprintf(stderr, "Failed to pipe\n");
        fprintf(stderr, "(parent_process>exiting…)\n");
        exit(1);
    }

    fprintf(stderr, "(parent_process>forking child 1)\n");
    int ProcId1 = fork();  // child 1 creation
    if (ProcId1 != -1 && ProcId1 != 0)
    {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", ProcId1);
    }
    
    if(ProcId1 == -1){
        fprintf(stderr, "Error during fork\n");
        fprintf(stderr, "(parent_process>exiting…)\n");
        exit(1);
    }else if (ProcId1 == 0)
    {
        close(STDOUT_FILENO);
        int writeEndFD = dup(pipeFD[1]);
        if(writeEndFD == -1){
            fprintf(stderr, "error in dup in first child when pipelining");
            close(pipeFD[1]);
            close(pipeFD[0]);
            exit(1);
        }
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        close(pipeFD[1]);
        char *argsP1[] = {"ls", "-l", NULL};
        fprintf(stderr, "(child1>going to execute cmd: ...)\n");
        if (execvp(argsP1[0], argsP1) == -1)
        {
            fprintf(stderr, "error in execvp in first child when pipelining");
            close(pipeFD[1]);
            close(pipeFD[0]);
            exit(1);
        }
        
        close(pipeFD[0]);
        close(writeEndFD);
    }
    else{
        fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
        close(pipeFD[1]); 

        fprintf(stderr, "(parent_process>forking child 2)\n");
        int ProcId2 = fork();  // child 2
        if (ProcId2 != -1 && ProcId2 !=0)
        {
            fprintf(stderr, "(parent_process>created process with id: %d)\n", ProcId2);
        }

        if(ProcId2 == -1){
            fprintf(stderr, "Error during fork2\n");
            fprintf(stderr, "(parent_process>exiting…)\n");
            exit(1);
        }else if (ProcId2 == 0){
            close(STDIN_FILENO);
            int readEndFD = dup(pipeFD[0]);
            if(readEndFD == -1){
                fprintf(stderr, "error in dup in second child when pipelining");
                close(pipeFD[1]);
                close(pipeFD[0]);
                exit(1);
            }
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
            close(pipeFD[0]);
            char *argsP2[] = {"tail", "-n", "2", NULL};
            fprintf(stderr, "(child2>going to execute cmd: ...)\n");
            if (execvp(argsP2[0], argsP2) == -1)
            {
                fprintf(stderr, "error in execvp in second child when pipelining");
                close(pipeFD[1]);
                close(pipeFD[0]);
                exit(1);
            }
            close(pipeFD[1]);
            close(readEndFD);
        }
        else{
            fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
            close(pipeFD[0]);

            fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
            if(waitpid(ProcId1, &status, 0) == -1){
                fprintf(stderr, "Error in the child1 proccess wait\n");
                fprintf(stderr, "(parent_process>exiting…)\n");
                exit(1);
            }
            if(waitpid(ProcId2, &status, 0) == -1){
                fprintf(stderr, "Error in the child2 proccess wait\n");
                fprintf(stderr, "(parent_process>exiting…)\n");
                exit(1);
            }
        }
    }
    exit(0);
}



