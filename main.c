#include <stdio.h>
#include <string.h>

#include  "my_malloc.h"

int main() 
{
    /*
    Blocks test_block;

    printf("%ld\n",sizeof(test_block.in_use));
    printf("%ld\n",sizeof(test_block.length));
    printf("%ld\n",sizeof(test_block.next));
    printf("%ld\n",sizeof(test_block.previous));
    printf("%ld\n",sizeof(test_block.marker));
    printf("%ld\n",sizeof(test_block));
    */

    // 1. test, testing my_malloc func
    int *a    = my_malloc(sizeof(int));
    char *b   = my_malloc(18);
    size_t *c = my_malloc(sizeof(size_t));

    if (a == NULL || b == NULL || c == NULL)
    {
        return 1; 
    }

    *a = 42;
    strcpy(b, "hello");
    *c = 123456;
    // 1. test, testing my_malloc func


    // 2. test, double free
    my_free(a); //32+16
    my_free(a); //32+16
    // 2. test, double free

    // 3. test, invalid pointer
    {
        char *invalid_pointer = "this is 3rd test";
        my_free(invalid_pointer); //undefined behaviour
    }
    // 3. test, invalid pointer


    my_free(b); //32+32
    my_free(c); //32+16


    

    return 0;
}
