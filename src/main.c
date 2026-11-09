#include  "my_malloc.h"
#include  "test_funcs.h"

typedef struct Test
{
    char *name;
    void (*fn) (void);
} Test;

static Test tests[] = {
    {"basic malloc",                        test_basic},
    {"double free",                         test_double_free},
    {"invalid pointer",                     test_invalid_pointer},
    {"reuse_same_size",                     reuse_same_size},
    {"split_single_block",                  split_single_block},
    {"split_middle_of_list",                split_middle_of_list},
    {"free_block_too_small",                free_block_too_small},
    {"no_split_small_remainder",            no_split_small_remainder},
    {"split_last_block",                    split_last_block},
    {"zero size and free null",             zero_size_and_free_null},
    {"data integrity",                      data_integrity},
    {"merge free blocks next",              merge_free_blocks_next},
    {"merge free blocks previous",          merge_free_blocks_previous},
    {"merge free blocks next and previous", merge_free_blocks_next_and_previous},
    {"no merge middle block",               no_merge_free_middle_block},
    {"no merge hole in memory",             no_merge_hole_in_mem},
    {"split on merged block",               split_on_merged_block},
    {"mergeed header not valid",            merged_header_not_valid},
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
