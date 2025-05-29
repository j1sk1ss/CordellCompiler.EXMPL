#include "../include/qsort.h"


static void _swap(void* a, void* b, size_t el_size) {
    char* temp = (char*)mm_malloc(el_size);
    str_memcpy(temp, a, el_size);
    str_memcpy(a, b, el_size);
    str_memcpy(b, temp, el_size);
    mm_free(temp);
}

static int _work(void* arr, int low, int high, size_t el_size, int (*cmp)(const void*, const void*)) {
    void* pivot = (char*)arr + high * el_size;
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        void* current = (char*)arr + j * el_size;
        if (cmp(current, pivot) <= 0) _swap((char*)arr + ++i * el_size, current, el_size);
    }

    _swap((char*)arr + (i + 1) * el_size, (char*)arr + high * el_size, el_size);
    return i + 1;
}

static void _quicksort(void* arr, int low, int high, size_t el_size, int (*cmp)(const void*, const void*)) {
    if (low < high) {
        int pi = _work(arr, low, high, el_size, cmp);
        _quicksort(arr, low, pi - 1, el_size, cmp);
        _quicksort(arr, pi + 1, high, el_size, cmp);
    }
}

int sort_qsort(void* arr, int arr_size, int el_size, int (*cmp)(const void*, const void*)) {
    if (!arr || arr_size <= 0 || el_size <= 0 || !cmp) return 0;
    _quicksort(arr, 0, arr_size - 1, el_size, cmp);
    return 1;
}
