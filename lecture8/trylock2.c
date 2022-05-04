#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

typedef struct {
    int32_t id;
    int32_t fuel;
    pthread_mutex_t mutex;
} Stove_t;

Stove_t stoves[4];

void* useStove(void* arg){
    int32_t isUsed = 0;
    while(isUsed == 0)
    {
        for(int i = 0; i < 4; i++){
            if(pthread_mutex_trylock(&stoves[i].mutex) == 0){
                int32_t usedFuel = rand() % 30;
                if(stoves[i].fuel - usedFuel >= 0){
                    stoves[i].fuel -= usedFuel;
                    isUsed = 1;
                    printf("Stove ID[%d]: remain fuel[%d]\n",stoves[i].id, stoves[i].fuel);
                    usleep(500000);
                }else{
                    printf("Stove ID[%d]: can't be used\n",stoves[i].id);
                }
                pthread_mutex_unlock(&stoves[i].mutex);
            }
            if(isUsed != 0) break;
        }
        if(isUsed == 0){
            printf("Can't use any stove sleep[0.5]\n");
            usleep(500000);
        }
    }
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 4; i++){
        stoves[i].id = i;
        stoves[i].fuel = 100;
        pthread_mutex_init(&stoves[i].mutex, NULL);
    }

    for(int i = 0; i < 10; i++){
        if(pthread_create(&th[i], NULL, useStove, NULL) != 0){
            perror("Failed to create thread\n");
        }
    }

    for(int i = 0; i < 10; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread\n");
        }
    }

    for(int i = 0; i < 4; i++){
        pthread_mutex_destroy(&stoves[i].mutex);
    }
}