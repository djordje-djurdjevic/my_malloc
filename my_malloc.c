#include <unistd.h>

#include "my_malloc.h"


static Blocks *head = NULL;
static Blocks *tail; // global/static is already null

void* my_malloc(size_t size)
{
    //alignment for multiples of 16
    if (size % ALIGNMENT) 
    {
        size = size + (ALIGNMENT - (size % ALIGNMENT));
    }

    //increasing the break equivalent to allocating memory
    void *ret_val = sbrk(sizeof(Blocks) + size); //sbrk returns the old break
    if(ret_val == (void *)-1) return NULL;


    Blocks *block = (Blocks *)ret_val;
    block->length = size;
    block->in_use = true;
    block->marker = BLOCK_MARKER;
    
    if (head == NULL)
    {
        block->previous = NULL;
        block->next = NULL;

        head = block;
        tail = block;
    }
    else 
    {
        block->previous = tail;
        block->next = NULL;
        
        tail->next = block;
        tail = block;
    }
                                // return (void *)(ret_val + sizeof(Blocks));
    return (void *)(block + 1); // next address after the block header
}