#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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
        gGasStation.fuel += 15;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        sleep(1);
    }
}

void* car(void* arg){
    pthread_mutex_lock(&gGasStation.mutex);
    while(gGasStation.fuel < 40){
        printf("No fuel. Waiting...\n");
        sleep(1);
    }
    gGasStation.fuel -= 40;
    printf("Got fuel. Now left: %d\n",gGasStation.fuel);
    pthread_mutex_unlock(&gGasStation.mutex);
}

int main(int argc, char* argv[]){
    pthread_t th[2];
    for(int i = 0; i < 2; i++){
        if(i == 1){
            if(pthread_create(&th[i], NULL, &fuel_filling, NULL) != 0){
                perror("Failed to create thread");
            }
        }
        else{
            if(pthread_create(&th[i], NULL, &car, NULL) != 0){
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