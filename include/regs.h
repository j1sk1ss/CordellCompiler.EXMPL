#ifndef REGS_H_
#define REGS_H_

#include "str.h"
#include "logg.h"
#include "token.h"
#include "syntax.h"

static const char* regs[4][8] = {
    { "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp" },
    { "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp" },
    { "ax",  "bx",  "cx",  "dx",  "si",  "di",   "",    ""    },
    { "al",  "bl",  "cl",  "dl",  "sil", "dil",  "",    ""    },
};

#define BASE_BITNESS    64
enum {
    RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP, R8, R9, R10
};

/*
Simple register getter. For more complex API, use get_reg.
Mapping:
0 - RAX EAX AX AL
1 - RBX EBX BX BL
2 - RCX ECX CX CL
4 - RDX EDX DX DL
5 - RBP EBP .. ..
6 - RSP ESP .. ..
*/
#define GET_REG(source, register) __get_register__(get_variable_size((node)->token), register)
#define GET_RAW_REG(size, register) __get_register__(size, register)
static inline const char* __get_register__(int size, int pos) {
    switch (size) {
        case 8:  return regs[3][pos];
        case 16: return regs[2][pos];
        case 32: return regs[1][pos];
        case 64:
        default: return regs[0][pos];
    }
}

#define GET_ASMVAR(node) \
    (!((node)->token->ro || (node)->token->glob) ? \
        format_from_stack((node)->variable_offset) : \
        format_from_data((node)->token->value, (node)->token->t_type))      

static inline char* format_from_stack(int offset) {
    static char stack_buff[64] = { 0 };
    snprintf(stack_buff, sizeof(stack_buff), "[%s - %d]", GET_RAW_REG(BASE_BITNESS, RBP), ALIGN_TO(offset, 8));
    return stack_buff;
}

static inline char* format_from_data(unsigned char* name, token_type_t type) {
    if (type != UNKNOWN_NUMERIC_TOKEN) {
        static char data_buff[64] = { 0 };
        if (type == ARR_VARIABLE_TOKEN || type == STR_VARIABLE_TOKEN) snprintf(data_buff, sizeof(data_buff), "__%s__", name);
        else snprintf(data_buff, sizeof(data_buff), "[__%s__]", name);
        return data_buff;
    }
    else {
        return (char*)name;
    }
}

typedef struct {
    const char* name;
    const char* operation;
    const char* move;
} regs_t;

/*
Fills regs struct by valid data about register.
Params:
- regs - Pointer to struct for storing data.
- size - Variable size for register.
- base64 - Base register (RAX, RBX, ... etc).
- ptr - Is pointer or array?

Return 1 in any case.
*/
int get_reg(regs_t* regs, int size, int reg, int ptr);

#endif