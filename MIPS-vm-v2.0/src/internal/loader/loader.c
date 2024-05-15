#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "loader.h"
#include "elf/elf_types.h"
#include "convert.h"
#include "devices/vmemory/vmemory.h"
#include "devices/vcpu/vcpu.h"

void loader_load_elf32(const char *filename)
{
    FILE *fp;
    uint8_t *memory_ref = vmemory_get_memory_ref();
    uint32_t *pc_ref = vcpu_get_pc_ref();
    Elf32_Ehdr eh;
    Elf32_Phdr ph[16];

    if ((fp = fopen(filename, "rb")) == NULL)
    {
        printf("ERROR: Can't open ELF-file %s.\n", filename);
        exit(EXIT_FAILURE);
    }

    fread(&eh, sizeof(eh), 1, fp);
    convert_elf32_ehdr_be_le(&eh);

    // Check header validity.
    if (eh.e_ident[EI_MAG0] != ELFMAG0 ||
        eh.e_ident[EI_MAG1] != ELFMAG1 ||
        eh.e_ident[EI_MAG2] != ELFMAG2 ||
        eh.e_ident[EI_MAG3] != ELFMAG3 ||
        eh.e_ident[EI_CLASS] != ELFCLASS32 ||
        eh.e_ident[EI_DATA] != ELFDATA2MSB ||
        eh.e_ident[EI_VERSION] != EV_CURRENT ||
        eh.e_machine != EM_MIPS)
    {
        printf("Invalid ELF header.");
        exit(EXIT_FAILURE);
    }

    // Check program headers size
    if (eh.e_phoff == 0 || eh.e_phnum == 0 || eh.e_phnum > 16 || eh.e_phentsize != sizeof(Elf32_Phdr))
    {
        printf("Invalid program headers.");
        exit(EXIT_FAILURE);
    }

    // Read program header
    fseek(fp, eh.e_phoff, SEEK_SET);
    fread(ph, eh.e_phentsize, eh.e_phnum, fp);

    // Load each segment
    for (int i = 0; i < eh.e_phnum; i++)
    {
        convert_elf32_phdr_be_le(&ph[i]);
        if (ph[i].p_type == PT_LOAD)
        {
            if (ph[i].p_filesz)
            {
                fseek(fp, ph[i].p_offset, SEEK_SET);
                fread(&memory_ref[ph[i].p_vaddr], ph[i].p_filesz, 1, fp);
            }
            if (ph[i].p_filesz < ph[i].p_memsz)
            {
                memset(&memory_ref[ph[i].p_vaddr + ph[i].p_filesz], 0, ph[i].p_memsz - ph[i].p_filesz);
            }
        }
    }

    // Point PC to entry point.
    *pc_ref = eh.e_entry;
}