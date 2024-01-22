#include <stdio.h>
#include <string.h>
/* 字符串一定要转义 \0 0*/
int main()
{
    char * ptr = "hello\\world";
    int len = strlen(ptr);

    printf("len:%d\n", len);
    printf("ptr:%s\n", ptr);

    return 0;
}