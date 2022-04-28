#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int prime[10] = {2, 3, 5, 7, 11, 13, 17, 19,23, 29};

void* routine(void* arg){
    int index = *(int *)arg;
    printf("%d ",prime[index]);
    free(arg);
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 10; i++){
        int* index = malloc(sizeof(int));
        *index = i;
        if(pthread_create(&th[i], NULL, routine, index) != 0){
            perror("Failed to create thread");
        }
    }

    for(int i = 0; i < 10; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    return 0;
}