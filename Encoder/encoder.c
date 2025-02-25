#include <stdlib.h>   //  my line
#include <stdio.h>   //  my line
#include <string.h>   //  in order to use the string functions we're allowed to use according to the assignment, saw this include on the internet in geeksforgeeks
#include <stdbool.h>   //  in order to use booleans, saw this include on the internet in geeksforgeeks

bool IsDebugMode = true;  // default = true as specified in the assignment   //  my line

char *EncodingStringGlobal = "0";   //   making this global variable for the encoding, like suggested in the assignment   //  my line
char *EncodingPointerToCurrentNumber = NULL;   //   to go through the encoding key   //  my line
bool IsEncodingPlus = true;  // indicates if we need to plus or minus the numbers in the encoding   //  my line

bool IsInputFile = false;  // indicates where to take the input from, console or file   //  my line
bool IsOutputFile = false;  // indicates where to take the output from, console or file   //  my line
FILE *infile = NULL, *outfile = NULL;   // global variables for file data, suggested to make these global variables in the assignment   //  my line

void DebugModeHandling(int argc, char **argv);   //  my line
void EncodeHandling(int argc, char **argv);   //  my line
char encode(char c);   //  my line

int main(int argc, char **argv){
    DebugModeHandling(argc, argv);  // Debug Mode Handling   //  my line
    EncodeHandling(argc, argv);   // Encoding Handling   //  my line
    fflush(stdout);   //  my line, eyal did this(fflush) at the end of lecture 1
    fflush(stdin);   //  my line, eyal did this(fflush) at the end of lecture 1
    fflush(stderr);   //  my line, eyal did this(fflush) at the end of lecture 1
    return 0;   //  my line
}


//*   ==============================================>>>>>>>>       ****  Down here are all of the functions  ****       <<<<<<<<==============================================


void EncodeHandling(int argc, char **argv){   //  my line
    char *InputFileName = NULL, *OutputFileName = NULL;   //  my line

    char cInInput;  // for later input scanning   //  my line
    
    for (int i=1; i < argc; i++)   //*    <<<<<------------------   this iteration is to check the command flags   //  my line
    {
        if (strncmp(argv[i], "+E", 2) == 0){   // <<<------------  plus encoder   //  my line, func given in assignment
            IsEncodingPlus = true;   //  my line
            EncodingStringGlobal = &argv[i][2];   //  my line
        }else if (strncmp(argv[i], "-E", 2) == 0){   // <<<------------  minus encoder   //  my line, func given in assignment
            IsEncodingPlus = false;   //  my line
            EncodingStringGlobal = &argv[i][2];   //  my line
        }


        if (strncmp(argv[i], "-i", 2) == 0){   //  my line, func given in assignment
            InputFileName = &argv[i][2];   //  my line
            infile = fopen(InputFileName, "r");   //  my line
            if(infile == NULL){   //  my line
                fprintf(stderr, "Input file %s given doesnt exist\n", InputFileName);   //  my line
                if(outfile){   //  my line
                    fclose(outfile);   //  to close if the whole thing we want to do isn't valid, because there's a problem      //  my line
                }
                exit(0);   //  my line
            }
            IsInputFile = true;   //  my line
        }

        if (strncmp(argv[i], "-o", 2) == 0){   //  my line, func given in assignment
            OutputFileName = &argv[i][2];   //  my line
            outfile = fopen(OutputFileName, "w");   //  my line
            if(outfile == NULL){  // opens a file in case that file doesn't exist, but if there was a problem opening it then we might need this if     //  my line
                fprintf(stderr, "Output file %s given doesnt exist\n", OutputFileName);   //  my line
                if(IsInputFile){   //  my line
                    fclose(infile);   //  to close if the whole thing we want to do isn't valid, because there's a problem   //  my line
                }
                exit(0);   //  my line
            }
            IsOutputFile = true;   //  my line
        }

    }


    EncodingPointerToCurrentNumber = EncodingStringGlobal;   //  my line

    if(IsInputFile == false){   //  input from the console   //  my line
        while ((cInInput = fgetc(stdin)) != EOF)   //  my line
        {
            if (IsOutputFile == true)   //  my line
            {
                fputc(encode(cInInput), outfile);  //  output to a file   //  my line
                fflush(outfile);  // learned from lecture 1 and examples in https://www.geeksforgeeks.org/use-fflushstdin-c/
            }else{
                fputc(encode(cInInput), stdout);  //  output to the console   //  my line
            }     
        }
    }else if (IsInputFile == true)   //  input from a file   //  my line
    {
        while ((cInInput = fgetc(infile)) != EOF)   //  my line
        {
            if (IsOutputFile == true)   //  my line
            {
                fputc(encode(cInInput), outfile);  //  output to a file   //  my line
            }else{
                fputc(encode(cInInput), stdout);  // output to the console   //  my line
            } 
        }
        printf("\n");   //  my line
    }
    
    if(IsInputFile){   //  my line
        fclose(infile);   //  my line
    }   //  my line
    if (IsOutputFile)   //  my line
    {   //  my line
        fclose(outfile);   //  my line
    }   //  my line

    
}








char encode(char c){              //*      <<<<---------------------       the encoding function.       <<<<---------------------
    if(strcmp(EncodingStringGlobal, "0") == 0){  // check with the pointer to the current encoding char   //  my line, func given in assignment
        return c;   //  my line
    }   //  my line

    char res = c;  // if some kind of other character then it will remain that same character.   //  my line
    int PlusOrMinusAmount = *EncodingPointerToCurrentNumber - '0';  // now we got what to add/subtract   //  my line

    if('a' <= c && c <= 'z')    // lower case characters   //  my line
    {
        // handled the wrap arounds like in intro to cs, with modulo:

        if(IsEncodingPlus){   //  my line
            res = 'a' + ((c - 'a' + PlusOrMinusAmount) % 26);   //  my line
        }else{   //  my line
            res = 'a' + ((c - 'a' - PlusOrMinusAmount + 26) % 26);   //  my line
        }   //  my line

    }else if('A' <= c && c <= 'Z')    // upper case characters   //  my line
    {
        if(IsEncodingPlus){   //  my line
            res = 'A' + ((c - 'A' + PlusOrMinusAmount) % 26);   //  my line
        }else{   //  my line
            res = 'A' + ((c - 'A' - PlusOrMinusAmount + 26) % 26);   //  my line
        }   //  my line

    }else if ('0' <= c && c <= '9')    // number characters   //  my line
    {
        if(IsEncodingPlus){   //  my line
            res = '0' + ((c - '0' + PlusOrMinusAmount) % 10);   //  my line
        }else{   //  my line
            res = '0' + ((c - '0' - PlusOrMinusAmount + 10) % 10);   //  my line
        }   //  my line

    }
    
    EncodingPointerToCurrentNumber++;   //  my line
    if (*EncodingPointerToCurrentNumber == '\0')   //  my line
    {
        EncodingPointerToCurrentNumber = EncodingStringGlobal;  //  reseting to the start of the encoding key   //  my line
    }
    
    return res;   //  my line
}







void DebugModeHandling(int argc, char **argv){   //  my line
    IsDebugMode = true;  // default = true as specified in the assignment   //  my line
    for(int i=0; i<argc; i++){   //  my line
        if(strcmp(argv[i], "+D") == 0)   //  my line
        {
            fprintf(stderr, "cmd argument: %s\n", argv[i]);   //  my line
            IsDebugMode = true;   //  my line
        }else if (strcmp(argv[i], "-D") == 0)   //  my line
        {
            fprintf(stderr, "cmd argument: %s\n", argv[i]);   //  my line
            IsDebugMode = false;   //  my line
        }else if(IsDebugMode)   //  my line
        {
            fprintf(stderr, "cmd argument: %s\n", argv[i]);   //  my line
        }        
    }
    fflush(stdout);   //  my line
    fflush(stdin);   //  my line
    fflush(stderr);   //  my line
}


