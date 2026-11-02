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

#endif