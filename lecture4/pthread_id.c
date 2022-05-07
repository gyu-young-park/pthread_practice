#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/syscall.h>

void* routine(void* arg){
    pthread_t id = pthread_self();
    printf("pthread[%lu] start!\n", id);
    printf("thread[%d] start!\n", (pid_t)syscall(SYS_gettid));
}

int main(int argc, char* argv[]){
    pthread_t th[2];
    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], NULL, routine, NULL) != 0){
            perror("Failed to create thread");
        }
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
}