## 1. 동적 할당을 사용하지 않고, 해결하기
이전 **lecture6**에서 **pthread_create**의 함수에 파라미터를 넘기는 방법에 대해서 알아보았다. 함수에 들어가는 파라미터가 ```void*```타입이기 때문에 포인터가 넘어가 **for**반복문에서 넘어간 인텍스가 변하는 문제가 생겼다. 즉, 하나의 주소에 값이 계속 바뀌는 상황에서, 내가 받기를 기대한 값이 원하는 타이밍에 실행되지 않아 다른 값으로 변하는 문제가 생겼었다. 

이를 해결하는 방법으로는 **동적 할당**을 통해서 값도 다르고, 주소도 다른 변수를 파라미터로 넘겨주는 것이었다. 그러나, 이는 동적할당된 포인터를 생성하고 해제하는 코드가 분리되어 코드의 복잡성을 야기했다. 

그렇다면 어떻게 해야할까?? 가장 좋은 방법은 **배열의 주소**를 넘겨주는 방법이다. 즉, 배열의 인덱스를 넘겨주는 것이 아니고, 또한 동적할당하여 새로운 메모리 주소에 배열의 인덱스 값을 할당해서 파라미터로 넘겨주는 것이 아니라, 배열에서 해당 값이 저장된 주소값을 넘겨주는 것이다.

```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int prime[10] = {2, 3, 5, 7, 11, 13, 17, 19,23, 29};

void* routine(void* arg){
    int* index = (int *)arg;
    printf("%d ",*index);
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 10; i++){
        if(pthread_create(&th[i], NULL, routine, (prime + i)) != 0){
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
```
다음의 코드를 보면, ```pthread_create```의 파라미터로 배열의 포인터인 ```(prime + i)```룰 넘겨준다. 이렇게 배열의 포인터를 넘겨주면 주소값(포인터)가 달라 문제가 안된다. 또한, 동적할당을 사용할 필요없어 코드의 복잡성 또한 증가하지 않는다.

결과는 다음과 같다.

```
2 3 5 7 11 13 17 19 23 29
```

결과과 잘 나왔음을 확인할 수 있다.

## 2. 응용 thread 두 개를 통해 배열의 합을 구하기
**thread**를 사용하는 가장 실용적인 방법은 **thread**가 일부를 계산해서 이 들의 결과를 돌려주어 합쳐내는 방법이다. 이를 구현해보도록 하자.

구현하는 방법은 매우 간단하다. 배열 사이즈가 10인 ```prime```배열이 있다면, 0~4는 thread1이 계산하고, 5~9까지는 thread2가 계산한 다음, 이들을 전역 변수 ```res```에 더하는 것이다. 각 thread들에게 주어는 파라미터는 순회의 시작 주소로, 시작 주소로 부터 5칸 씩 반복하여 덧셈 연산을 진행한다.

```c
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
```
덧셈 연산이 진행될 배열은 ```prime```이다. ```calc```함수에서는 매개변수로 ```int* pos = (int*)arg;```를 받는데, 이는 배열 ```prime```의 주소로, 해당 주소로 부터 5칸 앞으로 나아가는 것을 반복한다. 그리고 반복문이 진행될 때 ```sum```을 계산하고, 이를 전역변수 ```res```에 더한다.

```c
pthread_t th[2];
for(int i = 0; i < 2; i++){
    int start = (PRIME_ARR_SIZE / 2) * i;
    if(pthread_create((th + i), NULL, calc, (void *)(prime + start)) != 0){
        perror("Failed create thread!\n");
    }
}
```

**main.c** 함수를 보면, 다음과 같다. ```start```변수는 ```prime```의 주소에 더해져 ```pthread_create``` 함수의 파라미터로 들어간다. 이 주소를 기준으로 5칸 앞으로 반복하여 덧셈을 진행하는 것이다.

전체 코드를 확인하면 다음과 같다.

```c
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
```

결과는 다음과 같다.

```
um: 28
sum: 101
result: 129
time spend: 0.000226
```

이 처럼, thread를 이용하여 효율적인 덧셈을 진행할 수 있다. 그러나 이런 thread를 활용한 덧셈이 마냥 효율적인 것은 아니다. 그냥 thread를 사용하지 않고 **main 함수**에서만 덧셈을 진행하면 

```
time spend: 0.000038
```
평균 0.000030~0.000080```을 웃돈다. 

그러나, 위의 thread를 활용한 덧셈은 

```
time spend: 0.000226
```
이 나온다. 

**thread** 자체를 사용하는 것은 매우 큰 비용을 소모한다. 그렇기 때문에 간단한 덧셈 작업에 대해서는 **thread**를 사용하는 것이 비효율적일 수 있다. 그렇기 때문에 무조건 적으로 **thread**를 사용하는 것은 비효율적일 수 있으니 알아두록 하자.