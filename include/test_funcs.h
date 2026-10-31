#ifndef TEST_FUNCS_H
#define TEST_FUNCS_H

void call_test(void (*f)(void), const char *test_name);
void test_basic(void);
void test_double_free(void);
void test_invalid_pointer(void);

#endif