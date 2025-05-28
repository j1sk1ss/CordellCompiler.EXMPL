#ifndef QSORT_H_
#define QSORT_H_

#include "str.h"

int sort_qsort(void* arr, int arr_size, int el_size, int (*cmp)(const void*, const void*));

#endif