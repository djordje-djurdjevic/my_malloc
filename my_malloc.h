#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define ALIGNMENT 16
#define BLOCK_MARKER 0xDEADBEEF

void* my_malloc(size_t size);

typedef struct Blocks
{
    size_t length;   
    struct Blocks *next;
    struct Blocks *previous;
    uint32_t marker;
    bool in_use;
} Blocks;



#endif