#include "../include/regs.h"

static const char* _regs_table[][9] = {
    //      0     1      2     3      4    5    6   7    8
    /* rax */ { "", "al",   "ax",  "",  "eax", "", "", "", "rax" },
    /* rbx */ { "", "bl",   "bx",  "",  "ebx", "", "", "", "rbx" },
    /* rcx */ { "", "cl",   "cx",  "",  "ecx", "", "", "", "rcx" },
    /* rdx */ { "", "dl",   "dx",  "",  "edx", "", "", "", "rdx" },
    /* rsi */ { "", "sil",  "si",  "",  "esi", "", "", "", "rsi" },
    /* rdi */ { "", "dil",  "di",  "",  "edi", "", "", "", "rdi" },
    /* rbp */ { "", "bpl",  "bp",  "",  "ebp", "", "", "", "rbp" },
    /* rsp */ { "", "spl",  "sp",  "",  "esp", "", "", "", "rsp" },
    /* r8  */ { "", "r8b",  "r8w", "",  "r8d", "", "", "", "r8"  },
    /* r9  */ { "", "r9b",  "r9w", "",  "r9d", "", "", "", "r9"  },
    /* r10 */ { "", "r10b", "r10w","", "r10d", "", "", "", "r10" },
};

static const char* _types[] = { "", " byte ", " word ", "", " dword ", "", "", "", " " };

int get_reg(regs_t* regs, int size, int reg, int ptr) {
    regs->name      = _regs_table[reg][size];
    regs->operation = _types[size];
    regs->move      = (ptr && size == 8) ? "lea" : "mov";
    return 1;
}
