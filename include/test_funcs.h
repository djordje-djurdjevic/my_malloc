#ifndef TEST_FUNCS_H
#define TEST_FUNCS_H

void call_test(void (*f)(void), const char *test_name);

void test_basic(void);
void test_double_free(void);
void test_invalid_pointer(void);

void reuse_same_size(void);
void split_single_block(void);
void split_middle_of_list(void);
void free_block_too_small(void);
void no_split_small_remainder(void);
void split_last_block(void);
void zero_size_and_free_null(void);
void data_integrity(void);

void merge_free_blocks_previous(void);
void merge_free_blocks_next(void);
void merge_free_blocks_next_and_previous(void);
void no_merge_free_middle_block(void);
void no_merge_hole_in_mem(void);
void split_on_merged_block(void);
void merged_header_not_valid(void);

void test_threads(void);
void *worker(void *arg);

void alignment_test(void);
void test_max_size(void);



#endif