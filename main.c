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

    //1. test
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

    return 0;
}