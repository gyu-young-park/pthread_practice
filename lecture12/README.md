# Deadlock and recursive lock

## 1. Deadlock
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

## 2. Recursive lock
```deadlock```은 서로가 서로의 자원을 쥐고, 놓아지주지 않을 때 서로의 자원을 원하면 발생할 수 있다. 그런데, 사실은 '서로'라는 말은 자신도 포함된다. 즉, 자기자신이 자신이 쥐고있는 자원을 갖고있음에도 불구하고, 갖으려고 한다면 ```deadlock```이 발생할 수 있다는 것이다.

아주 간단한 예제가 바로 이것이다.
- recursive_lock.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

typedef struct {
    int amount;
    pthread_mutex_t mutex;
}Resource;

Resource resource = {
    .amount = 10,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

void* routine(void* arg){
    pthread_mutex_lock(&resource.mutex);
    resource.amount -= 2;
    pthread_mutex_unlock(&resource.mutex);
    return 0;
}

int main(int argc, char* argv[]){
    pthread_t th;
    if(pthread_create(&th, NULL, routine, NULL) != 0){
        perror("Failed to create pthread");
    }

    if(pthread_join(th, NULL) != 0){
        perror("Failed to join pthread");
    }
    printf("Amount[%d]\n",resource.amount);
}
```
다음의 간단한 예제를 구동해보면, ```Amount[8]```문제없이 결과가 나온 것을 확인할 수 있다.

그러나, 만약 ```routine```내부에서 또 ```mutex_lock```을 건다면 어떻게될까? 즉, 자기가 가진 자신의 자원에 대한 ```mutex_lock```을 걸려고 한다면, 어떻게 되냐는 것이다.

```c
void* routine(void* arg){
    pthread_mutex_lock(&resource.mutex);
    pthread_mutex_lock(&resource.mutex);
    resource.amount -= 2;
    pthread_mutex_unlock(&resource.mutex);
    return 0;
}
```
바로 이렇게 말이다.

코드를 다음과 같이 변경해놓고, 프로그램을 실행하면 ```deadlock```이 발생하여 프로그램이 동작하지 않는 것을 확인할 수 있다. 왜 ```mutex_lock```을 한 번 더 걸었을 뿐인데 ```deadlock```이 발생한 것일까?? 아주 간단한 이유인데, ```mutex lock```을 가진 ```thread```가 또 ```mutex lock```을 걸려고 하면, ```mutex```는 이미 ```mutex```가 ```lock```인 상태이므로 해당 thread를 block처리해버린다. 즉, ```mutex```는 그 어떤 ```thread```라도 구분없이 동작한다. 때문에 ```mutex lock```을 자신이 걸었더라도, 자신이 또 ```mutex lock```을 걸려고 하면, 반환되지 않는 자원을 요구하므로 ```deadlock```이 발생하는 것이다.

위의 문제처럼 대놓고 ```lock```을 두 번 걸어서 ```deadlock```이 발생하는 경우는 거의없다. 그러나, 문제는 ```recursive```하게 함수를 실행하려고 할 때이다. 

routine 함수를 다음과 같이 변경해보도록 하자.
- recursive_lock.c
```c
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
```

```amount```가 0이거나, 0보다 작을 때까지 ```routine```을 반복한다. 이때 ```recursive```하게 ```routine```을 반복하는데, 이 과정에서 ```deadlock```이 발생하는 것이다. 
- 결과
```
Start routine
resource value:[8]
Start routine
```
이 다음부터는 진행이 안된다. 왜냐하면 다음과 같다.

1. ```thread 1```이 ```routine```을 실행하고, ```mutex lock```을 얻는다.
2. ```thread 1```이 ```amount```에 ```-2```을 해주고, ```resource value:[8]```결과를 출력한다.
3. ```resource.amount > 0```이 만족하므로 , ```routine```이 실행된다.
4. ```thread 1```이 ```recursive```하게 ```routine```을 실행하는데, ```mutex lock```을 얻지 못한다.
5. 이는 이전에 이미 ```mutex lock```을 얻었지만, 이를 ```unlock```하지 않았기 때문에 ```block```된 것이다.
6. 자기 자신에 의해 ```deadlock```이 발생한 것이다.

이렇게 ```recursive```하게 함수가 실행될 때에는 어떻게 ```mutex lock```을 해주어야 할까? ```recursive```하게 함수를 호출하기전에 ```unlock```을 하면되지 않을까? 그러나, 이는 해결방법이 아니다.

```c
void* routine(void* arg){
    printf("Start routine\n");
    pthread_mutex_lock(&resource.mutex);
    resource.amount -= 2;
    sleep(1);
    printf("resource value:[%d]\n",resource.amount);
    pthread_mutex_unlock(&resource.mutex);
    if(resource.amount > 0){
        routine(NULL);
    }
    return 0;
}
```
```pthread_mutex_unlock```이후에 ```if(resource.amount > 0)```을 체크하여 ```routine```을 실행할 지 말지 결정하는 것이다. ```deadlock```은 해결했지만, ```race condition```의 문제가 있다. 즉, ```mutex unlock```이기 때문에, 다른 ```thread 2```가 ```resource.amount```에 접근이 가능하게되고, 원래의 로직과 의도한 것이 달라질 수 있게 될 가능성이 있다. 

그렇기 때문에 ```shared resource```에 대한 ```lock```을 포기할 수는 없다. 한 가지 방법은 ```mutex```도 ```recursive```하게 걸 수 있도록 하는 것이다. 이것이 바로 ```recursive lock```이다.

```recursive lock```은 전혀 새로운 개념이 아니다. ```mutex```에 속성을 부여하는데, 반복해서 ```mutex lock```이 가능하다. 단 **lock을 걸어준 횟수만큼 unlock**을 해주어야 한다. 즉, 5번 ```mutex lock```을 했다면 5번 ```mutex unlock```을 해주어야 한다.

```recursive lock```은 ```mutex lock```의 ```attributes```를 설정해주면 된다.

- mutex attribute
```c
pthread_mutexattr_t recursiveAttr;
pthread_mutexattr_init(&recursiveAttr);
pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&resource.mutex,&recursiveAttr);
```
이런 식으로 설정하면된다. 

1. ```pthread_mutexattr_t``` type를 가지는 ```attribute``` 인스턴스를 선언한다.
2. ```pthread_mutexattr_init```으로 초기화를 해준다.
3. ```pthread_mutexattr_settype```을 통해서 ```attribute```인스턴스에 속성 타입을 부여할 수 있는데, ```recursive```는 ```PTHREAD_MUTEX_RECURSIVE```이다.
4. 이제 해당 ```attribute```를 ```pthread_mutex_init```를 통해서 ```mutex```에 넣어주면 된다.

이를 사용하여 ```recursive lock```문제를 해결해보도록 하자.

- recursive_lock.c
```c
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
```
```mutex```에 ```recursive lock``` 속성을 설정해주고, 실행해보면 다음의 결과를 얻을 수 있다.

```
Start routine
resource value:[8]
Start routine
resource value:[6]
Start routine
resource value:[4]
Start routine
resource value:[2]
Start routine
resource value:[0]
Amount[0]
```
정상적으로 종료된 것을 확인할 수 있다. 자기 자신이 가진 ```mutex```에 대해서 또 ```lock```을 걸어도 ```deadlock```에 안걸리고, 또 ```lock```을 걸 수 있게 된 것이다. 단, ```unlock```역시도 마찬가지로 ```lock```한 횟수만큼 동일하게 해주어야 한다는 사실을 잊지말자. 

```recursive lock```은 위와 같이 ```recursive```하게 ```thread```가 자신의 함수를 호출할 때 필요하여 ```recursive lock```이라는 이름이 붙은 것이다.