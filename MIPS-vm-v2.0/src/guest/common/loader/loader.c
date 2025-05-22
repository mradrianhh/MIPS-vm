#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "loader.h"
#include "convert.h"
#include "guest/devices/vmemory/vmemory.h"
#include "guest/devices/vmemory/mapping.h"
#include "guest/devices/vcpu/vcpu.h"

void loader_load_elf32(const char *filename)
{
    FILE *fp;
    vMemory_t *memory_ref = vmemory_get_memory_ref();
    uint32_t *pc_ref = vcpu_get_pc_ref();
    Elf32_Ehdr eh;
    Elf32_Phdr ph[16];

    if ((fp = fopen(filename, "rb")) == NULL)
    {
        printf("ERROR: Can't open ELF-file %s.\n", filename);
        exit(EXIT_FAILURE);
    }

    fread(&eh, sizeof(eh), 1, fp);

    if (eh.e_ident[EI_DATA] == ELFDATA2MSB)
    {
        convert_elf32_ehdr_be_le(&eh);
    }

    // Check header validity.
    if (eh.e_ident[EI_MAG0] != ELFMAG0 ||
        eh.e_ident[EI_MAG1] != ELFMAG1 ||
        eh.e_ident[EI_MAG2] != ELFMAG2 ||
        eh.e_ident[EI_MAG3] != ELFMAG3 ||
        eh.e_ident[EI_CLASS] != ELFCLASS32 ||
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
        if (eh.e_ident[EI_DATA] == ELFDATA2MSB)
        {
            convert_elf32_phdr_be_le(&ph[i]);
        }
        if (ph[i].p_type == PT_LOAD)
        {
            uint8_t cacheable;
            uint32_t phys_addr = address_translation(ph[i].p_vaddr, ACCESS_MODE_STORE, &cacheable);
            if (ph[i].p_filesz)
            {
                fseek(fp, ph[i].p_offset, SEEK_SET);
                fread(&memory_ref[phys_addr], ph[i].p_filesz, 1, fp);
            }
            if (ph[i].p_filesz < ph[i].p_memsz)
            {
                memset(&memory_ref[phys_addr + ph[i].p_filesz], 0, ph[i].p_memsz - ph[i].p_filesz);
            }
        }
    }
}

void loader_flash_rom(const char *filename)
{
    FILE *fp;
    vMemory_t *memory_ref = vmemory_get_memory_ref();

    if ((fp = fopen(filename, "rb")) == NULL)
    {
        printf("ERROR: Can't open binary-file %s.\n", filename);
        exit(EXIT_FAILURE);
    }

    // Find size.
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if((RESET_VECTOR_PADDR + filesize) > VMEMORY_SIZE)
    {
        printf("ERROR: Not enough memory to load the ROM.\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // Read into memory.
    fread(&memory_ref->start[RESET_VECTOR_PADDR], filesize, 1, fp);
    fclose(fp);
    printf("Successfully flashed ROM from %s, filesize: %zu bytes.\n", filename, filesize);
}
