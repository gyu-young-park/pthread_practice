# Difference between trylock and lock in c
```trylock```과 ```lock```은 둘  다 ```mutual exclusion```을 보장한다는 것이다. 즉 shared data에 대한 배타적 접근을 지원한다는 것이다.

그러나 이 둘은 이미 잠겨져있는 상태에서 차이가 있는데, ```thread 1 ,2```가 있을 때 shared data A를 ```thread 1```이 쥐고 있다고 하자. 그렇다면, ```thread 1```이 ```lock```을 잡고 있는 것이된다. 이때 ```thread 2```가 shared data A에 접근하려고 하면, ```lock```에 의해서 ```block```되고, ```thread 1```이 작업을 완료한 후 ```unlock```을 실행하면 ```thread 2```가 깨어나고 다시 ```lock```을 걸도록 한다.

여기까지가 일반적은 ```lock```의 매커니즘이다. 그러나 ```trylock```을 사용하면 다르다. ```trylock```은 ```thread 1```이 ```lock```을 걸었을 때, ```thread 2```가 접근하려고 하면, ```thread 2```를 ```block```시키는 것이 아니라, 쿨하게 다음 코드를 실행하도록 한다. 즉, 기다리지않고, 다음 코드를 실행하는 것이다. 이처럼 ```trylock```은 **lock을 얻으려고 시도**하고, lock을 얻었다면 0을 반환하고 critical section의 코드를 실행한다. 반면 lock을 얻지 못했다면 critical section의 코드를 실행하지 않고, lock을 얻을 자신의 차례가 올 때까지 기다리지도 않고 다음 코드를 실행한다.

- lock.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t mutex;

void* routine(void* arg){
    pthread_mutex_lock(&mutex);
    printf("Got lock\n");
    sleep(1);
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char* argv[]){
    pthread_t th[4];
    pthread_mutex_init(&mutex, NULL);
    for(int i = 0; i < 4; i++){
        if(pthread_create(&th[i], NULL, &routine, NULL) != 0){
            perror("Error at creating thread");
        }
    }
    for(int i = 0; i < 4; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Error at joining thread");
        }
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}
```
다음의 코드는 일반적인 ```pthread_mutex_lock```코드이다. 총 4개의 thread는 ```routine```을 실행하는데, lock을 가지면 critical section의 코드 ```printf("Got lock\n)```을 실행하고, 아니면 ```lock```을 얻을 자신의 차례가 올 떄까지 block 상태로 기다린다. 

그래서, 다음의 코드를 실행하면

```
Got lock
Got lock
Got lock
Got lock
```

다음의 결과를 얻는데, sleep(1)을 했기 떄문에 1초마다 하나씩 로그가 찍히는 것을 확인할 수 있다. 즉, 일반적인 ```lock```은 ```lock```을 가지려고 했다가 못가지면 block되고 unlock되면 lock을 가지려 시도했던 thread을 깨워줘 lock을 할 수 있도록 해준다.

이제 ```pthread_mutex_trylock```을 사용해보자
```c
pthread_mutex_trylock(&mutex) // success 0 , otherwise failure
```
입력으로는 mutex의 포인터만 받고, 리턴으로 성공 시에는 0 실패 시에는 이외의 값들을 리턴한다. 성공하면 critical section에 있는 코드를 실행도록하고, 실패 시에는 다음 코드로 넘어가는 것을 명시하자. 참고로, 실패시에는 lock을 얻은 것이 아니므로 ```unlock```을 안해줘도 된다.

- trylock.c
```c
void* routine(void* arg){
    if(pthread_mutex_trylock(&mutex) == 0){
        printf("Got lock\n");
        sleep(1);
        pthread_mutex_unlock(&mutex);   
    }else{
        printf("failure get lock\n");
    }
}
```

```routine```부분만 위 처럼 변경해주면 된다. 위 코드를 실행해보면 다음의 결과가 나온다.

```
Got lock
failure get lock
failure get lock
failure get lock
```

한 개의 thread가 trylock을 통해 lock을 얻었지만, 1초간 sleep하게된다. 이떄 lock을 잠그고 block되었기 떄문에 나머지 thread들은 lock을 얻으려 try했다가 실패하여 ```failure get lock``` 로그를 출력하고 thread 실행을 종료한다.

## 1. trylock practical example
왜 ```trylock```을 사용할까? ```trylock```은 굉장히 유용하다. 한 가지 예시를 들어 확인해보자.

10명의 요리사들이 stove를 사용하여 음식을 만든다고 하자. 이떄 stove에는 연료가 있는데 연료는 요리사들끼리 공유하며 사용할 수 있다. 그러나 stove는 하나가 아니라 4개이다. 즉, 요리사는 하나의 thread가 되는 것이고, stove 연료는 하나의 shared data가 되는 것이다. 단, stove가 4개이므로 shared data가 되는 stove 연료도 4개가 된다. 

- trylock2.c
```c
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

typedef struct {
    int32_t id;
    int32_t fuel;
    pthread_mutex_t mutex;
} Stove_t;

Stove_t stoves[4];

void* useStove(void* arg){
    int32_t isUsed = 0;
    for(int i = 0; i < 4; i++){
        pthread_mutex_lock(&stoves[i].mutex);
        int32_t usedFuel = rand() % 30;
        if(stoves[i].fuel - usedFuel >= 0){
            stoves[i].fuel -= usedFuel;
            isUsed = 1;
            printf("Stove ID[%d]: remain fuel[%d]\n",stoves[i].id, stoves[i].fuel);
            usleep(500000);
        }else{
            printf("Stove ID[%d]: can't be used\n",stoves[i].id);
        }
        pthread_mutex_unlock(&stoves[i].mutex);
        if(isUsed != 0) break;
    }
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 4; i++){
        stoves[i].id = i;
        stoves[i].fuel = 100;
        pthread_mutex_init(&stoves[i].mutex, NULL);
    }

    for(int i = 0; i < 10; i++){
        if(pthread_create(&th[i], NULL, useStove, NULL) != 0){
            perror("Failed to create thread\n");
        }
    }

    for(int i = 0; i < 10; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread\n");
        }
    }

    for(int i = 0; i < 4; i++){
        pthread_mutex_destroy(&stoves[i].mutex);
    }
}
```
다음과 같이 코드를 만들 수 있을 것이다. 결과를 확인해보자.

```
Stove ID[0]: remain fuel[93]
Stove ID[0]: remain fuel[74]
Stove ID[0]: remain fuel[51]
Stove ID[0]: remain fuel[43]
Stove ID[0]: remain fuel[33]
Stove ID[0]: remain fuel[31]
Stove ID[0]: remain fuel[7]
Stove ID[0]: can't be used
Stove ID[1]: remain fuel[77]
Stove ID[0]: can't be used
Stove ID[1]: remain fuel[77]
Stove ID[0]: remain fuel[2]
```
모든 요리사(thread)들이 stove를 사용하여 잘 해결된 것을 확인할 수 있다. 그러나, 뭔가가 이상하다. 잘보면, stove 0번을 번갈아 쓰고, stove 0번의 연료가 고갈나므로 stove 1번으로 변경한 것을 확인할 수 있다. 즉, 이들은 현재 stove가 4대가 있는데도 불구하고 한 대씩 서로 나누어 쓰고 있는 것을 확인할 수 있다. 이는 매우 비효율적이며 다른 ```stove```를 사용하지 못한다는 단점이 있다. 이는 ```pthread_mutex_lock```이 오직 하나의 자원에 대해서만 ```lock```을 걸도록하고 실패하면 ```block```되기 떄문이다.

이를 해결하려면 어떻게해야할까?? 누군가 ```stove 0번```을 사용하고 있다면 ```stove 1번```을 사용하면 된다. ```stove 0번, 1번```이 모두 ```lock```되었다면 ```stove 2번```으로 넘어가면 된다. 이렇게 누군가 이미 자원(```stove```)를 선점하고 있다면 다른 ```stove```를 사용하면 된다는 것이다. 이때 ``trylock```을 사용하면 된다. ```trylock```은 일반적인 ```lock```과는 달리 어떤 자원만을 ```lock```을 걸어야할 지 꼭 집어서 ```lock```을 거는 것이 아니다. ```lock```의 경우는 pre-defined된 자원에 대해서만 ```lock```을 한다. 그리고 이미 ```lock```이 걸려있다면 오매불망 그 자원의 ```unlock```만을 기다리는 것이다. 반면 ```trylock```은 조금 다르다. ```trylock```은 pre-defined된 자원에 대해서만 ```lock```을 걸려고 기다리는 것이 아니라, 여러 자원에 대해서 ```lock```을 시도하고 이것 중 하나라도 되면 해당 자원을 사용하는 개념이다. 그래서 **여러개의 shared data 중에서 어떤 것이든 써도되는 상황일 떄 trylock을 자주 사용한다.** 

```useStove```만 다음의 코드로 변경해보도록 하자.
- trylock2.c
```c
void* useStove(void* arg){
    int32_t isUsed = 0;
    for(int i = 0; i < 4; i++){
        if(pthread_mutex_trylock(&stoves[i].mutex) == 0){
            int32_t usedFuel = rand() % 30;
            if(stoves[i].fuel - usedFuel >= 0){
                stoves[i].fuel -= usedFuel;
                isUsed = 1;
                printf("Stove ID[%d]: remain fuel[%d]\n",stoves[i].id, stoves[i].fuel);
                usleep(500000);
            }else{
                printf("Stove ID[%d]: can't be used\n",stoves[i].id);
            }
            pthread_mutex_unlock(&stoves[i].mutex);
        }else{
            printf("Stove ID[%d]: find next stove\n", stoves[i].id);
        }
        if(isUsed != 0) break;
    }
    if(isUsed == 0){
        printf("Can't use any stove\n");
    }
}
```
이번에는 ```trylock```을 사용하여 문제를 해결하도록 하였다. 다음의 결과를 확인해보도록 하자.

```
Stove ID[0]: remain fuel[93]
Stove ID[0]: find next stove
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: remain fuel[77]
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: find next stove
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: find next stove
Stove ID[3]: find next stove
Can't use any stove
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: find next stove
Stove ID[3]: find next stove
Can't use any stove
Stove ID[2]: find next stove
Stove ID[3]: find next stove
Can't use any stove
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: find next stove
Stove ID[3]: find next stove
Can't use any stove
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: find next stove
Stove ID[3]: find next stove
Can't use any stove
Stove ID[3]: remain fuel[92]
Stove ID[0]: find next stove
Stove ID[1]: find next stove
Stove ID[2]: find next stove
Stove ID[3]: find next stove
Can't use any stove
Stove ID[1]: remain fuel[81]
```
이전에는 ```stove 0, 1```만 순서대로 사용했다면, 이번에는 순서에 상관없이 ```stove 0, 1, 2, 3```모두 사용하고 있다. 굉장히 효율적이다. 그러나, 여기서 문제가 하나있는데 바로 아직 ```stove```를 사용하지 못한 ```요리사(thread)```들이 존재한다는 것이다. ```Can't use any stove``` 로그가 찍힌 ```thread```들이 바로 그 희생자이다. 왜 이런 일이 발생하는 것인가?? 이것은 ```for```문을 순회하여 ```trylock```을 사용할 떄 모든 자원(stove)들이 이미 선점 중이라 어느 것도 사용하지 못해서 그렇다. 그렇다면, 어느것도 사용할게 없다면, 다시 처음부터 하나씩 확인해보도록 하는 방법이 있다. ```while```을 추가하여, ``trylock```을 통해 한 번이라도 ```stove```를 사용했는 지 체크해주도록 하자.

- trylock2.c
```c
void* useStove(void* arg){
    int32_t isUsed = 0;
    while(isUsed == 0)
    {
        for(int i = 0; i < 4; i++){
            if(pthread_mutex_trylock(&stoves[i].mutex) == 0){
                int32_t usedFuel = rand() % 30;
                if(stoves[i].fuel - usedFuel >= 0){
                    stoves[i].fuel -= usedFuel;
                    isUsed = 1;
                    printf("Stove ID[%d]: remain fuel[%d]\n",stoves[i].id, stoves[i].fuel);
                    usleep(500000);
                }else{
                    printf("Stove ID[%d]: can't be used\n",stoves[i].id);
                }
                pthread_mutex_unlock(&stoves[i].mutex);
            }
            if(isUsed != 0) break;
        }
        if(isUsed == 0){
            printf("Can't use any stove sleep[0.5]\n");
            usleep(500000);
        }
    }
}
```
```while```문을 추가하여 busy waiting하도록 만들었고, 일부 로그는 너무 많이 찍혀서 제거했다. 이제 잘 해결되었는 지 확인해보도록 하자.

```
Stove ID[0]: remain fuel[93]
Stove ID[1]: remain fuel[81]
Can't use any stove sleep[0.5]
Can't use any stove sleep[0.5]
Stove ID[3]: remain fuel[92]
Stove ID[2]: remain fuel[77]
Can't use any stove sleep[0.5]
Can't use any stove sleep[0.5]
Can't use any stove sleep[0.5]
Can't use any stove sleep[0.5]
Stove ID[0]: remain fuel[83]
Stove ID[1]: remain fuel[79]
Stove ID[2]: remain fuel[53]
Stove ID[3]: remain fuel[84]
Can't use any stove sleep[0.5]
Can't use any stove sleep[0.5]
Stove ID[0]: remain fuel[60]
Stove ID[1]: remain fuel[60]
```
모든 ```stove```들이 효율적으로 사용되었고 ```stove```를 사용하지 못하는 ```요리사(thread)```들도 없게 되었다.

```trylock```은 이렇게 여러개의 ```shared data```에 대해서 접근할 때 어떤 것에 접근하든 지 상관없이 접근하여 로직을 처리해주어도 가능할 떄 사용할 수 있다.

