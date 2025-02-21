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
#define SIZEMSG 255

void myPipe();

int main(int argc, char **argv)
{
    myPipe();
	return 0;
}


void myPipe(){
    int pipeFD[2];
    char ReadBuffer[SIZEMSG];

    if(pipe(pipeFD) == -1){
        perror("Failed to pipe\n");
        exit(1);
    }

    int ProcId = fork();
    
    if(ProcId == -1){
        perror("Error during fork\n");
        exit(1);
    }else if (ProcId == 0)
    {
        close(pipeFD[0]);
        char *message = "hello";
        write(pipeFD[1], message, SIZEMSG); 
        close(pipeFD[1]);
    }
    else{
        close(pipeFD[1]);
        read(pipeFD[0], ReadBuffer, SIZEMSG);
        printf("The message received by the parent is: %s\n", ReadBuffer);
        close(pipeFD[0]);
    }
    exit(0);
}

