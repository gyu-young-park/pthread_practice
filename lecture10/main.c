#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>                  

void* routine(void* arg){
    printf("hello routine!\n");
    return 0;
    printf("bye routine!\n");
}