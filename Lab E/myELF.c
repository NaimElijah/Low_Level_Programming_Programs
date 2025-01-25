#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    void *map1;  // memory location of the mapped file
    void *map2;
    int fd1; // for mapping
    int fd2;
    char *fileName1;
    char *fileName2;
    struct stat file_info1; // gives information about the file
    struct stat file_info2;
} handle_file;

struct fun_desc {
    char *name;
    void (*fun)();
};

handle_file *currFd = NULL;    // GLOBAL file handling

int numOpenFiles = 0; // 0, 1 or 2
int debug_mode = 0;  // the default for debug mode is off
Elf32_Ehdr* header;
//Elf32_Shdr* sh_header;




char *sh_type(int sh_type)
{
    if (sh_type == SHT_NULL)
        return "NULL";
    if (sh_type == SHT_PROGBITS)
        return "PROGBITS";
    if (sh_type == SHT_SYMTAB)
        return "SYMTAB";
    if (sh_type == SHT_STRTAB)
        return "STRTAB";
    if (sh_type == SHT_RELA)
        return "RELA";
    if (sh_type == SHT_HASH)
        return "HASH";
    if (sh_type == SHT_DYNAMIC)
        return "DYNAMIC";
    if (sh_type == SHT_NOTE)
        return "NOTE";
    if (sh_type == SHT_NOBITS)
        return "NOBITS";
    if(sh_type == SHT_REL)
        return "REL";
    if(sh_type == SHT_SHLIB)
        return "SHLIB";
    if(sh_type == SHT_DYNSYM)
        return "DYNSYM";
    else
        return "UNKNOWN";
}














char *sh_name(int s_index)
{
    if (s_index == SHN_UNDEF)
        return "UNDEF";
    if (s_index == SHN_LORESERVE)
        return "LORESERVE";
    if (s_index == SHN_LOPROC)
        return "LOPROC";
    if (s_index == SHN_BEFORE)
        return "BEFORE";
    if (s_index == SHN_AFTER)
        return "AFTER";
    if (s_index == SHN_HIPROC)
        return "HIPROC";
    if (s_index == SHN_LOOS)
        return "LOOS";
     if (s_index == SHN_HIOS)
        return "HIOS";
    if (s_index == SHN_ABS)
        return "ABS";
    if (s_index == SHN_COMMON)
        return "COMMON";
    if (s_index == SHN_XINDEX)
        return "XINDEX";
    if (s_index == SHN_HIRESERVE)
        return "HIRESERVE";
    else
        return "UNKNOWN";
}












void toggle_Debug_Mode(){
    if (debug_mode == 0) {
        printf("debug flag is now on\n");
        debug_mode = 1;
    }
    else {
        printf("debug flag is now off\n");
        debug_mode = 0;
    }
}













void examine_ELF_File(){
    char buffer[BUFSIZ];
    int protectionFlag = PROT_READ;
    int mappingFlag = MAP_SHARED;
    int fd = -1;
    char fileName[256];

    printf("Please enter file name:\n");
    fscanf(stdin, "%s", buffer);
    strcpy(fileName, buffer);

    if (numOpenFiles == 2){
        printf("There are already 2 opened ELF files\n");
    }
    else {
        int file = open(buffer, O_RDONLY);
        struct stat file_info;
        void *map;

        if (file < 0) {
            perror("failed opening the file\n");
            freee();
            exit(1);
        }

        if (fstat(file, &file_info) < 0) { // fstat retrieves information about the file and puts it in file_info
            perror("fstat failed\n");
            close(file);
            freee();
            exit(1);
        }

        map = mmap(0, file_info.st_size, protectionFlag, mappingFlag, file, 0);
        if (map == MAP_FAILED) {
            printf("Mapping failed\n");
            close(file);
            fd = -1;
            munmap(map, file_info.st_size);
            freee();
            exit(1);
        }

        header = (Elf32_Ehdr *) map;

        if (strncmp((char *) header->e_ident, (char *) ELFMAG, 4) != 0) {
            printf("The magic number isn't consistent with an ELF file\n");
            close(file);
            fd = -1;
            munmap(map, file_info.st_size);
            freee();
            exit(1);
        } else {
            fd = file;
        }

        if (numOpenFiles == 1) {
            currFd->fd2 = fd;
            currFd->fileName2 = malloc(strlen(fileName) + 1);  //  allocate memory for the file
            strcpy(currFd->fileName2, fileName);               //  copy the file name
            currFd->file_info2 = file_info;
            currFd->map2 = map;
        } else { // if numOpenFiles == 0
            currFd->fd1 = fd;
            currFd->fileName1 = malloc(strlen(fileName) + 1);  // allocate memory for the file
            strcpy(currFd->fileName1, fileName);    // copy the file name
            currFd->file_info1 = file_info;
            currFd->map1 = map;
        }

        numOpenFiles++;

        // now for all the prints:

        printf("Bytes 1,2,3 of the magic number: %x %x %x\n", header->e_ident[EI_MAG1], header->e_ident[EI_MAG2], header->e_ident[EI_MAG3]);
        if (header->e_ident[EI_DATA] == ELFDATA2LSB)
            printf("The data encoding scheme of the object file is: 2's complement, little endian\n");
        else
            printf("The data encoding scheme of the object file is: 2's complement, big endian\n");
        printf("Entry point is: %x \n", header->e_entry);
        printf("The file offset in which the section header table resides: %d \n", header->e_shoff);
        printf("The number of section headers entries: %d \n", header->e_shnum);
        printf("The size of each section header entry: %d \n", header->e_shentsize);
        printf("The file offset in which the program header table resides: %d \n", header->e_phoff);
        printf("The number of program header table entries: %d \n", header->e_phnum);
        printf("The size of each program header entry: %d \n", header->e_phentsize);
    }
}












void print_Section_For_File(void *map, char *fileName)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize); // find str_table location in memory
    printf("File %s \n", fileName);
    if(debug_mode == 0){
        printf("[%5s] %-16s %-16s %-16s %-16s %-16s\n", "index", "section_name", "section_address", "section_offset",
            "section_size", "section_type");
        for (int i = 0; i < elf_header->e_shnum; i++) {
            Elf32_Shdr *curr_sectionHeader_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
            char *curr_sectionHeader_name = map + str_table->sh_offset + curr_sectionHeader_entry->sh_name;
            printf("[%2d] %2s %-16s %08x %7s %06x %9s %06x %10s %-20s\n", i, "", curr_sectionHeader_name,
                   curr_sectionHeader_entry->sh_addr,
                   "", curr_sectionHeader_entry->sh_offset, "", curr_sectionHeader_entry->sh_size, "",
                   sh_type(curr_sectionHeader_entry->sh_type));
        }
    }
    else{
        printf("[%5s] %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n", "index", "section_name", "section_address", "section_offset",
               "section_size", "section_type", "shstrndx", "section name offset");
        for(int i = 0; i < elf_header->e_shnum; i++){
            Elf32_Shdr *curr_sectionHeader_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
            char *curr_sectionHeader_name = map + str_table->sh_offset + curr_sectionHeader_entry->sh_name;
            printf("[%2d] %2s %-16s %08x %7s %06x %9s %06x %10s %-15s %d %13s %d\n", i, "", curr_sectionHeader_name,
                   curr_sectionHeader_entry->sh_addr, "", curr_sectionHeader_entry->sh_offset, "", curr_sectionHeader_entry->sh_size,
                   "", sh_type(curr_sectionHeader_entry-> sh_type), elf_header->e_shstrndx, "", (elf_header->e_shoff + (elf_header->e_shentsize * i)));
        }
    }
}













void print_Section_Names()
{
    if (numOpenFiles == 0)
    {
        printf("No mapped file\n");
        return;
    }
    else if(numOpenFiles == 1){
        print_Section_For_File(currFd->map1, currFd->fileName1);
    }
    else{
        print_Section_For_File(currFd->map1, currFd->fileName1);
        print_Section_For_File(currFd->map2, currFd->fileName2);
    }
}














void print_Symbols_For_File(void *map, char *fileName){
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *dynsym = NULL;
    Elf32_Shdr *str_table_temp = NULL;
    Elf32_Shdr *str_table;
    Elf32_Shdr *section_entry;
    Elf32_Sym *curr_entry;
    int num_of_entries = 0; //TODO check if need this
  
    str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize);

    for (int i = 0; i < elf_header->e_shnum && (symtab == NULL || dynsym == NULL || str_table_temp == NULL); i++){
        curr_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
        char *curr_name = map + str_table->sh_offset + curr_entry->st_name;
        if (strcmp(curr_name, ".symtab") == 0){
            symtab = (Elf32_Shdr *)curr_entry;
        }
        else if(strcmp(curr_name, ".dynsym") == 0){
            dynsym = (Elf32_Shdr *)curr_entry;
        }
        else if (strcmp(curr_name, ".strtab") == 0){
           str_table_temp = (Elf32_Shdr *)curr_entry;
        }
    }

    if (dynsym != NULL){
        num_of_entries = dynsym->sh_size / sizeof(Elf32_Sym);
        // str_table = (Elf32_Shdr *)str_table;
        str_table = str_table_temp;
        printf("File %s \n", fileName);
        printf("Symbol table '.dynsym' \n");
        printf("[%5s] %-16s %-16s %-16s %-16s\n", "index", "value", "section index", "section name", "symbol name");
        for(int i = 0; i < num_of_entries; i++) {
            curr_entry = map + dynsym->sh_offset + (sizeof(Elf32_Sym) * i);
            char *section_name = sh_name(curr_entry->st_shndx);    //  returns the entry name
            if (strcmp(section_name, "UNKNOWN") == 0) {
                section_entry = map + elf_header->e_shoff + (curr_entry->st_shndx * elf_header->e_shentsize);
                section_name = map + str_table->sh_offset + section_entry->sh_name;
            }
            char *symbol_name = map + str_table->sh_offset + curr_entry->st_name;

            printf("[%d] %3s %-16x %-16d %-16s %-16s\n", i, "", curr_entry->st_value, curr_entry->st_shndx,
                   section_name, symbol_name);
        }
        if (debug_mode == 1){ 
            printf("\nsize of symbol table: 0000%0x \n", dynsym->sh_size);
            printf("num of symbols: %d \n\n", num_of_entries);
        }
    }

    if (symtab != NULL) {
        num_of_entries = symtab->sh_size / sizeof(Elf32_Sym);
        str_table = str_table_temp;
        if (dynsym == NULL)
            printf("\nFile %s \n", fileName);
        printf("Symbol table '.symtab' \n");
        printf("[%5s] %-16s %-16s %-24s %-16s\n", "index", "value", "section index", "section name", "symbol name");
        for (int i = 0; i < num_of_entries; i++) {
            curr_entry = map + symtab->sh_offset + (sizeof(Elf32_Sym) * i);
            char *section_name = sh_name(curr_entry->st_shndx);
            if (strcmp(section_name, "UNKNOWN") == 0) {
                section_entry = map + elf_header->e_shoff + (curr_entry->st_shndx * elf_header->e_shentsize);
                section_name = map + str_table->sh_offset + section_entry->sh_name;
            }
            char *symbol_name = map + str_table->sh_offset + curr_entry->st_name;
            printf("[%2d] %3s %-16x %-16d %-24s %-16s\n", i, "", curr_entry->st_value, curr_entry->st_shndx,
                   section_name, symbol_name);
        }
        if (debug_mode == 1) {
            printf("\nsize of symbol table: 0000%0x \n", symtab->sh_size);
            printf("num of symbols: %d \n", num_of_entries);
        }
    }
}













void print_Symbols(){
    if (numOpenFiles == 0)
    {
        printf("No mapped file\n");
        return;
    }
    else if(numOpenFiles == 1)
    {
        print_Symbols_For_File(currFd->map1, currFd->fileName1);
    }
    else if (numOpenFiles == 2)
    { // num of open files is 2
        print_Symbols_For_File(currFd->map1, currFd->fileName1);
        print_Symbols_For_File(currFd->map2, currFd->fileName2);
    }
}













Elf32_Shdr *find_symbol_table(void *map)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *str_table;
    Elf32_Sym *curr_entry;

    str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize);
   
    for (int i = 0; i < elf_header->e_shnum && (symtab == NULL); i++) {
        curr_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);   //  getting the entry
        char *curr_name = map + str_table->sh_offset + curr_entry->st_name;   //  getting the entry's name
        if (strcmp(curr_name, ".symtab") == 0)
        {
            return (Elf32_Shdr *) curr_entry;
        }
    }
    return NULL;
}













int check_num_symbol_tables(void *map)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *dynsym = NULL;
    Elf32_Shdr *str_table;
    Elf32_Sym *curr_entry;
   
    str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize);
    
    for (int i = 0; i < elf_header->e_shnum && (symtab == NULL || dynsym == NULL); i++)
    {
        curr_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i); //  getting the current entry in the Section header table
        char *curr_name = map + str_table->sh_offset + curr_entry->st_name; //  getting the current entry name in the String table
        if (strcmp(curr_name, ".symtab") == 0)
        {
            symtab = (Elf32_Shdr *)curr_entry;
        }
        else if (strcmp(curr_name, ".dynsym") == 0)
        {
            dynsym = (Elf32_Shdr*)curr_entry;
        }
    }

    if (symtab != NULL && dynsym != NULL)
    {
        return 0;
    }
    return 1;
}












void check_Files_For_Merge()
{
    if(numOpenFiles != 2)
    {
        printf("2 ELF files required for merging\n");
        return;
    }
  
    void *map1 = currFd->map1;
    void *map2 = currFd->map2;
    
    int check_file1 = check_num_symbol_tables(map1);
    int check_file2 = check_num_symbol_tables(map2);

    if(check_file1 == 0 || check_file2 == 0)   //  means there is more than one sym table
    {
        printf("feature not supported\n");
        return;
    }

    Elf32_Shdr *symtab_file1 = find_symbol_table(map1);   //  getting the symbol table
    Elf32_Shdr *symtab_file2 = find_symbol_table(map2);   //  getting the symbol table

    Elf32_Ehdr *elf_header_file1 = (Elf32_Ehdr*)(map1);
    Elf32_Ehdr *elf_header_file2 = (Elf32_Ehdr*)(map2);

    int num_of_entries_file1 = symtab_file1->sh_size / sizeof(Elf32_Sym);  // num of entries in the symbol table
    int num_of_entries_file2 = symtab_file2->sh_size / sizeof(Elf32_Sym);  // num of entries in the symbol table

    Elf32_Shdr *str_table1 = map1 + elf_header_file1->e_shoff + (symtab_file1->sh_link * elf_header_file1->e_shentsize); // symbols string table
    Elf32_Shdr *str_table2 = map2 + elf_header_file2->e_shoff + (symtab_file2->sh_link * elf_header_file2->e_shentsize); // symbols string table

    for(int i = 0; i < num_of_entries_file1; i++){
        Elf32_Sym *curr_entry1 = map1 + symtab_file1->sh_offset + (sizeof(Elf32_Sym) * i); // entry in symbol table
        char *section_name1 = sh_name(curr_entry1->st_shndx);
        char *symbol_name1 = map1 + str_table1->sh_offset + curr_entry1->st_name;  // getting the symbol name
        if(curr_entry1->st_shndx == SHN_UNDEF){
            int found = 0;
            for(int j = 1; j < num_of_entries_file2 && (found == 0); j++){   //  same iteration but for the 2nd ELF file, from after "dummy"
                Elf32_Sym *curr_entry2 = map2 + symtab_file2->sh_offset + (sizeof(Elf32_Sym) * j);
                char *section_name2 = sh_name(curr_entry2->st_shndx);
                char *symbol_name2 = map2 + str_table2->sh_offset + curr_entry2->st_name;
                if(strcmp(symbol_name1, symbol_name2) == 0){
                    found = 1;
                    if(curr_entry2->st_shndx == SHN_UNDEF)
                        printf("symbol sym undefined\n");
                }
            }
            if(found == 0)
                printf("symbol sym undefined\n");
        }
        else{     // if the symbol is defined
            int found = 0;
            for(int j = 1; j < num_of_entries_file2 && (found == 0); j++){   //  same iteration but for the 2nd ELF file, from after "dummy"
                Elf32_Sym *curr_entry2 = map2 + symtab_file2->sh_offset + (sizeof(Elf32_Sym) * j);
                char *section_name2 = sh_name(curr_entry2->st_shndx);
                char *symbol_name2 = map2 + str_table2->sh_offset + curr_entry2->st_name;

                if((strcmp(symbol_name1, "") == 0) && (strcmp(symbol_name2, "") == 0))
                    continue;
                
                if(strcmp(symbol_name1, symbol_name2) == 0){
                    found = 1;
                    if(curr_entry2->st_shndx != SHN_UNDEF)
                        printf("symbol sym multiply defined\n");
                }
            }
        }
    }
}
















void merge_ELF_Files()
{
    void *map1 = currFd->map1;
    void *map2 = currFd->map2;
    
    Elf32_Ehdr *elf_header1 = (Elf32_Ehdr *)map1;
    Elf32_Ehdr *elf_header2 = (Elf32_Ehdr *)map2;
    
    Elf32_Shdr *section_header1 = map1 + elf_header1->e_shoff;
    Elf32_Shdr *section_header2 = map2 + elf_header2->e_shoff;

    Elf32_Shdr *symtab_file1 = find_symbol_table(map1);
    Elf32_Shdr *symtab_file2 = find_symbol_table(map2);

    Elf32_Shdr *str_table1 = map1 + elf_header1->e_shoff + (symtab_file1->sh_link * elf_header1->e_shentsize);  // symbols string table
    Elf32_Shdr *str_table2 = map2 + elf_header2->e_shoff + (symtab_file2->sh_link * elf_header2->e_shentsize);  // symbols string table

    int num_of_entries_file1 = symtab_file1->sh_size / sizeof(Elf32_Sym);  // num of entries in the symbol table
    int num_of_entries_file2 = symtab_file2->sh_size / sizeof(Elf32_Sym);  // num of entries in the symbol table
    
    int out_file = open("out.ro", O_WRONLY | O_CREAT, 0666); // create a new ELF file
    if (out_file < 0) {
        perror("failed to create out file\n");
        return;
    }
    
    write(out_file, (char*)elf_header1, elf_header1->e_ehsize); // copy initial version of elf's header as the new elf's header

    //declares an array of Elf32_shdr. The size of the array is determined by header1->e_shnum, which represents the number of entries in the
    // section header table of the first ELF file.
    Elf32_Shdr section_header_arr[elf_header1->e_shnum];

    //copy the contents of the section_header1 to the section_header_arr
    memcpy((char *)section_header_arr, (char *)section_header1, elf_header1->e_shnum * sizeof(Elf32_Shdr));

    for (int i = 0; i < elf_header1->e_shnum; i++) {  // loop over the entries of the NEW section header table
        section_header_arr[i].sh_offset = lseek(out_file, 0, SEEK_CUR);
        Elf32_Shdr *curr_section_header = map1 + elf_header1->e_shoff + ((elf_header1->e_shentsize) * i);  //  current entry
        char *curr_section_name = map1 + str_table1->sh_offset + curr_section_header->sh_name;
        if(strcmp(curr_section_name, ".text") == 0 || strcmp(curr_section_name, ".data") == 0 || strcmp(curr_section_name, ".rodata") == 0){
            // Copy the section contents from the first ELF file
            write(out_file, (char *)elf_header1 + curr_section_header->sh_offset, curr_section_header->sh_size);

            // Get the corresponding section header from the second file
            for(int j = 0; j < elf_header2->e_shnum; j++){
                Elf32_Shdr *curr_section_header2 = map2 + elf_header2->e_shoff + ((elf_header2->e_shentsize) * j);
                char *curr_section_name2 = map2 + str_table2->sh_offset + curr_section_header2->sh_name;
                if(strcmp(curr_section_name, curr_section_name2) == 0){
                    // Append the section header from the second file
                    write(out_file, (char *)elf_header2 + section_header2->sh_offset, section_header2->sh_size);
                    section_header_arr[i].sh_size = section_header_arr[i].sh_size + curr_section_header2->sh_size;
                }
            }
            // Set sh_offset after writing section contents
            curr_section_header->sh_offset = lseek(out_file, 0, SEEK_CUR);  // set sh_offset after writing section contents
        }
        else if(strcmp(curr_section_name, ".symtab") == 0){
            // Declares an array of Elf32_Sym. The size of the array is num_of_entries_file1
            Elf32_Sym symtab[num_of_entries_file1];
            Elf32_Sym *curr_entry1 = map1 + symtab_file1->sh_offset + (sizeof(Elf32_Sym) * i);
            memcpy((char *)symtab, (char *)curr_entry1, section_header_arr[i].sh_size);

            for(int j = 0; j < num_of_entries_file1; j++){              // B  <<-------
                Elf32_Sym *curr_entry_j = map1 + symtab_file1->sh_offset + (sizeof(Elf32_Sym) * j);  //  current symbol
                char *section_name1 = sh_name(curr_entry_j->st_shndx);
                char *symbol_name1 = map1 + str_table1->sh_offset + curr_entry_j->st_name;  //  symbol name
                if(curr_entry_j->st_shndx == SHN_UNDEF){
                // copy over symbol values and definition of symbols from the second ELF file,               // B  <<-------
                // for every symbol that is UNDEFINED in the first ELF file
                    for(int k = 0; k < num_of_entries_file2; k++){
                        Elf32_Sym *curr_entry2 = map2 + symtab_file2->sh_offset + (sizeof(Elf32_Sym) * k);
                        char *section_name2 = sh_name(curr_entry2->st_shndx);
                        char *symbol_name2 = map2 + str_table2->sh_offset + curr_entry2->st_name;
                        if(strcmp(symbol_name1, symbol_name2) == 0){
                            symtab[k].st_value = curr_entry2[k].st_value;              // B  <<-------
                            for(int x = 0; x < elf_header1->e_shnum; x++){
                                Elf32_Shdr *curr_section_header_x = map1 + elf_header1->e_shoff + ((elf_header1->e_shentsize) * x);
                                char *curr_section_name_x = map1 + str_table1->sh_offset + curr_section_header_x->sh_name;
                                if(strcmp(curr_section_name_x, section_name2) == 0){
                                    symtab[j].st_shndx = x;
                                }
                            }
                        }
                    }
                }
            }

            write(out_file, (char *)symtab, section_header1[i].sh_size);
        }
        else{
            write(out_file, (char *)(map1 + section_header1[i].sh_offset), section_header1[i].sh_size);  // append section to out_file
        }
    }

    int sh_offset = lseek(out_file, 0, SEEK_CUR);
    write(out_file, (char *)section_header_arr, elf_header1->e_shnum * sizeof(Elf32_Shdr));
    lseek(out_file, 32, SEEK_SET);
    write(out_file, (char *)(&sh_offset), sizeof(int));
    close(out_file);  // close the file after writing
}












void freee(){
    if (debug_mode == 1)
    { // debug mode is on
        printf("quitting\n");
    }
    if (currFd->fd1 != -1)
    {
        munmap(currFd->map1, currFd->file_info1.st_size);
        close(currFd->fd1);
        numOpenFiles--;
    }
    if(currFd->fd2 != -1)
    {
        munmap(currFd->map2, currFd->file_info2.st_size);
        close(currFd->fd2);
        numOpenFiles--;
    }
}





void freeAndQuit0()
{
    freee();
    exit(0);
}










int main(int argc, char **argv)
{
    currFd = malloc(sizeof(handle_file));
    currFd->fd1 = -1;
    currFd->fd2 = -1;

    struct fun_desc menu[] = {
        {"Toggle Debug Mode", toggle_Debug_Mode},
        {"Examine ELF File", examine_ELF_File},
        {"Print Section Names", print_Section_Names},
        {"Print Symbols", print_Symbols},
        {"Check Files For Merge", check_Files_For_Merge},
        {"Merge ELF Files", merge_ELF_Files},
        {"QUIT", freeAndQuit0},
        {NULL, NULL}
    };

    while(1)
    {
        int menuSize = sizeof(menu) / sizeof(struct fun_desc) - 1;
        int i = 0;
        fprintf(stdout, "Choose action:\n");
        while(menu[i].name != NULL)
        {
            printf("%d) %s\n", i, menu[i].name);
            i++;
        }

        int action = -1; // In order to examine the option the user chose
        printf("action:\n");
        scanf("%d", &action);
        fgetc(stdin); //to clear buffer
        if(feof(stdin)) // when user presses CTRL^D- exit
            break;

        if (action >= 0 && action < menuSize)
        {
            printf("Within bounds\n");
            printf("\n");
        }
        else
        {
            printf("Not within bounds\n");
            exit(0);
        }
        menu[action].fun();
        printf("\n");
    }
}


