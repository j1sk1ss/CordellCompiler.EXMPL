#ifndef REGISTERS_H_
#define REGISTERS_H_

#include "token.h"
#include "syntax.h"
#include <stdio.h>

static const char* regs[4][4] = {
    { "rax", "rbx", "rcx", "rdx" },
    { "eax", "ebx", "ecx", "edx" },
    { "ax",  "bx",  "cx",  "dx"  },
    { "al",  "bl",  "cl",  "dl"  },
};

/*
register:
0 - RAX EAX AX AL
1 - RBX EBX BX BL
2 - RCX ECX CX CL
4 - RDX EDX DX DL
*/
#define GET_REG(source, register) __get_register__(get_variable_size((node)->token), register)
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
    snprintf(stack_buff, sizeof(stack_buff), "[rbp - %d]", ALIGN_TO(offset, 8));
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

#endif