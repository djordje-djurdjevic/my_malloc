#include  "my_malloc.h"
#include  "test_funcs.h"

typedef struct Test
{
    char *name;
    void (*fn) (void);
} Test;

static Test tests[] = {
    {"basic malloc", test_basic},
    {"double free", test_double_free},
    {"invalid pointer", test_invalid_pointer}
};

int main() 
{
    //fflush(stdout);
    for(size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
    {
        call_test(tests[i].fn, tests[i].name);
    }
    

    return 0;
}
