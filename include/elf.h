#ifndef ELF64_
#define ELF64_

typedef struct {
    unsigned int  magic;
    unsigned char bitness;
    unsigned char endian;
    unsigned char version;
    unsigned char abi;
    unsigned char abi_version;
    unsigned char padd[7];
} __attribute__((packed)) elf_ident_t;

/* https://wiki.osdev.org/ELF */
typedef struct {
    elf_ident_t indent;
    unsigned short type;
    unsigned short machine;
    unsigned int   version;
    unsigned long  entry;
    unsigned long  phoff;
    unsigned long  shoff;
    unsigned int   flags;
    unsigned short ehsize;
    unsigned short phentsize;
    unsigned short phnum;
    unsigned short shentsize;
    unsigned short shnum;
    unsigned short shstrndx;
} __attribute__((packed)) elf64_header_t;

#endif