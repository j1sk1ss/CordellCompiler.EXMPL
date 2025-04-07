#ifndef STR_H_
#define STR_H_

#include <stddef.h>
#include <limits.h>

void* str_memcpy(void* destination, void* source, size_t num);
void* str_memset(void* pointer, unsigned char value, size_t num);
int str_memcmp(void* firstPointer, void* secondPointer, size_t num);

char* str_strncpy(char* dst, char* src, int n);
int str_strcmp(char* f, char* s);
int str_atoi(char *str);

#endif