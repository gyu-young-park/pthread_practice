#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    int amount;
    pthread_mutex_t mutex;
}Resource;

typedef struct{
    Resource fuel;
    Resource water;
} GasStation;

GasStation gasStation = {
    .fuel.amount = 50,
    .water.amount = 10,
    .fuel.mutex = PTHREAD_MUTEX_INITIALIZER,
    .water.mutex = PTHREAD_MUTEX_INITIALIZER, 
};

void* car1(void* arg){
    pthread_mutex_lock(&gasStation.fuel.mutex);
    sleep(1);
    pthread_mutex_lock(&gasStation.water.mutex);
    gasStation.fuel.amount -= 10;
    gasStation.water.amount -= 2;
    pthread_mutex_unlock(&gasStation.water.mutex);
    pthread_mutex_unlock(&gasStation.fuel.mutex);
}

void* car2(void* arg){
    pthread_mutex_lock(&gasStation.fuel.mutex);
    pthread_mutex_lock(&gasStation.water.mutex);
    sleep(1);
    gasStation.water.amount -= 2;
    gasStation.fuel.amount -= 10;
    pthread_mutex_unlock(&gasStation.water.mutex);
    pthread_mutex_unlock(&gasStation.fuel.mutex);
}

int main(int argc, char* argv[]){
    pthread_t th[2];
    for(int i = 0; i < 2; i++){
        if(i == 0){
            if(pthread_create(&th[i], NULL, car1, NULL) != 0){
               perror("Failed to create thread 1");
            }
        }
        else{
            if(pthread_create(&th[i], NULL, car2, NULL) != 0){
               perror("Failed to create thread 2");
            }
        }
    }
    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    printf("Remaining Fuel:[%d]\n",gasStation.fuel.amount);
    printf("Remaining water:[%d]\n",gasStation.water.amount);
}