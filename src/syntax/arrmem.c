#include "../../include/arrmem.h"


static array_info_t* _arrs_h = NULL;

static array_info_t* _create_info_array_entry(const char* name, int el_size, int size) {
    array_info_t* entry = (array_info_t*)mm_malloc(sizeof(array_info_t));
    if (!entry) return NULL;

    str_strncpy(entry->name, name, TOKEN_MAX_SIZE);
    entry->el_size = el_size;
    entry->size = size;
    entry->next = NULL;
    return entry;
}

int get_array_info(const char* name, array_info_t* info) {
    array_info_t* h = _arrs_h;
    while (h) {
        if (!str_strcmp(h->name, (char*)name)) {
            str_memcpy(info, h, sizeof(array_info_t));
            return 1;
        }

        h = h->next;
    }

    return 0;
}

int add_array_info(const char* name, int el_size, int size) {
    array_info_t* new_node = _create_info_array_entry(name, el_size, size);
    if (!new_node) return 0;

    array_info_t* h = _arrs_h;
    if (!h) {
        _arrs_h = new_node;
        return 1;
    }

    while (h->next) {
        h = h->next;
    }

    if (h) {
        h->next = new_node;
        if (h->next) return 1;
        else return -1;
    }

    return 0;
}

int unload_arrmap() {
    array_info_t* h = _arrs_h;
    while (h) {
        array_info_t* n = h->next;
        mm_free(h);
        h = n;
    }

    return 1;
}
