#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>

#include "test_funcs.h"
#include "my_malloc.h"

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