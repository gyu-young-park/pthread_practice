#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>                  

void* routine(void* arg){
    printf("hello routine!\n");
    return 0;
    printf("bye routine!\n");
}

int main(){
    pthread_t th;
    if(pthread_create(&th, NULL, routine, NULL) != 0){
        perror("Failed to create thread");
    }
    pthread_exit(0);
    if(pthread_join(th, NULL) != 0){
        perror("Failed to join thread");
    }
    printf("Done process exit\n");
} 