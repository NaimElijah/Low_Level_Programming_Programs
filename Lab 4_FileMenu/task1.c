#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
} state;

struct fun_desc
{
    char *name;
    void (*fun)(state* s);
};



void toggle_Debug_Mode(state* s){
    if(s->debug_mode == '0'){  // debug off
        printf("Debug flag now on\n");
        s->debug_mode = '1';
    }
    else{  // debug on
        printf("Debug flag now off\n");
        s->debug_mode = '0';
    }
}



void set_File_Name(state* s){
    char* fileName = NULL;
    char buffer[BUFSIZ];  // BUFSIZ = max buffer size
    printf("Please enter file name:\n");
    fgets(buffer, BUFSIZ, stdin);
    sscanf(buffer, "%ms", &fileName); // reads the file name from the buffer  //!  <<<<<<=========================   check why red.
    // "%ms" - allocates memory in order to avoid buffer overflow issues while reading the input from the user
    strcpy(s->file_name, fileName);
    if(s->debug_mode == '1'){  // debug mode is on
        fprintf(stderr, "Debug: file name set to %s\n", fileName);
    }
    free(fileName);
}



void set_Unit_Size(state* s){
    int num; // in order to save the file name the user entered
    printf("Please enter a number:\n");
    scanf("%d", &num);
    fgetc(stdin);
    if((num == 1) || (num == 2) || (num == 4)){
        s->unit_size = num;
    }else{
        printf("Invalid value\n");
    }
    if (s->debug_mode == '1')
    {
        fprintf(stderr, "Debug: set size to %d\n", num);
    }
}




void load_Into_Memory(state* s){
    if (strcmp(s->file_name, "") == 0)
    {
        printf("file name is empty\n");
        free(s);
        exit(1);
    }
    FILE* fileToRead = fopen(s->file_name, "r+");
    if (fileToRead == NULL)
    {
        printf("failed to open the file\n");
        free(s);
        exit(1);
    }

    char input[BUFSIZ];
    printf("Please enter <location> <length>\n");
    int location = 0;
    int length = 0;

    if (fgets(input, BUFSIZ, stdin) == NULL)
    {
        printf("Failed to read user input\n");
        free(s);
        exit(1);
    }

    if (sscanf(input, "%x %d", &location, &length) != 2)  // parsing location and length using sscanf
    {
        printf("Invalid input. Expected hexadecimal location and decimal length\n");
        free(s);
        exit(1);
    }

    if(s->debug_mode == '1'){
        printf("file name: %s\n", s->file_name);
        printf("location: %x\n", location);
        printf("length: %d\n", length);
    }

    fseek(fileToRead, location, SEEK_SET);  // copy from file_name starting at position location
    fread(s->mem_buf, s->unit_size, length, fileToRead);

    s->mem_count += length*s->unit_size;   //  updating bytes amount counter of mem_buf in s.
    printf("Loaded %d units into memory\n", length);
    fclose(fileToRead);
}




void toggle_Display_Mode(state* s){
    if (s->display_mode == '0')
    {
        s->display_mode = '1';
        printf("Display flag now on, hexadecimal representation\n");
    }
    else
    {
        s->display_mode = '0';
        printf("Display flag now off, decimal representation\n");
    }
}




//TODO
void file_Display(state* s){
    char input[BUFSIZ];
    int offset = 0;
    // unsigned int offset = 0;       //       <<<<<<<<=======================  maybe this    <<<==================
    int u = 0;
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    void* startPrint;
    unsigned char file_buffer[10000];
    // char* buffer = (char*)malloc(10000);       //      <<<====================  maybe like this   <<<===================

    FILE* fileToRead = fopen(s->file_name, "rb");

    if(fileToRead == NULL){
        printf("failed to open the file\n");
        free(s);
        exit(1);
    }

    printf("Enter file offset and length:\n");
    if (fgets(input, BUFSIZ, stdin) == NULL){
        printf("Failed to read user input\n");
        fclose(fileToRead);
        free(s);
        exit(1);
    }

    if (sscanf(input, "%x %d", &offset, &u) != 2){
        printf("Invalid input. Expected hexadecimal offset and decimal length\n");
        fclose(fileToRead);
        free(s);
        exit(1);
    }

    // Seek to the offset and read file contents into memory
    if (fseek(fileToRead, offset, SEEK_SET) != 0){
        printf("Failed to seek to the offset\n");
        fclose(fileToRead);
        free(s);
        exit(1);
    }


    size_t bytesRead = fread(file_buffer, s->unit_size, u, fileToRead);
    fclose(fileToRead);

    if (bytesRead == 0){
        printf("No data read from file\n");
        free(s);
        exit(1);
    }

    
    startPrint = file_buffer;
    // Print values from buffer, similar to memory_Display
    if (s->display_mode == '0') {  // Decimal
        printf("Decimal\n");
        printf("==========\n");
        for (int i = 0; i < u; i++) {
            printf(dec_formats[s->unit_size - 1], *((int*)startPrint));
            startPrint += s->unit_size;
        }
    } else {  // Hexadecimal
        printf("Hexadecimal\n");
        printf("==========\n");
        for (int i = 0; i < u; i++) {
            printf(hex_formats[s->unit_size - 1], *((int*)startPrint));
            startPrint += s->unit_size;
        }
    }

    // free(file_buffer);  not needed, this is on the stack   <<===========  I did this just as a reminder that doesn't need free.

    if (s->debug_mode == '1') {
        printf("file name: %s\n", s->file_name);
        printf("offset: %x\n", offset);
        printf("length: %d\n", u);
    }
}





void memory_Display(state* s){
    char input[BUFSIZ];
    int address = 0;
    int u = 0;
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    void* startPrint;
    FILE* fileToRead = fopen(s->file_name, "rb");
    if(fileToRead == NULL){
        printf("failed to open the file\n");
        free(s);
        exit(1);
    }

    printf("Please enter address and length:\n");
    if (fgets(input, BUFSIZ, stdin) == NULL)
    {
        printf("Failed to read user input\n");
        free(s);
        exit(1);
    }

    if (sscanf(input, "%x %d", &address, &u) != 2)  // parsing location and length
    {
        printf("Invalid input. Expected hexadecimal location and decimal length\n");
        free(s);
        exit(1);
    }

    if (address == 0)
    {
        startPrint = &(s->mem_buf);
    }
    else
    {
        startPrint = &address;
    }

    if (s->display_mode == '0')  // decimal
    {
        printf("Decimal\n");
        printf("==========\n");
        for (int i = 0; i < u; i++)
        {
            printf(dec_formats[s->unit_size - 1], *((int*)startPrint));
            startPrint += s->unit_size;
        }
    }
    else
    {  // hexadecimal
        printf("Hexadecimal\n");
        printf("==========\n");
        for (int i = 0; i < u; i++)
        {
            printf(hex_formats[s->unit_size -1], *((int*)startPrint));
            startPrint += s->unit_size;
        }
    }

    if (s->debug_mode == '1')
    {
        printf("file name: %s\n", s->file_name);
        printf("location: %x\n", address);
        printf("length: %d\n", u);
    }

    fclose(fileToRead);
}



void save_Into_File(state* s){
    int source =0;
    int target = 0;
    int length = 0;
    void* startPoint;
    char input[BUFSIZ];
    FILE* fileToWrite = fopen(s->file_name, "r+");

    if (fileToWrite == NULL)
    {
        printf("failed to open the file\n");
        free(s);
        exit(1);
    }

    printf("Please enter <source-address> <target-location> <length>\n");
    if(fgets(input, BUFSIZ, stdin) == NULL){
        printf("failed to read user input\n");
        free(s);
        exit(1);
    }

    if(sscanf(input, "%x %x %d", &source, &target, &length) != 3){
        printf("Invalid input. Expected hexadecimal source, hexadecimal target, and decimal length.\n");
        free(s);
        exit(1);
    }

    if (s->debug_mode == '1')
    {
        printf("file name: %s\n", s->file_name);
        printf("source address: %x\n", source);
        printf("target location: %x\n", target);
        printf("length: %d\n", length);
    }

    int fileSize = 0;
    fseek(fileToWrite, 0, SEEK_END); // end of file
    fileSize = ftell(fileToWrite);  // current file pointer
    if (target > fileSize)
    {
        printf("Error: target location is out of bounds of the size of file");
    }
    else // do only if target is smaller than fileSize
    {
        if (source == 0)
        {
            startPoint = &(s->mem_buf);
        }
        else
        {
            startPoint = &source;
        }

        fseek(fileToWrite, 0, SEEK_SET);  // sets file position indicator to beginning of file
        fseek(fileToWrite, target, SEEK_SET);  // sets file position indicator to target
        fwrite(startPoint, s->unit_size, length, fileToWrite);  // writes to file, length elements each with the size of unit size
        // the data is taken from the memory location that startPointpoints to
    }
    fclose(fileToWrite);
}





void memory_Modify(state* s){
    int location = 0;
    int val = 0;
    char input[BUFSIZ];
    printf("Please enter <location> <val>\n");
    if (fgets(input, BUFSIZ, stdin) == NULL)
    {
        printf("Failed to read user input\n");
        free(s);
        exit(1);
    }

    if (sscanf(input, "%x %x", &location, &val) != 2)  // parse location and length
    {
        printf("Invalid input. Expected hexadecimal source, hexadecimal target and decimal length\n");
        free(s);
        exit(1);
    }

    if (s->debug_mode == '1')
    {
        printf("file name: %s\n", s->file_name);
        printf("loaction: %x\n", location);
        printf("val: %x\n", val);
    }

    memcpy(&(s->mem_buf[location]), &val, s->unit_size);  // copying val into the memory buffer at location with unit size number of bytes

    // if ((s->mem_count - location) >= s->unit_size)
    // {
    //     memcpy(&(s->mem_buf[location]), &val, s->unit_size);  // copying val into the memory buffer at location with unit size number of bytes
    // }
    // else
    // {
    //     printf("Error: Can't modify a unit of size unit_size there, because not enough bytes exist from the location given\n");
    // }
    
}





void quit(state* s){
    if (s->debug_mode == '1')
    {
        printf("quitting\n");
    }
    free(s);
    exit(0);
}


/*
Choose action:
0-Toggle Debug Mode
1-Set File Name
2-Set Unit Size
3-Load Into Memory
4-Toggle Display Mode
5-File Display
6-Memory Display
7-Save Into File
8-Memory Modify
9-Quit
*/



int main(int argc, char **argv){
    state* currState = malloc(sizeof(state));
    currState->debug_mode = '0';
    currState->display_mode = '0';
    currState->unit_size = 1;  // default
    currState->mem_count = 0;
    
    struct fun_desc menu[] = {
        {"Toggle Debug Mode", toggle_Debug_Mode},
        {"Set File Name", set_File_Name},
        {"Set Unit Size", set_Unit_Size},
        {"Load Into Memory", load_Into_Memory},
        {"Toggle Display Mode", toggle_Display_Mode},
        {"File Display", file_Display},
        {"Memory Display", memory_Display},
        {"Save Into File", save_Into_File},
        {"Memory Modify", memory_Modify},
        {"Quit", quit},
        {NULL, NULL}  };

    while (1)
    {
        int menuSize = sizeof(menu) / sizeof(struct fun_desc) - 1;
        int i = 0;

        if (currState->debug_mode == '1')
        {
            printf("unit size = %d\n", currState->unit_size);
            printf("file name: %s\n", currState->file_name);
            printf("mem count = %d\n\n", currState->mem_count);
        }

        fprintf(stdout, "Choose action:\n");
        while (menu[i].name != NULL)
        {
            printf("%d) %s\n", i, menu[i].name);
            i++;
        }

        int action = -1;  // in order to save the option the user chose
        printf("action:\n");
        scanf("%d", &action);
        fgetc(stdin);

        if(feof(stdin)){
            break;
        }

        if (action >= 0 && action < menuSize)
        {
            printf("Within bounds\n");
            printf("\n");
        }
        else
        {
            printf("Not within bounds\n");
            free(currState);
            exit(0);
        }

        menu[action].fun(currState);
        printf("\n");

    }
    
}




