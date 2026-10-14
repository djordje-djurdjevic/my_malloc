#include <unistd.h>

#include "my_malloc.h"


static Blocks *head = NULL;
static Blocks *tail; // global/static is already null

void* my_malloc(size_t size)
{   
    if(!size) return NULL;

    //alignment for multiples of 16
    if (size % ALIGNMENT) 
    {
        size = size + (ALIGNMENT - (size % ALIGNMENT));
    }


    //checking freed blocks
    Blocks *iter = head;
    while (iter != NULL)
    {
        if (!iter->in_use && iter->length >= size)
        {   
            //split if big enough, else give the whole block
            if (iter->length >= size + sizeof(Blocks) + ALIGNMENT) //32 for header and atleast 16 for new possible block
            {
        
                Blocks *new_block = (Blocks *) ((char *)(iter +1) + size);
                new_block->previous = iter;
                new_block->next     = iter->next;
                new_block->marker   = BLOCK_MARKER;
                new_block->in_use   = false;
                new_block->length   = iter->length - size - sizeof(Blocks);

                iter->in_use = true;
                iter->length = size;
                
                if (iter->next != NULL) 
                {
                    iter->next->previous = new_block;
                }
                else //if iter is last block
                {
                    tail = new_block;
                }
                iter->next = new_block;

            }
            else 
            {
                iter->in_use = true;
            }

            return (iter +1);
        }
        iter = iter->next;
    }

    //increasing the break equivalent to allocating memory
    void *ret_val = sbrk(sizeof(Blocks) + size); //sbrk returns the old break
    if (ret_val == (void *)-1) return NULL;


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
    return (void *)(block +1); // next address after the block header
}

void my_free(void *pointer)
{   
    if (pointer == NULL) return;

    Blocks *block = (Blocks *)pointer -1;

    if (block->marker != BLOCK_MARKER)
    {
        char temp[] = "my_free: invalid pointer\n"; 
        write(2, temp, sizeof(temp) -1);
        return;
    }
    if (!block->in_use)
    {
        char temp[] = "my_free: double free detected\n";
        write(2, temp, sizeof(temp) -1);
        return;
    }
    
    block->in_use = false;
}
