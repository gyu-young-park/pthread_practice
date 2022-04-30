#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define PRIME_ARR_SIZE 10

int prime[PRIME_ARR_SIZE] = { 2, 3, 5, 7, 11, 13 ,17, 19 ,23, 29};

int main(int argc, char* argv[]){
    clock_t begin = clock();
    int res = 0;
    for(int i = 0; i < PRIME_ARR_SIZE; i++){
        res += prime[i];
    }
    printf("result: %d\n", res);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spend: %lf\n", time_spent);
}