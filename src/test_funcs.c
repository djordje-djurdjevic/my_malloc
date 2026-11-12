#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>


#include "test_funcs.h"
#include "my_malloc.h"

#include <pthread.h>
#include <sched.h>

#define NUM_OF_THREADS 10
#define THREAD_ITER 1000

void call_test(void (*fn)(void), const char *test_name)
{
    pid_t pid = fork();

    if(pid == -1)
    {   
        char temp[] = "Error: pid -1\n";
        ssize_t r = write(2, temp, sizeof(temp) - 1);
        (void)r;
        return;
    }
    else if(pid == 0)
    {
        fn();
        exit(0);
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);

        if (WIFSIGNALED(status)) //child process terminated by a signal
        {
            ssize_t r = write(2, test_name, strlen(test_name));
            char temp[] = " crashed\n\0";
            r = write(2, temp, sizeof(temp));
            (void )r;
        }
        else if (WIFEXITED(status))
        {
            ssize_t r = write(2, test_name, strlen(test_name));
            char temp[] = " passed\n\0";
            r = write(2, temp, sizeof(temp));
            (void )r;
        }
        else
        {
            ssize_t r = write(2, test_name, strlen(test_name));
            char temp[] = " failed\n\0";
            r = write(2, temp, sizeof(temp));
            (void )r;
        }
    }
}


void test_basic(void)
{
    // 1. test, testing my_malloc func
    int *a    = my_malloc(sizeof(int));
    char *b   = my_malloc(18);
    size_t *c = my_malloc(sizeof(size_t));

    if (a == NULL || b == NULL || c == NULL)
    {
        return; 
    }

    *a = 42;
    strcpy(b, "hello");
    *c = 123456;
    // 1. test, testing my_malloc func
}

void test_double_free(void)
{   
    int *a = my_malloc(sizeof(int));

    // 2. test, double free
    my_free(a); //32+16
    my_free(a); //32+16
    // 2. test, double free   
}

void test_invalid_pointer(void)
{
    // 3. test, invalid pointer
    size_t invalid_pointer[32] = {0};
    my_free(invalid_pointer + 4);
    // 3. test, invalid pointer
}

void reuse_same_size(void)
{   
    /* 
    test_freed_block1: allocates a block, frees it, then allocates the same size.
    Checks that the freed block is reused (same address). 
    */
    
    int *a = my_malloc(sizeof(int));
    my_free(a);
    int *b = my_malloc(sizeof(int));

    assert(b == a);
}

void split_single_block(void)
{   
    /*
    test_freed_block2: frees a 400-byte block, then requests 16 bytes.
    Checks the split: the first block becomes 16, the remainder is a new free block.
    */

    int *a = my_malloc(100 * sizeof(int));
    my_free(a);
    int *b = my_malloc(sizeof(int));

    assert(b == a);
    assert(((Blocks *)b - 1)->length == 16);
    assert(((Blocks *)b - 1)->in_use == true);


    Blocks *c = (Blocks *)((char *)b + 16); //header of next block
    assert(c->marker == BLOCK_MARKER); 
    assert(c->in_use == false);
    assert(c->length == 100 * sizeof(int) - 16 - sizeof(Blocks));
    assert(((Blocks *)b - 1)->next == c);

}

void split_middle_of_list(void)
{
    /*
    test_freed_block3: splits a free block in the middle of the list (c follows it).
    Checks the remainder is inserted between d and c with correct previous/next links.
    */
    int *a = my_malloc(100 * sizeof(int));
    int *b = my_malloc(100 * sizeof(int));
    int *c = my_malloc(100 * sizeof(int));
    my_free(b);

    int *d = my_malloc(sizeof(int));



    assert(d == b);
    assert(((Blocks *)a - 1)->next == (Blocks *)d - 1);

    assert(((Blocks *)d - 1)->length == 16);
    assert(((Blocks *)d - 1)->in_use == true);


    Blocks *e = (Blocks *)((char *)d + 16); //header of next block
    assert(e->marker == BLOCK_MARKER); 
    assert(e->in_use == false);
    assert(e->length == 100 * sizeof(int) - 16 - sizeof(Blocks));
    assert(((Blocks *)d - 1)->next == e);

    //e is between d and c
    assert(e->previous == (Blocks *)d - 1);
    assert(e->next == (Blocks *)c - 1);
    assert(((Blocks *)c - 1)->previous == e);
}

void free_block_too_small(void)
{

    /*
    test_freed_block4: the free block is too small for a 400-byte request.
    Checks a new block is appended to the list and the free one stays untouched.
    */
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    my_free(a);

    int *c = my_malloc(100 * sizeof(int));

    assert(a != c);
    assert(c >  b);

    assert(!((Blocks *)a - 1)->in_use);
    assert( ((Blocks *)c - 1)->in_use);
    assert( ((Blocks *)c - 1)->length   == 100 * sizeof(int));
    assert( ((Blocks *)c - 1)->previous == (Blocks *)b -1);
    assert( ((Blocks *)b - 1)->next     == (Blocks *)c -1);

}

void no_split_small_remainder(void)
{   
    /*
    test_freed_block5: free block of 48 bytes, request of 16, remainder too small to split.
    Checks the whole block is returned (length stays 48).
    */

    int *a = my_malloc(12 * sizeof(int)); //48
    my_free(a);
    int *b = my_malloc(4 * sizeof(int)); //16

    assert(b == a);
    assert(((Blocks *)b - 1)->length == 48);
    assert(((Blocks *)b - 1)->next == NULL);
}

void split_last_block(void)
{
    /*
    test_freed_block6: splits a free block that is the last one in the list (tail).
    Checks tail moves to the remainder, so the next new block goes after it.
    */
   
    int *a = my_malloc(100 * sizeof(int)); //400
    my_free(a);
    int *b = my_malloc(sizeof(int)); //16 + 32
    int *c = my_malloc(100 * sizeof(int));

    assert(b == a);
    assert(!((Blocks *)((char *)b + 16))->in_use);
    assert( ((Blocks *)((char *)b + 16))->length == 352); //400 - 16 - 32
    assert( ((Blocks *)((char *)b + 16))->marker == BLOCK_MARKER);
    assert(c > b);
    assert(((Blocks *)((char *)b + 16))->next == (Blocks *)c - 1);
    assert(((Blocks *)c - 1)->previous == ((Blocks *)((char *)b + 16)) );
}

void zero_size_and_free_null(void)
{
    int *a = my_malloc(0);
    assert(a == NULL);
    my_free(a);

    my_free(my_malloc(0));
}

void data_integrity(void)
{
    int *a = my_malloc(100 * sizeof(int));
    int *b = my_malloc(100 * sizeof(int));
    int *c = my_malloc(100 * sizeof(int));

    memset(a, 0xAA, ((Blocks *)a - 1)->length);
    memset(b, 0xBB, ((Blocks *)b - 1)->length);
    memset(c, 0xCC, ((Blocks *)c - 1)->length);

    assert(((Blocks *)a - 1)->marker == BLOCK_MARKER);
    assert(((Blocks *)b - 1)->marker == BLOCK_MARKER);
    assert(((Blocks *)c - 1)->marker == BLOCK_MARKER);

    unsigned char *pa = (unsigned char *)a;
    unsigned char *pb = (unsigned char *)b;
    unsigned char *pc = (unsigned char *)c;
    
    for(size_t i = 0; i < ((Blocks *)a - 1)->length; i++)
    {
        assert(pa[i] == 0XAA);
        assert(pb[i] == 0XBB);
        assert(pc[i] == 0XCC);
    }

    my_free(b);
    int *d = my_malloc(100 * sizeof(int));
    
    assert(d == b);

    for(size_t i = 0; i < ((Blocks *)a - 1)->length; i++)
    {   
        assert(pa[i] == 0XAA);
        assert(pc[i] == 0XCC);
    }
}

void merge_free_blocks(void)
{
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    int *c = my_malloc(sizeof(int));

    my_free(a);
    my_free(b);

    int *d = my_malloc(64); // 16 + 16 + 32

    assert(d == a);
    assert(((Blocks *)d - 1)->length == 64);
    assert(((Blocks *)d - 1)->next == (Blocks *)c - 1);
    assert(((Blocks *)c - 1)->previous == (Blocks *)d - 1);

}

void merge_free_blocks_previous(void)
{
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    int *c = my_malloc(sizeof(int));

    my_free(a);
    my_free(b);

    int *d = my_malloc(64); // 16 + 16 + 32

    assert(d == a);
    assert(((Blocks *)d - 1)->length == 64);
    assert(((Blocks *)d - 1)->next == (Blocks *)c - 1);
    assert(((Blocks *)c - 1)->previous == (Blocks *)d - 1);

}

void merge_free_blocks_next(void)
{   
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    int *c = my_malloc(sizeof(int));

    my_free(b);
    my_free(a);

    int *d = my_malloc(64); // 16 + 16 + 32

    assert(d == a);
    assert(((Blocks *)d - 1)->length == 64);
    assert(((Blocks *)d - 1)->next == (Blocks *)c - 1);
    assert(((Blocks *)c - 1)->previous == (Blocks *)d - 1);
}

void merge_free_blocks_next_and_previous(void)
{
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    int *c = my_malloc(sizeof(int));

    my_free(a);
    my_free(c);
    my_free(b);

    assert( ((Blocks *)a - 1)->length == 112);
    assert( ((Blocks *)a - 1)->next == NULL);
    assert(!((Blocks *)a - 1)->in_use);


    int *d = my_malloc(64 + 48); // 64+48   
    int *e = my_malloc(sizeof(int));

    assert(d == a);
    assert(((Blocks *)d - 1)->length == 64 + 48);

    //e is the last block
    assert(e > d);
    assert(((Blocks *)d - 1)->next == (Blocks *)e - 1);
    assert(((Blocks *)e - 1)->previous == (Blocks *)d - 1);
}

void no_merge_free_middle_block(void)
{
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    int *c = my_malloc(sizeof(int));

    my_free(b);

    assert(((Blocks *)b - 1)->length == 16);
    assert(((Blocks *)a - 1)->next == (Blocks *)b - 1);
    assert(((Blocks *)c - 1)->previous == (Blocks *) b - 1);
}

void no_merge_hole_in_mem(void)
{
    int *a = my_malloc(sizeof(int));
    sbrk(16);
    int *b = my_malloc(sizeof(int));

    my_free(a);
    my_free(b);

    assert(((Blocks *)a - 1)->length == 16);
    assert(((Blocks *)b - 1)->length == 16);

    assert(((Blocks *)a - 1)->next == (Blocks *)b - 1);
}

void split_on_merged_block(void)
{
    int *a = my_malloc(16);
    int *b = my_malloc(16);

    my_free(a);
    my_free(b);

    int *c = my_malloc(sizeof(int));

    assert(c == a);

    assert(((Blocks *)c - 1)->length == 16);
    assert(((Blocks *)c - 1)->next->length == 16);
    assert(!((Blocks *)c - 1)->next->in_use);
    assert(((Blocks *)c - 1)->next->next == NULL);

}


void merged_header_not_valid(void)
{
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));

    my_free(a);
    my_free(b);

    my_free(b);
    assert(((Blocks *)b - 1)->marker == 0);    

}

void test_threads(void)
{
    pthread_t threads[NUM_OF_THREADS];
    size_t arg[NUM_OF_THREADS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    for(size_t i = 0; i < NUM_OF_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, worker, (void *)&arg[i]);
    }

    char temp[32];
    for(size_t i = 0; i < NUM_OF_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        size_t len = snprintf(temp, 31, "Thread[%zu] finished\n", i);
        ssize_t r = write(2, temp, len);
        (void)r;
    }
}    

void *worker(void *arg)
{
    int id = *(size_t *)arg;
    int *p[THREAD_ITER];

    for (size_t i = 0; i < THREAD_ITER; i++)
    {
        p[i] = my_malloc(4 * ((i % 10) + 1));
        assert(p[i] != NULL);
    }

    int data_val[NUM_OF_THREADS] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44};
    for (size_t i = 0; i < THREAD_ITER; i++)
    {
        memset(p[i], data_val[id], ((Blocks *)p[i] - 1)->length);
        sched_yield(); //increases chance for race condition to happen
    }
        
    for (size_t i = 0; i < THREAD_ITER; i++)
    {
        unsigned char *p_char = (unsigned char *)p[i];

        for(size_t j = 0; j < ((Blocks *)p[i] - 1)->length; j++)
        {
            assert(p_char[j] == data_val[id]);
        }

        my_free(p[i]);
    }

    return NULL;
}


void alignment_test(void)
{
    int *a = my_malloc(1);
    int *b = my_malloc(7);
    int *c = my_malloc(16);
    int *d = my_malloc(17);
    int *e = my_malloc(100);
    int *f = my_malloc(1000);

    assert((uintptr_t)a % ALIGNMENT == 0);
    assert((uintptr_t)b % ALIGNMENT == 0);
    assert((uintptr_t)c % ALIGNMENT == 0);
    assert((uintptr_t)d % ALIGNMENT == 0);
    assert((uintptr_t)e % ALIGNMENT == 0);
    assert((uintptr_t)f % ALIGNMENT == 0);

    //after free and split
    my_free(f);

    int *g = my_malloc(32);
    int *h = my_malloc(32);
    int *i = my_malloc(32);

    assert((uintptr_t)g % ALIGNMENT == 0);
    assert((uintptr_t)h % ALIGNMENT == 0);
    assert((uintptr_t)i % ALIGNMENT == 0);

    //after free and merge
    my_free(a);
    my_free(b);
    
    int *j = my_malloc(64);
    
    assert(j == a);
    assert((uintptr_t)j % ALIGNMENT == 0);

    my_free(c);
    my_free(d);
    my_free(e);
    my_free(g);
    my_free(h);
    my_free(i);
    my_free(j);
}

void test_max_size(void)
{
    assert(my_malloc(SIZE_MAX)      == NULL);
    assert(my_malloc(SIZE_MAX - 1 ) == NULL);
    assert(my_malloc(SIZE_MAX - 31) == NULL);
    assert(my_malloc(SIZE_MAX - 47) == NULL);

 
    int *a = my_malloc(16);
    assert(my_malloc(1UL << 50) == NULL);
    int *b = my_malloc(16);

    assert(a != NULL);
    assert(b != NULL);
    assert(b > a);
    
    assert(((Blocks *)a - 1)->next     == (Blocks *)b - 1);
    assert(((Blocks *)b - 1)->previous == (Blocks *)a - 1);

    my_free(a);
    my_free(b);
}