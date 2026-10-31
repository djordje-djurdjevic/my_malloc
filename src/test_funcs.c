#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "test_funcs.h"
#include "my_malloc.h"

void call_test(void (*fn)(void), const char *test_name)
{
    pid_t pid = fork();

    if(pid == -1)
    {   
        char temp[] = "Error: pid -1\n";
        ssize_t r = write(2, temp, sizeof(temp) -1);
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
            char temp[] = " chrashed\n\0";
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
    int *a    = my_malloc(sizeof(int));

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