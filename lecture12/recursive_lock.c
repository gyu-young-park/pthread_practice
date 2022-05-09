#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

typedef struct {
    int amount;
    pthread_mutex_t mutex;
}Resource;

Resource resource = {
    .amount = 10,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

void* routine(void* arg){
    printf("Start routine\n");
    pthread_mutex_lock(&resource.mutex);
    resource.amount -= 2;
    sleep(1);
    printf("resource value:[%d]\n",resource.amount);
    if(resource.amount > 0){
        routine(NULL);
    }
    pthread_mutex_unlock(&resource.mutex);
    return 0;
}

int main(int argc, char* argv[]){
    pthread_mutexattr_t recursiveAttr;
    pthread_mutexattr_init(&recursiveAttr);
    pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&resource.mutex,&recursiveAttr);

    pthread_t th;
    if(pthread_create(&th, NULL, routine, NULL) != 0){
        perror("Failed to create pthread");
    }

    if(pthread_join(th, NULL) != 0){
        perror("Failed to join pthread");
    }
    printf("Amount[%d]\n",resource.amount);
}