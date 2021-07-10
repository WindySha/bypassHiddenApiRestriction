//  MIT License
//  Copyright (c) 2021 Windysha
//  https://github.com/WindySha/bypass_dlfunctions
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//      of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//      to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//      copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//      AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <stdbool.h>
#include "efl_parser.h"

static void *getBaseAddressFromMaps(const char *file_name) {
    FILE *maps;
    char buff[256];
    off_t load_addr;
    maps = fopen("/proc/self/maps", "r");
    int result = 0;
    while (fgets(buff, sizeof(buff), maps)) {
        if ((strstr(buff, "r-xp") || strstr(buff, "r--p")) && strstr(buff, file_name)) {
            result = sscanf(buff, "%lx", &load_addr);
            break;
        }
    }
    fclose(maps);
    if (result != 1) {
        LOGE("failed to read loaded eld address for %s", file_name);
        return NULL;
    }

    LOGD("get elf base %s: %p", file_name, load_addr);
    return (void *) load_addr;
}

// parse the elf file's .text section, find the address which instruction equals to instruction_addr
void *getInstructionAddressFromElf(const char *elf_file, const ASM_Instruction* instruction_addr, size_t instruction_legnth) {

    //load the elf file
    int fd = open(elf_file, O_RDONLY);
    if (fd < 0) {
        LOGE("failed to open file -> %s", elf_file);
        return NULL;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size <= 0) {
        LOGE("lseek() failed for file -> %s", elf_file);
    }

    Elf_Ehdr *elf_header = (Elf_Ehdr *) mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (elf_header == MAP_FAILED) {
        LOGE("mmap() failed for file %s", elf_file);
        return NULL;
    }

    off_t bias = -4096;
    Elf_Shdr *progbits_section = NULL;

    // find the section header, according to the section header offset in the head
    Elf_Shdr *section_header = (Elf_Shdr *) (((size_t) elf_header) + elf_header->e_shoff);
    size_t section_header_ptr_size =
        elf_header->e_shentsize;  //this number equals to: sizeof(Elf_Shdr)

    // find the string table offset by the section string offset in the header
    Elf_Off sec_str_off = section_header[elf_header->e_shstrndx].sh_offset;
    char *section_str = (char *) (((size_t) elf_header) + sec_str_off);

    size_t iterate_sec_h = (size_t) section_header;
    for (int i = 0; i < elf_header->e_shnum;
         i++, iterate_sec_h += section_header_ptr_size) {
        Elf_Shdr *sec_h = (Elf_Shdr *) iterate_sec_h;
        if (sec_h->sh_type == SHT_PROGBITS) {

            if (bias == -4096) {
                bias = (off_t) sec_h->sh_addr - (off_t) sec_h->sh_offset;
            }

            //sh_name is the name offset in the section string table
            char *section_header_name = sec_h->sh_name + section_str;

            if (strcmp(section_header_name, ".text") == 0) {
                progbits_section = sec_h;
                break;
            }
        }
    }

    if (progbits_section == NULL) {
        LOGE("cannot find the .text section part in file %s", elf_file);
        goto error_exit;
    }

    void *base_address = getBaseAddressFromMaps(elf_file);

    ASM_Instruction* instr_start = (ASM_Instruction *) (((size_t) elf_header) + progbits_section->sh_offset);
    size_t instr_code_size = sizeof(ASM_Instruction);
    size_t instruc_offset = 0;
    for (int i = 0; i < progbits_section->sh_size; i+=instr_code_size, instr_start++) {
        bool instruction_matched = true;
        for (int j = 0; j < instruction_legnth; j++) {
            if (*(instr_start + j) != *(instruction_addr + j)) {
                instruction_matched = false;
                break;
            }
        }
        if (instruction_matched) {
            instruc_offset = i;
            break;
        }
    }
    off_t off = progbits_section->sh_addr + instruc_offset;

    if (elf_header != MAP_FAILED)
        munmap(elf_header, size);

    return (void *) ((size_t) base_address + off - bias);

    error_exit:
    if (elf_header != MAP_FAILED)
        munmap(elf_header, size);
    return NULL;
}