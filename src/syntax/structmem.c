#include "../../include/structmem.h"


typedef struct struct_association {
    char name[TOKEN_MAX_SIZE];
    struct_field_info_t* info;
    struct struct_association* next;
} struct_association_t;

static struct_association_t* _associations_h = NULL;

static struct_association_t* _create_association(const char* name, struct_field_info_t* info) {
    struct_association_t* node = (struct_association_t*)mm_malloc(sizeof(struct_association_t));
    if (!node) return NULL;

    str_strncpy(node->name, name, TOKEN_MAX_SIZE);
    node->info = info;
    node->next = NULL;
    return node;
}

int unload_associations() {
    while (_associations_h) {
        struct_association_t* n = _associations_h->next;
        mm_free(_associations_h);
        _associations_h = n;
    }

    return 1;
}

int register_association(const char* name, struct_field_info_t* info) {
    struct_association_t* node = _create_association(name, info);
    if (!node) return NULL;
    if (!_associations_h) {
        _associations_h = node;
        return 1;
    }

    struct_association_t* h = _associations_h;
    while (h->next) h = h->next;
    h->next = node;
    return 1;
}

struct_field_info_t* get_associated_struct(const char* name) {
    struct_association_t* h = _associations_h;
    while (h) {
        if (!str_strcmp(h->name, name)) return h;
        h = h->next;
    }

    return NULL;
}

static struct_info_t* _struct_h = NULL;

struct_info_t* get_structmap_head() {
    return _struct_h;
}

int set_structmap_head(struct_info_t* h) {
    _struct_h = h;
    return 1;
}

static struct_field_info_t* _create_info_field_entry(const char* name, int size) {
    struct_field_info_t* entry = (struct_field_info_t*)mm_malloc(sizeof(struct_field_info_t));
    if (!entry) return NULL;

    str_strncpy(entry->name, name, TOKEN_MAX_SIZE);
    entry->next = NULL;
    entry->size = size;
    entry->offset = 0;
    return entry;
}

static struct_info_t* _create_info_struct_entry(const char* name) {
    struct_info_t* entry = (struct_info_t*)mm_malloc(sizeof(struct_info_t));
    if (!entry) return NULL;

    str_strncpy(entry->name, name, TOKEN_MAX_SIZE);
    entry->next = NULL;
    entry->field = NULL;
    entry->offset = 0;
    return entry;
}

struct_info_t* get_struct_info(const char* name) {
    struct_info_t* h = _struct_h;
    while (h) {
        if (!str_strcmp(h->name, (char*)name)) {
            return h;
        }

        h = h->next;
    }

    return NULL;
}

int add_struct_info(const char* name) {
    struct_info_t* new_node = _create_info_struct_entry(name);
    if (!new_node) return 0;

    struct_info_t* h = _struct_h;
    if (!h) {
        _struct_h = new_node;
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

int add_struct_field(const char* struct_name, const char* filed_name, int field_size) {
    struct_info_t* info = get_struct_info(struct_name);
    if (!info) return 0;

    struct_field_info_t* field = _create_info_field_entry(filed_name, field_size);
    if (!field) return 0;

    int filed_offset = 0;
    struct_field_info_t* field_h = info->field;
    if (!field_h) {
        info->field = field;
        return 1;
    }

    filed_offset += field_h->size;
    while (field_h->next) {
        field_h = field_h->next;
        filed_offset += field_h->size;
    }

    field->offset = filed_offset;
    field_h->next = field;
    return 1;
}

int unload_structmap(struct_info_t* h) {
    while (h) {
        while (h->field) {
            struct_field_info_t* nf = h->field->next;
            mm_free(h->field);
            h->field = nf;
        }

        struct_info_t* n = h->next;
        mm_free(h);
        h = n;
    }

    return 1;
}
