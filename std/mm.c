#include "../include/mm.h"


static unsigned char _buffer[ALLOC_BUFFER_SIZE];
static mm_block_t* _mm_head = (mm_block_t*)_buffer;
static int _allocated = 0;


int mm_init() {
    _mm_head->magic = MM_BLOCK_MAGIC;
    _mm_head->size  = ALLOC_BUFFER_SIZE - sizeof(mm_block_t);
    _mm_head->free  = 1;
    _mm_head->next  = NULL;
    return 1;
}

static int __coalesce_memory() {
    int merged = 0;
    mm_block_t* current = _mm_head;
    
    do {
        merged = 0;
        current = _mm_head;

        while (current && current->next) {
            if (current->free && current->next->free) {
                current->size += sizeof(mm_block_t) + current->next->size;
                current->next = current->next->next;
                merged = 1;
            } else {
                current = current->next;
            }
        }
    } while (merged);
    return 1;
}

static void* __malloc_s(size_t size, int prepare_mem) {
    if (size == 0) return NULL;
    if (prepare_mem) __coalesce_memory();

    size = (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
    mm_block_t* current = _mm_head;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size >= size + sizeof(mm_block_t)) {
                mm_block_t* new_block = (mm_block_t*)((unsigned char*)current + sizeof(mm_block_t) + size);
                new_block->magic = MM_BLOCK_MAGIC;
                new_block->size = current->size - size - sizeof(mm_block_t);
                new_block->free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size;
            }

            current->free = 0;
            _allocated += current->size + sizeof(mm_block_t);
            print_mm("Allocated node [%p] with [%i] size / [%i]", (unsigned char*)current + sizeof(mm_block_t), current->size, _allocated);
            return (unsigned char*)current + sizeof(mm_block_t);
        }

        current = current->next;
    }

    print_mm("Allocation error! I can't allocate [%i]!", size);
    return prepare_mem ? NULL : __malloc_s(size, 1);
}

void* mm_malloc(size_t size) {
    void* ptr = __malloc_s(size, 0);
    if (!ptr) print_mm("Allocation error! I can't allocate [%i]!", size);
    return ptr;
}

void* mm_realloc(void* ptr, size_t elem) {
    void* new_data = NULL;
    if (elem) {
        if(!ptr) return mm_malloc(elem);
        new_data = mm_malloc(elem);
        if(new_data) {
            str_memcpy(new_data, ptr, elem);
            mm_free(ptr);
        }
    }

    return new_data;
}

int mm_free(void* ptr) {
    if (!ptr || ptr < (void*)_buffer || ptr >= (void*)(_buffer + ALLOC_BUFFER_SIZE)) return -1;
    
    mm_block_t* block = (mm_block_t*)((unsigned char*)ptr - sizeof(mm_block_t));
    if (block->magic != MM_BLOCK_MAGIC) return -1;
    if (block->free) return -1;

    block->free = 1;
    _allocated -= block->size + sizeof(mm_block_t);
    print_mm("Free [%p] with [%i] size / [%i]", ptr, block->size, _allocated);
    
    return 1;
}