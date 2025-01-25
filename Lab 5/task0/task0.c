#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>

void print_phdr_info(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, (void*)phdr);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr*, int), int arg){
    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)map_start;     //  pointing to the Elf32_Ehdr structure, to the header of the ELF
    Elf32_Phdr *phdr = (Elf32_Phdr*)(map_start + ehdr->e_phoff);   //  pointing to the program header table
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        func(&phdr[i], i);
    }
    
    return 0;
}

void load_elf(const char* filename){
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Get file size
    off_t filesize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Map the file into memory
    void *map_start = malloc(filesize);
    if (!map_start) {
        perror("Memory allocation error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (read(fd, map_start, filesize) != filesize) {    //  putting the file in the memory location we allocated
        perror("Error reading file");
        free(map_start);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Iterate over program headers
    foreach_phdr(map_start, print_phdr_info, 0);        //  our program header iteration function

    free(map_start);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <32bit ELF executable>\n", argv[0]);
        return EXIT_FAILURE;
    }

    load_elf(argv[1]);

    return EXIT_SUCCESS;
}
