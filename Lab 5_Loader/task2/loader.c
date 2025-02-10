#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <sys/mman.h>

extern int startup(int argc, char **argv, void (*start)());

void print_phdr_info(Elf32_Phdr *phdr, int index) {
    const char *type_str;
    switch (phdr->p_type) {
        case PT_PHDR:   type_str = "PHDR"; break;
        case PT_INTERP: type_str = "INTERP"; break;
        case PT_LOAD:   type_str = "LOAD"; break;
        case PT_DYNAMIC: type_str = "DYNAMIC"; break;
        case PT_NOTE:   type_str = "NOTE"; break;
        default:        type_str = "UNKNOWN"; break;
    }

    // Determine mmap protection and mapping flags
    int prot = 0;
    int flags = 0;
    
    if (phdr->p_flags & PF_R) {
        prot |= PROT_READ;
    }
    if (phdr->p_flags & PF_W) {
        prot |= PROT_WRITE;
    }
    if (phdr->p_flags & PF_X) {
        prot |= PROT_EXEC;
    }

    // Always MAP_PRIVATE for loading
    flags |= MAP_PRIVATE;

    printf("%-10s 0x%08x 0x%08x 0x%08x 0x%06x 0x%06x %c%c 0x%08x  Prot: %s, Flags: %s\n",
           type_str,
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           (phdr->p_flags & PF_R) ? 'R' : '-',
           (phdr->p_flags & PF_W) ? 'W' : '-',
           phdr->p_align,
           prot == PROT_READ ? "PROT_READ" :
           prot == (PROT_READ | PROT_WRITE) ? "PROT_READ | PROT_WRITE" :
           prot == (PROT_READ | PROT_EXEC) ? "PROT_READ | PROT_EXEC" :
           prot == (PROT_READ | PROT_WRITE | PROT_EXEC) ? "PROT_READ | PROT_WRITE | PROT_EXEC" : "None",
           flags == MAP_PRIVATE ? "MAP_PRIVATE" : "Unknown");
}

//LOADs (Iterate over program headers)
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        func(&phdr[i], i);
    }
    return 0;
}

//load into memory on the correct place
//map each Phdr that has the PT_LOAD flag set, into memory
void load_phdr(Elf32_Phdr *phdr, int fd)
{
    if (phdr->p_type == PT_LOAD)
    {
        void *vaddr = (void*) (phdr->p_vaddr & 0xfffff000);
        int offset = phdr->p_offset & 0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        void *map_start = mmap(vaddr, phdr->p_memsz + padding, 
                            (phdr->p_flags & PF_R ? PROT_READ : 0) |
                            (phdr->p_flags & PF_W ? PROT_WRITE : 0) | (phdr->p_flags & PF_X ? PROT_EXEC : 0), 
                            MAP_FIXED | MAP_PRIVATE, fd, offset);
        if (map_start == MAP_FAILED)
        {
            printf("Failed to map the file.\n");
            exit(1);
        }
    }
    else
        return; // type is not load.
}

//load the elf file
void load_elf(const char *filename, int argc, char *argv[]) {

    //handle the exe we given
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Get file size
    off_t filesize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Map the file into memory (get data)
    void *map_start = malloc(filesize);
    if (!map_start) {
        perror("Memory allocation error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (read(fd, map_start, filesize) != filesize) {
        perror("Error reading file");
        free(map_start);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Print header information
    printf("Type          Offset     VirtAddr   PhysAddr   FileSiz MemSiz Flg Align       Prot Flags\n");
    
    // Iterate over program headers
    foreach_phdr(map_start, print_phdr_info, 0);

    // Load program headers into memory
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        load_phdr(&phdr[i], fd);
    }
    
    startup(argc-1, argv+1, (void *)(((Elf32_Ehdr *) map_start)->e_entry));
    free(map_start);
    close(fd);
}

int main(int argc, char *argv[]) {
    load_elf(argv[1], argc, argv);
    return EXIT_SUCCESS;
}
