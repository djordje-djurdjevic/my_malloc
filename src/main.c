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
    {"reuse same size",                     reuse_same_size},
    {"split single block",                  split_single_block},
    {"split middle of list",                split_middle_of_list},
    {"free block too small",                free_block_too_small},
    {"no split small remainder",            no_split_small_remainder},
    {"split last block",                    split_last_block},
    {"zero size and free null",             zero_size_and_free_null},
    {"data integrity",                      data_integrity},
    {"merge free blocks next",              merge_free_blocks_next},
    {"merge free blocks previous",          merge_free_blocks_previous},
    {"merge free blocks next and previous", merge_free_blocks_next_and_previous},
    {"no merge middle block",               no_merge_free_middle_block},
    {"no merge hole in memory",             no_merge_hole_in_mem},
    {"split on merged block",               split_on_merged_block},
    {"merged header not valid",             merged_header_not_valid},
    {"thread test",                         test_threads},
    {"alignment test",                      alignment_test},
    {"allocate MAX_SIZE",                   test_max_size},
    {"return memory to kernel",             return_memory_to_kernel},
    {"return memory hole before tail",      return_memory_hole_before_tail},
    {"no return hole after tail",           no_return_hole_after_tail},
    {"return memory all blocks",            return_memory_all_blocks},
    {"test calloc",                         test_calloc},
    {"calloc test max size",                calloc_test_max_size},
    {"calloc parameters zero",              calloc_parameter_zero},
    {"realloc null parameter",              realloc_null_parameter},
    {"realloc zero parameter",              realloc_zero_parameter},
    {"realloc increase size",               realloc_increase_size},
    {"realloc decrease size",               realloc_decrease_size},
    {"realloc freed pointer",               realloc_freed_pointer},
    {"realloc invalid pointer",             realloc_invalid_pointer},
    {"realloc test max size",               realloc_test_max_size},
    {"realloc with adjacent blocks",        realloc_with_adjacent_blocks},
    {"realloc_freed_block",                 realloc_freed_block}
};


int main() 
{
    for(size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
    {   
        call_test(tests[i].fn, tests[i].name);
    }

    return 0;
}

