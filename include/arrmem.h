#ifndef ARRMEM_H_
#define ARRMEM_H_

#include "str.h"
#include "token.h"

typedef struct array_info {
    int size;
    int el_size;
    char func[TOKEN_MAX_SIZE];
    char name[TOKEN_MAX_SIZE];
    struct array_info* next;
} array_info_t;


int add_array_info(const char* name, const char* func, int el_size, int size);
int get_array_info(const char* name, const char* func, array_info_t* info);

int unload_arrmap();

#endif