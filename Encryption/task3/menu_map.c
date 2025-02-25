#include <stdlib.h>       // I think this was already in here
#include <stdio.h>       // I think this was already in here
#include <string.h>       // I saw in the internet in geekforgeeks that I need this for some functionalities I needed to use
#include <ctype.h>       // I saw in the internet in geekforgeeks that I need this for some functionalities I needed to use
#include <stdbool.h>       // I saw in the internet in geekforgeeks that I need this for some functionalities I needed to use

char my_get(char c); /* Ignores c, reads and returns a character from stdin using fgetc. */       //   function declaraction given in the assignment
char cprt(char c); /* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */    //   function declaraction given in the assignment
char encrypt(char c); /* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x1F and 0x7E it is returned unchanged */    //   function declaraction given in the assignment
char decrypt(char c); /* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x21 and 0x7F it is returned unchanged */    //   function declaraction given in the assignment
char xprt(char c); /* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */    //   function declaraction given in the assignment
char dprt(char c); /* dprt prints the value of c in a decimal representation followed by a new line, and returns c unchanged. */    //   function declaraction given in the assignment
void menu();    //  my line

typedef struct func_data{    //   struct shape given in the assignment, but I already knew how to do this
  char *FuncName;    //   struct shape given in the assignment, but I already knew how to do this
  char (*funcP)(char);    //   struct shape given in the assignment, but I already knew how to do this
}func_data;    //   struct shape given in the assignment, but I already knew how to do this

func_data functions[] = { { "Get string", &my_get }, { "Print string", &cprt }, { "Encrypt", &encrypt }, { "Decrypt", &decrypt }, { "Print Hex", &xprt }, { "Print Dec", &dprt }, { NULL, NULL } };
//   array shape given in the assignment, but I already knew how to do this

char* map(char *array, int array_length, char (*f) (char)){   //  line was already here
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));   //  line was already here
  /* TODO: Complete during task 2.a */   //  line was already here
  for (int i = 0; i < array_length; i++)    //  my line
  {    //  my line
    mapped_array[i] = f(array[i]);    //  my line
  }    //  my line
  return mapped_array;    //  my line
}    //  my line

int main(int argc, char **argv){    //  line was already here
  menu();    //  my line
  return 0;    //  my line
}









void menu(){    //  my line
  int bounds = 5;    //  my line

  char *input = (char*)(malloc(bounds*sizeof(char)));   //  saw in "man" what it gives me and searched up the internet for examples for using malloc: https://www.geeksforgeeks.org/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/
  
  char *carray = (char*)(calloc(bounds, sizeof(char)));   //  saw in "man" what it gives me and searched up the internet for examples for using calloc: https://www.geeksforgeeks.org/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/

  printf("Select operation from the following menu (ctrl^D for exit):\n");  //  printing according to what's needed to be printed as specified in the assignment and in the example
  for (int i = 0; i < (sizeof(functions)/sizeof(*functions)) - 1; i++)    //  my line
  {    //  my line
    printf("%d)  %s\n", i, functions[i].FuncName);    //  my line   //  printing according to what's needed to be printed as specified in the assignment and in the example
  }    //  my line
  printf("Option : ");     //  printing according to what's needed to be printed as specified in the assignment and in the example

  int atoi_save = 0;    //  my line
  while((input = fgets(input, bounds*sizeof(char), stdin)) != NULL)    //  my line
  {    //  my line
    atoi_save = atoi(input);    //  my line
    if (0 <= atoi_save && atoi_save <= 5)    //  my line
    {    //  my line
      printf("\nWithin bounds\n");    //  my line   //  printing according to what's needed to be printed as specified in the assignment and in the example
    }else    //  my line
    {    //  my line
      printf("\nNot within bounds\n");      //  my line   //  printing according to what's needed to be printed as specified in the assignment and in the example
      free(carray);      //  my line
      free(input);      //  my line
      exit(0);      //  my line
    }      //  my line
    char *tempForFreeing = carray;      //  my line
    carray = map(carray, bounds, functions[atoi(input)].funcP);      //  my line
    free(tempForFreeing);      //  my line

    printf("DONE.\n\n");      //  my line

    printf("Select operation from the following menu (ctrl^D for exit):\n");   //  printing according to what's needed to be printed as specified in the assignment and in the example
    for (int i = 0; i < (sizeof(functions)/sizeof(*functions)) - 1; i++)      //  my line
    {      //  my line
      printf("%d)  %s\n", i, functions[i].FuncName);      //  my line   //  printing according to what's needed to be printed as specified in the assignment and in the example
    }      //  my line

    printf("Option : ");   //  printing according to what's needed to be printed as specified in the assignment and in the example

  }
  
  free(carray);      //  my line
  free(input);      //  my line
  exit(0);      //  my line
  
}






char my_get(char c){    /* Ignores c, reads and returns a character from stdin using fgetc. */   // function stamp from the assignment
  char res = fgetc(stdin);      //  my line
  return res;      //  my line
}

char cprt(char c){    /* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */    // function stamp from the assignment
  if (0x20 <= c && c <= 0x7E){      //  my line                                                                        // function stamp from the assignment
    printf("%c\n", c);      //  my line
  }else{      //  my line
    printf(".\n");      //  my line
  }      //  my line
  return c;      //  my line
}


char encrypt(char c){    /* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x1F and 0x7E it is returned unchanged */     // function stamp from the assignment
  if (0x1F <= c && c <= 0x7E){      //  my line
    return c+1;      //  my line
  }else{      //  my line
    return c;      //  my line
  }
}


char decrypt(char c){    /* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x21 and 0x7F it is returned unchanged */    // function stamp from the assignment
  if (0x21 <= c && c <= 0x7F){      //  my line
    return c-1;      //  my line
  }else{      //  my line
    return c;      //  my line
  }
}

char xprt(char c){    /* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */     // function stamp from the assignment
  printf("%x\n", c);      //  my line, learned the format specifiers in the geeksforgeeks website: https://www.geeksforgeeks.org/format-specifiers-in-c/
  return c;      //  my line
}

char dprt(char c){    /* dprt prints the value of c in a decimal representation followed by a new line, and returns c unchanged. */     // function stamp from the assignment
  printf("%d\n", c);      //  my line
  return c;      //  my line
}

