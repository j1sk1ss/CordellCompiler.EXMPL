#ifndef STR_H_
#define STR_H_

#include <stddef.h>

void* str_memcpy(void* destination, void* source, size_t num);
void* str_memset(void* pointer, unsigned char value, size_t num);
int str_memcmp(void* firstPointer, void* secondPointer, size_t num);

#endif