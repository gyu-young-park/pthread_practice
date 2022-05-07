#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

void* routine(void* arg){
    sleep(1);
    printf("hello routine start\n");
}

int main(int argc, char* argv[]){       
    pthread_t th[2];
    pthread_attr_t detachedThread;
    pthread_attr_init(&detachedThread);
    pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);

    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], &detachedThread, &routine, NULL) != 0){
            perror("Failed to create thread");
        }
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    pthread_attr_destroy(&detachedThread);
    pthread_exit(0);
}