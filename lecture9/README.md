# condition variable
한 가지 예시를 들어보자, 주유소의 경우는 연료를 사용하기도 하고, 연료를 소모하기도 한다. 이를 ```thread```로 표현해보도록 하자.

그렇다면 ```thread 1```은 연료를 채우도록 하고, ```thread 2```는 연료를 소모하도록 한다. 이떄 연료인 ```fuel```은 공유 변수이므로 이곳에 접근하는 것은 ```race condition```을 발생시킬 수 있는 ```critical section```이기 때문에 ``mutex```가 필요할 것이다.

이를 구현한 코드가 바로 다음이다.

- gas_station.c
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

typedef struct{
    int fuel;
    pthread_mutex_t mutex;
}GasStation_t;

GasStation_t gGasStation ={
    .fuel = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER
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
}
```

```fuel_filling```은 5번에 걸쳐서 연료를 채워넣고, ```car```는 한 번에 연료를 쭉 소모한다. 이들을 각기 다른 ```thread```에 넣어 ```thread 0```은 ```car``` 함수를 실행하여 연료를 소모하도록 하고, ```thread 1```은 ```fuel_filling```을 호출하도록 하여 연료를 채워놓도록 하였다.

실행하면 다음의 결과를 얻게된다.

```
Got fuel. Now left: -40
Filled fuel ... -25
Filled fuel ... -10
Filled fuel ... 5
Filled fuel ... 20
Filled fuel ... 35
```
연료가 채워넣어졌지만, 연료가 음수가 된다?? 이는 의도하지않은 경우이다. 그래서, ```car```에 ```fuel```이 ```40```보다 작으면 연료를 채우지 않도록 하자.

```c
void* car(void* arg){
    pthread_mutex_lock(&gGasStation.mutex);
    if(gGasStation.fuel >= 40){
        gGasStation.fuel -= 40;
        printf("Got fuel. Now left: %d\n",gGasStation.fuel);
    }
    else{
        printf("No fuel\n");
    }
    pthread_mutex_unlock(&gGasStation.mutex);
}
```
```car```함수에 if문만 추가하여 ```fuel```의 양을 검사한 것이다. 실행해보면

```
No fuel
Filled fuel ... 15
Filled fuel ... 30
Filled fuel ... 45
Filled fuel ... 60
Filled fuel ... 75
```

이번에는 음수가 나오진 않았지만, 연료를 빼가지도 않게되었다. 그렇다면, ```while```을 사용하여 ```fuel```이 ```40```보다 작다면 기다리도록 하고, 크거나 같으면 연료를 채우도록 하면 어떨까?

```c
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
```
```car```를 다음과 같이 ```if```문 대신에 ```while```문으로 조건을 달도록하고, 조건을 만족시킬 떄까지 기다리는 것이다.

실행해보면
```
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
No fuel. Waiting...
...
```
다음의 로그가 계속 반복되고, 연료를 채워지지도 빠지지도 않게 된다.

왜그럴까?? 아주 단순한데, 연료를 빼내는 ```car```함수를 실행하는 ```thread 0```가 ```mutex```를 ```lock```으로 걸고 sleep했기 때문이다. 

이 때문에 ```fuel_filling```함수를 실행하는 ```thread 1```는 ```mutex```가 ```unlock```되기만을 기다린채 block되는데, 반면 ```mutex```를 잡고 있는 ```thread 0```는 ```thread 1```이 연료를 채워주기만을 기다리고 있다. 이렇게 서로가 서로의 자원을 탐내어 영원히 해결되지 않는 문제를 ```데드락(Deadlock)```이라고 한다.

왜 데드락이 발생한 것인가? 그것은 서로가 서로의 자원을 원하지만 자신이 쥐고있는 자원에 대해서는 포기하지 않으려고 하기 때문이다. 

이러한 문제를 해결하기위해 ```condition variable```이 나오게 된다.

```condition variable```은 ```thread```간 communication하기 위한 변수로 특정 condition이 만족되면 이 사실을 다른 ```thread```들에게 알리기 위한 변수이다.

여기서 ```condition```은 위의 예제에서 **```fuel```이 40보다 작을 때** 이다.

# condition variable 사용법
```c
pthread_cond_t condition;
```
다음과 같이 condition variable 변수를 선언할 수 있다. mutex와 같이 초기화와 해제도 필요하다.

```c
pthread_cond_init(&cond, NULL);
pthread_cond_destroy(&cond);
```

```init```함수를 따로 쓰지않고 ```PTHREAD_COND_INITIALIZER```를 변수에 넣어줘서 써도 된다.

