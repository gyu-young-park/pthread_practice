#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int thread_number[5] = {0,1,2,3,4};

typedef struct{
    int fuel;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}GasStation_t;

GasStation_t gGasStation ={
    .fuel = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
};

void* fuel_filling(void* arg){
    for(int i = 0; i < 5; i++){
        pthread_mutex_lock(&gGasStation.mutex);
        gGasStation.fuel += 60;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        pthread_cond_broadcast(&gGasStation.cond);
        sleep(1);
    }
}

void* car(void* arg){
    int id = *(int*)arg;
    pthread_mutex_lock(&gGasStation.mutex);
    while(gGasStation.fuel < 40){
        printf("ID:%d, No fuel. Waiting...\n", id);
        pthread_cond_wait(&gGasStation.cond, &gGasStation.mutex);
    }
    gGasStation.fuel -= 40;
    printf("ID: %d, Got fuel. Now left: %d\n",id,gGasStation.fuel);
    pthread_mutex_unlock(&gGasStation.mutex);
}

int main(int argc, char* argv[]){
    pthread_t th[5];
    for(int i = 0; i < 5; i++){
        if(i == 4){
            if(pthread_create(&th[i], NULL, &fuel_filling, NULL) != 0){
                perror("Failed to create thread");
            }
        }
        else{
            if(pthread_create(&th[i], NULL, &car, (void*)(thread_number + i)) != 0){
                perror("Failed to create thread");
            }
        }
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    pthread_mutex_destroy(&gGasStation.mutex);
    pthread_cond_destroy(&gGasStation.cond);
}