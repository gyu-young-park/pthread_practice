#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define PRIME_ARR_SIZE 10

int prime[PRIME_ARR_SIZE] = { 2, 3, 5, 7, 11, 13 ,17, 19 ,23, 29};
int res = 0;

void* calc(void *arg){
    int* pos = (int*)arg;
    int sum = 0;
    for(int i =0; i < PRIME_ARR_SIZE/2; i++, pos++){
        sum += *pos;
    }
    printf("sum: %d\n", sum);
    res += sum;
}

int main(int argc, char* argv[]){
    clock_t begin = clock();
    pthread_t th[2];
    for(int i = 0; i < 2; i++){
        int start = (PRIME_ARR_SIZE / 2) * i;
        if(pthread_create((th + i), NULL, calc, (void *)(prime + start)) != 0){
            perror("Failed create thread!\n");
        }
    }
    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i],NULL) != 0){
            perror("Failed to join thread\n");
        }
    }
    printf("result: %d\n", res);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spend: %lf\n", time_spent);
}