#include "../include/str.h"


void* str_memcpy(void* destination, void* source, size_t num) {
    unsigned int num_dwords = num / 4;
    unsigned int num_bytes = num % 4;
    unsigned int* dest32 = (unsigned int*)destination;
    unsigned int* src32 = (unsigned int*)source;
    unsigned char* dest8 = ((unsigned char*)destination) + num_dwords * 4;
    unsigned char* src8 = ((unsigned char*)source) + num_dwords * 4;
    unsigned int i = 0;

    for (i = 0; i < num_dwords; i++) dest32[i] = src32[i];
    for (i = 0; i < num_bytes; i++) dest8[i] = src8[i];

    return destination;
}

void* str_memset(void* pointer, unsigned char value, size_t num) {
    unsigned int num_dwords = num / 4;
    unsigned int num_bytes = num % 4;
    unsigned int *dest32 = (unsigned int*)pointer;
    unsigned char *dest8 = ((unsigned char*)pointer) + num_dwords * 4;
    unsigned char val8 = (unsigned char)value;
    unsigned int val32 = value | (value << 8) | (value << 16) | (value << 24);
    unsigned int i = 0;

    for (i = 0; i < num_dwords; i++) dest32[i] = val32;
    for (i = 0; i < num_bytes; i++) dest8[i] = val8;
    
    return pointer;
}

int str_memcmp(void* firstPointer, void* secondPointer, size_t num) {
    const unsigned char* u8Ptr1 = (const unsigned char *)firstPointer;
    const unsigned char* u8Ptr2 = (const unsigned char *)secondPointer;
    for (unsigned short i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}
