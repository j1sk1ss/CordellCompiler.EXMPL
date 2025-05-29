#ifndef STR_H_
#define STR_H_

#include <stddef.h>
#include <limits.h>

/*
Memory special functions. 
*/
void* str_memcpy(void* destination, const void* source, size_t num);
void* str_memset(void* pointer, unsigned char value, size_t num);
int str_memcmp(const void* firstPointer, const void* secondPointer, size_t num);

/*
String special functions.
*/
char* str_strncpy(char* dst, const char* src, int n);
int str_strcmp(const char* f, const char* s);
int str_strncmp(const char* str1, const char* str2, size_t n);
int str_atoi(const char *str);
unsigned int str_strlen(const char* str);
char* str_strcpy(char* dst, const char* src);
char* str_strcat(char* dest, const char* src);

/*
ctype special functions.
*/
int is_number(char* s);
int str_isdigit(int c);

#endif