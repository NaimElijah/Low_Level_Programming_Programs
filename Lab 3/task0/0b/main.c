#include "util.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define STDOUT 1
#define SEEK_SET 0
#define O_RDRW 2

extern int system_call();

int main (int argc , char* argv[], char* envp[]){
  system_call(SYS_WRITE,STDOUT, "Hello world!\n",strlen("Hello world!\n"));
  return 0;
}


