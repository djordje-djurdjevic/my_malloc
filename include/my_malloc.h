#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT 16
#define BLOCK_MARKER 0xDEADBEEF
#define LEN_4KB 4096

void* my_malloc(size_t size);
void  my_free(void *ptr);

void* my_calloc(size_t num, size_t size);
void* my_realloc(void *ptr, size_t size);

typedef struct Blocks
{
    size_t length;   
    struct Blocks *next;
    struct Blocks *previous;
    uint_least32_t marker;
    bool in_use;
} Blocks;



#endif