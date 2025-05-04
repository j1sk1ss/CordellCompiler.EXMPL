#ifndef STRUCT_MEM_H_
#define STRUCT_MEM_H_

#include "mm.h"
#include "token.h"

/*
We save name of struct, also we save all field names.
Also we padd all fields when allocate memory in stack.
struct structName {
    int a; // 4 bytes
    // padding with 4 bytes

    long b; // 8 bytes

    arr array 100 char; // 100 bytes
    // padding with 4 bytes

    ptr char pointer; // 4 bytes
} // 128 bytes len

sizeof structName == 128
*/

typedef struct struct_field_info {
    int size;
    int offset;
    char name[TOKEN_MAX_SIZE];
    struct struct_field_info* next;
} struct_field_info_t;

typedef struct struct_info {
    char name[TOKEN_MAX_SIZE];
    struct struct_info* next;
    struct_field_info_t* field;
} struct_info_t;


struct_info_t* get_structmap_head();
int set_structmap_head(struct_info_t* h);
int unload_structmap(struct_info_t* h);

struct_info_t* get_struct_info(const char* name);
int add_struct_info(const char* name);
int add_struct_field(const char* struct_name, const char* filed_name, int field_size);

#endif