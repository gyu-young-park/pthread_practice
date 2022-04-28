#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

void* roll_dice(){
    int value = (rand() % 6 + 1);
    int *result = malloc(sizeof(int));
    *result = value;
    printf("%d\n", value);
    return (void*)result;
}

int main(int argc, char* argv[]){

    int* ret;
    srand(time(NULL));
    pthread_t th;
    if(pthread_create(&th, NULL, &roll_dice, NULL) != 0){
        return 1;
    }
    if(pthread_join(th, (void**)&ret) != 0){
        return 2;
    }
    printf("result is : %d\n",*ret);
    free(ret);
    return 0;
}