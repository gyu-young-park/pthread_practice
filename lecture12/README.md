# Deadlock and recursive lock
```deadlock```은 무엇인가? ```deadlock```은 각 thread들이 서로의 자원을 원하고 있는데, 서로가 자원을 얻지못하여 영원히 block되는 상태를 말한다. 가령, 주유소에 ```fuel```과 ```water```이 있다고 하자. 이 때, ```fuel```을 ```mutex```인 ```fuelMutex```가 있고, ```water```의 ```mutex```인 ```waterMutex```가 있다.

- deadlock.c
```c
typedef struct {
    int amount;
    pthread_mutex_t mutex;
}Resource;

typedef struct{
    Resource fuel;
    Resource water;
} GasStation;
```
이를 구현한게 바로 위이다. ```Resource```는 자원을 의미하며, 자원의 양인 ```amount```와 ```mutex```로 구성되어있다. 두 자원인 ```fuel, water```는 ```GasStation```의 자원으로 ```fuel, water```로 정의되어있다. 

이제, 자동차 역할인 ```thread 1```은 ```fuel```을 먼저 채우고, ```water```을 얻으려고 한다. ```thread 2```는 ```water```을 먼저 채우고, ```fuel```을 얻으려고 한다. 이를 코드로 구현하다면 다음과 같다.

- deadlock.c
```c
void* car1(void* arg){
    pthread_mutex_lock(&gasStation.fuel.mutex);
    pthread_mutex_lock(&gasStation.water.mutex);
    gasStation.fuel.amount -= 10;
    gasStation.water.amount -= 2;
    pthread_mutex_unlock(&gasStation.water.mutex);
    pthread_mutex_unlock(&gasStation.fuel.mutex);
}

void* car2(void* arg){
    pthread_mutex_lock(&gasStation.water.mutex);
    pthread_mutex_lock(&gasStation.fuel.mutex);
    gasStation.water.amount -= 2;
    gasStation.fuel.amount -= 10;
    pthread_mutex_unlock(&gasStation.fuel.mutex);
    pthread_mutex_unlock(&gasStation.water.mutex);
}
```
이를 코드로 구현하면, ```car1```은 ```fuel```의 ```mutex lock```을 걸고, ```water```의 ```mutex lock```을 건다. 반대로, ```car2```는 ```water```의 ```mutex lock```을 걸고 ```fue```의 ```mutex lock```을 건다. 이렇게 되었을 때 발생할 수 있는 문제점이 있다.

1. thread1이 ```fuel```의 ```mutex```를 얻고, 실행을 잠시 중지 thread2가 실행됨
2. thread2는 ```water```의 ```mutex```를 얻고, 실행을 잠시 중지 thread1이 실행됨
3. thread1는 ```water```의 ```mutex```를 얻으려고 하지만, 실패 block처리가 된다. 이후 thread2가 실행
4. thread2는 ```fuel```의 ```mutex```를 얻으려고 하지만, 실패 block처리가 된다.
5. thread1, thread2는 각각 ```fuel mutex```, ```water mutex```를 들고 있지만, 서로의 자원을 놓지 않은 채 서로의 자원을 원한다. 이러한 문제를 ```deadlock```이라고 한다.

그러나, 위와 같이 코드를 만들었다해서 ```deadlock```이 반드시 발생하는 것은 아니다. 그래서 강제로 발생시키기 위해 ```mutex lock```을 얻는 중간에 ```sleep(1)```를 걸어보도록 하자.

다음의 코드는 완벽히 ```deadlock```을 재현한다.

- deadlock.c
```c
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
    pthread_mutex_lock(&gasStation.water.mutex);
    sleep(1);
    pthread_mutex_lock(&gasStation.fuel.mutex);
    gasStation.water.amount -= 2;
    gasStation.fuel.amount -= 10;
    pthread_mutex_unlock(&gasStation.fuel.mutex);
    pthread_mutex_unlock(&gasStation.water.mutex);
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
```
```deadlock```이 발생하였기 때문에 강제로 종료하는 방법밖에 없다.

그렇다면 어떻게 ```deadlock```을 해결하도록 해야할까?? 가장 좋은 방법은 ```lock```을 얻을 때에 순서를 강제하도록 하는 방법이다. 즉, ```water```을 얻으려면 ```fuel```의 ```mutex lock```을 먼저 얻도록 하는 방법이다. 현대 운영체제는 ```deadlock```이 발생하면 이를 해결하려고 하지않고, 그냥 받으들인 다음 실행을 강제종료하도록 한다. 왜냐하면 ```deadlock```이 발생하는 것을 예측하고, 이를 회피하는 것은 많은 자원을 소모하기 때문이다. 

그래서 다음과 같이 ```lock```을 얻는 순서만 바꿔줘도 쉽게 해결가능하다.

```c
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
```
문제없이 다음과 같은 결과가 나오는 것을 확인할 수 있다.

```
Remaining Fuel:[30]
Remaining water:[6]
```
원하는 결과를 확인할 수 있다.