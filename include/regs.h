#ifndef REGS_H_
#define REGS_H_

#include "str.h"
#include "logg.h"

typedef struct {
    const char* name;
    const char* operation;
    const char* move;
} regs_t;

int get_reg(regs_t* regs, int size, const char* base64, int ptr);

#endif