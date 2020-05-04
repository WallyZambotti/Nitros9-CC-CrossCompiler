#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("sizeof(pointer) = %d\n",sizeof(void*));
    printf("sizeof(short) = %d\n",sizeof(short));
    printf("sizeof(int) = %d\n",sizeof(int));
    printf("sizeof(long) = %d\n",sizeof(long));
    printf("sizeof(long long) = %d\n",sizeof(long long));
    return 0;
}