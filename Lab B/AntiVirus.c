#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

const int X=8;
char *buff;
char *infectedFileTofix = NULL;
char *input = NULL;
bool IsLittleEndian = true;  // for later, check before calling readVirus.

typedef struct virus {
    unsigned short SigSize;   // 2 Bytes
    char virusName[16];  // 16 Bytes
    unsigned char* sig;  // N/SigSize amount of Bytes
} virus;

typedef struct link link;
struct link {
    link *nextVirus;
    virus *vir;
};

link *virusList = NULL;      //*                 <<<<<<================================================   Global virus list

typedef struct func_data{    //   struct shape given in the assignment, but I already knew how to do this
  char *FuncName;    //   struct shape given in the assignment, but I already knew how to do this
  void (*funcP)();    //   struct shape given in the assignment, but I already knew how to do this
}func_data;    //   struct shape given in the assignment, but I already knew how to do this

//Wrapper functions
void entryLoadSignatures();
void entryPrintSignatures();
void entryDetectViruses();
void entryFixFile();
void entryQuit();
void quit();

func_data functions[] = { { "Load signatures", &entryLoadSignatures }, { "Print signatures", &entryPrintSignatures }, { "Detect viruses", &entryDetectViruses }, { "Fix file", &entryFixFile }, {"Quit", &entryQuit}, { NULL, NULL } };

void PrintHex(char *buffer, int length);
void printHexInFile(char *buffer, int length, FILE *output);
void hexaPrint(char *fileN);

virus* readVirus(FILE *file);
void printVirus(virus *virus, FILE *output);

void detect_virus(char *buffer, unsigned int size, link *virus_list);
void neutralize_virus(char *fileName, int signatureOffset);

void list_print(link *virus_list, FILE *file);
link* list_append(link* virus_list, virus* data);
void list_free(link *virus_list);
void virusDetector();

int main(int argc, char **argv)           //*              <<<<<============================================    MAIN
{
    
    virusDetector();
    
    fflush(stdout);   //  my line, eyal did this(fflush) at the end of lecture 1
    fflush(stdin);   //  my line, eyal did this(fflush) at the end of lecture 1
    fflush(stderr);   //  my line, eyal did this(fflush) at the end of lecture 1
    return 0;   //  my line
}


// Menu Based on Lab 1
void virusDetector(){            //*             <<<<<<=======================================    MENU function
    int inputBounds = 60;
    int menu_size = sizeof(functions)/sizeof(*functions) - 1;

    input = (char*)(malloc(inputBounds*sizeof(char)));  // menu number selection
    if (!input) {
        free(input);
        quit();
        perror("Failed to allocate memory for Menu's input");
        exit(1);
    }

    printf("Select operation from the following menu (ctrl^D for exit):\n");
    for (int i = 1; i < menu_size + 1; i++)
    {
        printf("%d)  %s\n", i, functions[i - 1].FuncName);
    }
    printf("Option : ");

    int atoi_save = 0;
    while((input = fgets(input, inputBounds*sizeof(char), stdin)) != NULL)
    {
        atoi_save = atoi(input); //https://www.geeksforgeeks.org/c-atoi-function/
        if (1 <= atoi_save && atoi_save <= menu_size)
        {
            printf("\nWithin bounds\n");   //  printing according to what's needed to be printed as specified in the assignment and in the example
        }else
        {
            printf("\nNot within bounds\n");   //  printing according to what's needed to be printed as specified in the assignment and in the example
            free(input);
            quit();
            exit(0);
        }
        
        functions[atoi_save - 1].funcP();   //* the brain, the function execution.

        printf("DONE.\n\n");

        printf("Select operation from the following menu (ctrl^D for exit):\n");
        for (int i = 1; i < menu_size + 1; i++)
        {
            printf("%d)  %s\n", i, functions[i - 1].FuncName);
        }
        printf("Option : ");

    }

    list_free(virusList);
    free(input);
    quit();
    exit(0);
}


void entryLoadSignatures(){
    int fileParamaterSize = 255;
    char *fileInput = (char*)(malloc(fileParamaterSize));

    printf("Enter the file name of the Virus Signatures: ");
    if ((fileInput = fgets(fileInput, fileParamaterSize, stdin)) != NULL)
    {
        int len = strlen(fileInput); //https://www.geeksforgeeks.org/strlen-function-in-c/
        if (len > 0 && fileInput[len - 1] == '\n') {
            fileInput[len - 1] = '\0';
        }

        FILE *signaturesFile = fopen(fileInput, "rb"); //https://www.geeksforgeeks.org/c-fopen-function-with-examples/
        if(signaturesFile == NULL){
            perror("Failed to read the signatures file\n");
            free(fileInput);
            quit();
            exit(1);
        }

        char buffer[5];
        fread(buffer, 1, 4, signaturesFile);
        buffer[4] = '\0';

        if (strcmp(buffer, "VIRB") == 0)
        {
            IsLittleEndian = false;
        }else if (strcmp(buffer, "VIRL") == 0) //https://www.geeksforgeeks.org/strcmp-in-c/
        {
            IsLittleEndian = true;
        }
        else{
            perror("Magic number is incorrect\n");
            fclose(signaturesFile);
            free(fileInput);
            quit();
            exit(1);
        }
        
        virus *v;
        while((v = readVirus(signaturesFile)) != NULL){
            virusList = list_append(virusList, v);
        }
        
        fclose(signaturesFile);
    }
    free(fileInput);
    
}


void entryPrintSignatures(){
    if(virusList != NULL){
        list_print(virusList, stdout);
    }
}


void entryDetectViruses(){
    int fileParamaterSize = 255;

    printf("Enter the file name of the \"suspected\" file: ");
    if(infectedFileTofix != NULL){
        free(infectedFileTofix);
    }
    infectedFileTofix = (char*)(malloc(fileParamaterSize));
    
    if ((infectedFileTofix = fgets(infectedFileTofix, fileParamaterSize, stdin)) != NULL)
    {
        int len = strlen(infectedFileTofix);
        if (len > 0 && infectedFileTofix[len - 1] == '\n') {
            infectedFileTofix[len - 1] = '\0';
        }

        FILE *suspectedFile = fopen(infectedFileTofix, "rb");
        if(suspectedFile == NULL){
            perror("Failed to read the suspected file\n");
            free(infectedFileTofix);
            quit();
            exit(1);
        }

        int bufferSize = 10000;
        char buffer[bufferSize];
        int fileSize = fread(buffer, sizeof(char), bufferSize, suspectedFile); //https://www.geeksforgeeks.org/fread-function-in-c/
        detect_virus(buffer, fileSize, virusList);
        fclose(suspectedFile);
    }
}


void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link *currLink = virus_list;  // runner
    while(currLink != NULL){
        virus *currVirus = currLink -> vir;
        for(int position = 0; position < size; position++){
            if(memcmp(currVirus -> sig, &buffer[position], currVirus -> SigSize) == 0){ //https://www.w3schools.com/c/ref_string_memcmp.php
                printf("The starting byte location in the suspected file: %d\n", position);
                printf("The virus's name: %s\n", currVirus -> virusName);
                printf("The virus's signature size: %d\n", currVirus -> SigSize);
                printf("\n");
            }
        }
        currLink = currLink -> nextVirus;
    }
}


void entryFixFile(){
    //entryDetectViruses();  //    not needed because 3 in the menu already done.
    
    if(infectedFileTofix == NULL){
        perror("Failed to open the file(while using Fix File), first execute \"3. Detect Virus\" before using \"4. Fix File\"\n");
        return;
    }
    
    int bufferSize = 10000;
    char buffer[bufferSize];
    
    FILE* fileTempForReading = fopen(infectedFileTofix, "rb");
    int fileSize = fread(buffer, sizeof(char), bufferSize, fileTempForReading); //https://www.geeksforgeeks.org/fread-function-in-c/
    fclose(fileTempForReading);

    link *currLink = virusList;  // runner
    while(currLink != NULL){
        virus *currVirus = currLink -> vir;
        for(int position = 0; position < fileSize; position++){
            if(memcmp(currVirus -> sig, &buffer[position], currVirus -> SigSize) == 0){  // https://www.w3schools.com/c/ref_string_memcmp.php
                neutralize_virus(infectedFileTofix, position);
            }
        }
        currLink = currLink -> nextVirus;
    }
}


void neutralize_virus(char *fileName, int signatureOffset){
    FILE *fileToFix = fopen(infectedFileTofix, "wb+");
    if(fileToFix == NULL){
        perror("Failed to open the file(in neutralize_virus)\n");
        quit();
        exit(1);
    }
    if(fileName == NULL){
        perror("Failed to open the file in neutralize_virus\n");
        quit();
        exit(1);
    }
    if(fseek(fileToFix, signatureOffset, 0) != 0){ //https://www.geeksforgeeks.org/fseek-in-c-with-example/
        perror("Error during fseek in neutralize_virus\n");
        fclose(fileToFix);
        quit();
        exit(1);
    }
    char *neutralizer = "0xC3";
    fwrite(neutralizer, 1, 1, fileToFix);

    fclose(fileToFix);
}



void entryQuit(){
    quit();
    exit(0);
}



void quit(){
    list_free(virusList);
    if(infectedFileTofix != NULL){
        free(infectedFileTofix);
        infectedFileTofix = NULL;
    }
    if(input != NULL){
        free(input);
        input = NULL;
    }
}




virus* readVirus(FILE *file){   // starting already from a virus
    virus *virus = malloc(sizeof(*virus));

    
    if(fread(virus, sizeof(char), 18, file) != 18){
        free(virus);
        if(feof(file)){ //https://www.geeksforgeeks.org/eof-and-feof-in-c/
            return NULL;
        }
        else{
            perror("Failed to read the first 18 bytes of the virus\n");
            quit();
            exit(1);
        }
    }
    else{
        if (!IsLittleEndian){
            virus -> SigSize = ((virus -> SigSize) >> 8) | ((virus -> SigSize) << 8); //   https://www.geeksforgeeks.org/bit-manipulation-swap-endianness-of-a-number/
        }
        
        virus -> sig = malloc(virus -> SigSize);
        if(fread(virus -> sig, sizeof(char), virus -> SigSize, file) != virus -> SigSize){
            free(virus);
            perror("Failed to read the bytes of the signature of the virus\n");
            quit();
            exit(1);
        }
    }
    
    return virus;
}


void list_print(link *virus_list, FILE *file){
    link *runner = virus_list;
    while (runner != NULL)
    {
        printVirus(runner -> vir, file);
        runner = runner -> nextVirus;
    }
    
}


link* list_append(link* virus_list, virus* data){   // appending to the front
    link *res = malloc(sizeof(struct link));
    res -> nextVirus = virus_list;
    res -> vir = data;
    return res;
}


void list_free(link *virus_list){
    if(virus_list != NULL){
        free(virus_list -> vir -> sig);
        free(virus_list -> vir);
        list_free(virus_list -> nextVirus);
        free(virus_list);
    }
}


void printVirus(virus *virus, FILE *output){
    fprintf(output, "Virus name: %s\n", virus -> virusName);
    fprintf(output, "Virus signature length: %d\n", virus -> SigSize);
    printHexInFile(virus -> sig, virus -> SigSize, output);
    printf("\n");
}


void printHexInFile(char *buffer, int length, FILE *output){
    int line = 0;
    for (int i = 0; i < length; i++) {
        fprintf(output, "%02X ", (unsigned char)buffer[i]);
        line++;
        
        if(line == 20){
            line = 0;
            fprintf(output, "\n");
        }
    }
    if(line != 20 && line != 0){
        fprintf(output, "\n");
    }
    
}







