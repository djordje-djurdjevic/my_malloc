#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include "my_malloc.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static Blocks *head = NULL;
static Blocks *tail; // global/static is already null

void* my_malloc(size_t size)
{   
    if (!size) return NULL;
    if (size > SIZE_MAX - ALIGNMENT - sizeof(Blocks)) return NULL;

    //alignment for multiples of 16
    if (size % ALIGNMENT) 
    {
        size = size + (ALIGNMENT - (size % ALIGNMENT));
    }


    pthread_mutex_lock(&mutex); 
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

            pthread_mutex_unlock(&mutex);
            return (iter +1);
        }
        iter = iter->next;
    }

    //increasing the break equivalent to allocating memory
    void *ret_val = sbrk(sizeof(Blocks) + size); //sbrk returns the old break
    if (ret_val == (void *)-1) 
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    


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


    pthread_mutex_unlock(&mutex);                                
    return (void *)(block + 1); // next address after the block header,  ||  return (void *)(ret_val + sizeof(Blocks));
}

void my_free(void *pointer)
{   
    if (pointer == NULL) return;


    pthread_mutex_lock(&mutex);
    Blocks *block = (Blocks *)pointer - 1;

    if (block->marker != BLOCK_MARKER)
    {
        pthread_mutex_unlock(&mutex);
        char temp[] = "my_free: invalid pointer\n"; 
        ssize_t r = write(2, temp, sizeof(temp) - 1);
        (void)r;
        return;
    }
    if (!block->in_use)
    {
        pthread_mutex_unlock(&mutex);
        char temp[] = "my_free: double free detected\n";
        ssize_t r = write(2, temp, sizeof(temp) - 1);
        (void)r;
        return;
    }
    
    block->in_use = false;

    //merging free memory
    if ((block->next != NULL && !block->next->in_use) && ((Blocks *)((char *)(block + 1) + block->length) == block->next))
    {
        block->length += sizeof(Blocks) + block->next->length;
        block->next->marker = 0;


        if (block->next->next != NULL)
        {
            block->next->next->previous = block;
            block->next = block->next->next;
        }
        else
        {
            tail = block;
            block->next = NULL;
        }
    }
    if((block->previous != NULL && !block->previous->in_use) && ((Blocks *)((char *)(block - 1) - block->previous->length) == block->previous))
    {
        block->previous->length += sizeof(Blocks) + block->length;
        block->marker = 0;


        if (block->next != NULL)
        {
            block->previous->next = block->next;
            block->next->previous = block->previous;
        }
        else
        {
            tail = block->previous;
            block->previous->next = NULL;
        }
    }

    pthread_mutex_unlock(&mutex);
}
