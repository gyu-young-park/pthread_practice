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

condition variable은 3가지의 동작을 가지고 있다.

```c
int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *)
int pthread_cond_broadcast(pthread_cond_t *)
int pthread_cond_signal(pthread_cond_t *);
```
```pthread_cond_wait```을 통해 ```thread```를 block 상태로 만들 수 있다. 중요한 것은 두번쨰 입력 파라미터가 ```mutex```라는 것이다. ```mutex```를 넘겨주는 이유는 ```pthread_cond_wait```이 실행되면 ```mutex```를 ```unlock```하기 때문이다.

그러나, block되었기 떄문에 누군가가 꺠워줘야 한다. 이것이 바로 ```pthread_cond_signal```이다. 단 ```condition variable```로 깨워줄 떄는 ```mutex```를 ```unlock```시키고 해야한다. 

깨어난 ```thread```는 ```mutex```를 ```lock```을 하고 일어나기 때문이다. 만약 ```unlock```이 안되었다면 실행이 안될 것이다.

```pthread_cond_broadcast```은 block된 모든 ```thread```들을 깨우는 것이다. 즉, ```condition variable```에 의해 ```wait```상태로 간 ```thread```들은 ```queue```에 들어가고 이 ```queue```에서 하나씩 ```pop```을 하는 것이 ```pthread_cond_signal```이고, 전부 실행하는 것이 ```pthread_cond_broadcast```이다. 

어떻게보면 ```condition variable```은 하나의 ```queue```인 것이다.

```condition variable```을 이용하여 위의 문제를 해결해보자.

- gat_station.c
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

typedef struct{
    int fuel;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}GasStation_t;

GasStation_t gGasStation ={
    .fuel = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
};

void* fuel_filling(void* arg){
    for(int i = 0; i < 5; i++){
        pthread_mutex_lock(&gGasStation.mutex);
        gGasStation.fuel += 15;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        pthread_cond_signal(&gGasStation.cond);
        sleep(1);
    }
}

void* car(void* arg){
    pthread_mutex_lock(&gGasStation.mutex);
    while(gGasStation.fuel < 40){
        printf("No fuel. Waiting...\n");
        pthread_cond_wait(&gGasStation.cond, &gGasStation.mutex);
    }
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
    pthread_mutex_destroy(&gGasStation.mutex);
    pthread_cond_destroy(&gGasStation.cond);
}
```

위의 코드를 실행하면 다음과 같다.
```
No fuel. Waiting...
Filled fuel ... 15
No fuel. Waiting...
Filled fuel ... 30
No fuel. Waiting...
Filled fuel ... 45
Got fuel. Now left: 5
Filled fuel ... 20
Filled fuel ... 35
```
fuel이 40보다 작다면 기다고 있다가 40보다 커지는 45가 되는 순간 ```car```의 연산이 수행된 것을 확인할 수 있다.

하나하나, 추가한 부분을 살펴보자.
```GasStation_t```에 ```pthread_cond_t cond;```가 추가되었다.

```c
void* car(void* arg){
    pthread_mutex_lock(&gGasStation.mutex);
    while(gGasStation.fuel < 40){
        printf("No fuel. Waiting...\n");
        pthread_cond_wait(&gGasStation.cond, &gGasStation.mutex);
    }
    gGasStation.fuel -= 40;
    printf("Got fuel. Now left: %d\n",gGasStation.fuel);
    pthread_mutex_unlock(&gGasStation.mutex);
}
```
```car```에서는 ```while```을 통해서 연료량이 감소되는 연료량보다 적다면 ```pthread_cond_wait```하도록 하였다. 여기서 중요한 것은 반드시 ```while```안에 써야한다는 것이다. 만약 ```while```이 없었다면 제대로 동작하지 않았을 것이다.

```c
void* fuel_filling(void* arg){
    for(int i = 0; i < 5; i++){
        pthread_mutex_lock(&gGasStation.mutex);
        gGasStation.fuel += 15;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        pthread_cond_signal(&gGasStation.cond);
        sleep(1);
    }
}
```
```fuel_filling```에서는 ```pthread_cond_signal```로 block 상태에 있는 ```thread```를 깨운다. 

이렇게 ```condition variable```을 통해 ```thread```간의 일정 조건이 만족하면 신호를 주어 로직을 실행할 수 있도록 communication하는 것이다. 이것으로 deadlock의 문제를 해결한 것이다.

앞서 말했듯이 ```pthread_cond_wait```가 사용된 코드 주변을 잘보면 ```mutex```를 ``unlock```도, ```lock```도 하지 않는다. 이는 ```pthread_cond_wait``` 내부적으로 ```unlock```과 ```lock```을 수행하기 때문이다.

```c/
pthread_cond_wait(&cond, &mutex);
/*
1. pthread_mutex_unlock(&mutex);
2. wait for signal on condition variable
3. pthread_mutex_lock(&mutex);
*/
```
이렇게 내부적으로 동작한다고 생각하면 된다.

정리하자면, ```condition variable```은 ```queue```이자 어떤 ```조건```의 ```identifier```이다. 이 ```조건```이 만족하면 ```signal```을 보내고, 만족하지 않으면 ```wait```하는 것이다.

# pthread_cond_broadcast
좀 더 예제를 심화시켜서 현실 세계에 가깝게 해보자. 주유소에 하나의 차만 올리가 없다. 여러 차들이 와서 연료를 가져가려 한다면 어떻게될까??

- real_gas_station.c
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int thread_number[5] = {0,1,2,3,4};

typedef struct{
    int fuel;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}GasStation_t;

GasStation_t gGasStation ={
    .fuel = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
};

void* fuel_filling(void* arg){
    for(int i = 0; i < 5; i++){
        pthread_mutex_lock(&gGasStation.mutex);
        gGasStation.fuel += 15;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        pthread_cond_signal(&gGasStation.cond);
        sleep(1);
    }
}

void* car(void* arg){
    int id = *(int*)arg;
    pthread_mutex_lock(&gGasStation.mutex);
    while(gGasStation.fuel < 40){
        printf("ID:%d, No fuel. Waiting...\n", id);
        pthread_cond_wait(&gGasStation.cond, &gGasStation.mutex);
    }
    gGasStation.fuel -= 40;
    printf("ID: %d, Got fuel. Now left: %d\n",id,gGasStation.fuel);
    pthread_mutex_unlock(&gGasStation.mutex);
}

int main(int argc, char* argv[]){
    pthread_t th[5];
    for(int i = 0; i < 5; i++){
        if(i == 4){
            if(pthread_create(&th[i], NULL, &fuel_filling, NULL) != 0){
                perror("Failed to create thread");
            }
        }
        else{
            if(pthread_create(&th[i], NULL, &car, (void*)(thread_number + i)) != 0){
                perror("Failed to create thread");
            }
        }
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    pthread_mutex_destroy(&gGasStation.mutex);
    pthread_cond_destroy(&gGasStation.cond);
}
```
다음의 예제는 ``thread``` 수를 총 5가지까지 늘리고 마지막 ```thread```를 제외한 4개는 ```car```를 실행하고, 마지막 ```thread```만 ```fuel_filling```을 실행한다.

과연 어떻게 실행되는 지 확인해보자.

```c
ID:0, No fuel. Waiting...
Filled fuel ... 15
ID:2, No fuel. Waiting...
ID:3, No fuel. Waiting...
ID:0, No fuel. Waiting...
ID:1, No fuel. Waiting...
Filled fuel ... 30
ID:2, No fuel. Waiting...
Filled fuel ... 45
ID: 3, Got fuel. Now left: 5
Filled fuel ... 20
ID:0, No fuel. Waiting...
Filled fuel ... 35
ID:1, No fuel. Waiting...
(무한 기다림...)
```
프로세스가 종료되지 않고 무한 대기하게 된다. 이는 ```thread0,1,2,3```은 ```car```을 실행하는데, 연료를 얻기만(-40) 기다린다. ```signal```을 보내주는 ```fuel_filling```을 실행하는 ```thread 4```는 ```signal```을 보내주긴 하지만, 채워지는 양이 충분하지 않아 일부 ```thread```들이 계속 ```block```된 상태에서 기다리게 된다.

그래서 ```fuel_filling```에서 ```+15```가 아니라 ```+60```을 하도록하여 모두가 ```fuel```을 얻고 종료할 수 있도록 하자. 이는 ```condition variable```을 사용할 떄 조심해야하는 부분으로, 조건이 만족하지 않으면 block되는 ```thread```들이 너무 많이 생성되는 문제가 발생하지 않도록 조심해야 한다.

```c
void* fuel_filling(void* arg){
    for(int i = 0; i < 5; i++){
        pthread_mutex_lock(&gGasStation.mutex);
        gGasStation.fuel += 60;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        pthread_cond_signal(&gGasStation.cond);
        sleep(1);
    }
}
```

결과는 다음과 같다.
```
ID:0, No fuel. Waiting...
ID:3, No fuel. Waiting...
Filled fuel ... 60
ID: 2, Got fuel. Now left: 20
ID:1, No fuel. Waiting...
ID:0, No fuel. Waiting...
Filled fuel ... 80
ID: 3, Got fuel. Now left: 40
Filled fuel ... 100
ID: 1, Got fuel. Now left: 60
Filled fuel ... 120
ID: 0, Got fuel. Now left: 80
```
잘 해결된 것을 확인할 수 있다. 그런데, 이상한 것을 확인할 수 있는데, 바로 ```fuel```이 80일 때이다.

```
Filled fuel ... 80
ID: 3, Got fuel. Now left: 40
Filled fuel ... 100
```
해당 부분을 잘보면 이상한 것이 있다. 바로 ```fuel```은 80이고, 아직 ```thread 0, 1,3```이 연료를 받지 못한 상황인데, 기껏 연료를 받은 것은 ```thread 3```번 뿐이다.```fuel```이 ```80```이기 때문에 최대 두개의 ```thread```의 연료를 채워줄 수 있는데, 한 개 밖에 안채운 것이다. 

**왜냐하면 ```signal```은 오직 ```queue```에 있는 하나의 ```thread```만을 ```awaken```시키기 때문이다.**

이럴 떄 바로 ```pthread_cond_broadcast```를 사용하는 것이다. 모든 ```thread```를 깨워서 ```fuel```을 얻도록 하는 것이다.

```c
void* fuel_filling(void* arg){
    for(int i = 0; i < 5; i++){
        pthread_mutex_lock(&gGasStation.mutex);
        gGasStation.fuel += 60;
        printf("Filled fuel ... %d\n", gGasStation.fuel);
        pthread_mutex_unlock(&gGasStation.mutex);
        pthread_cond_broadcast(&gGasStation.cond);
        sleep(1);
    }
}
```
```pthread_cond_signal```을 ```pthread_cond_broadcast```으로 바꾼 것 밖에 없다.

결과는 다음과 같다.
```
ID:0, No fuel. Waiting...
ID:3, No fuel. Waiting...
ID:1, No fuel. Waiting...
Filled fuel ... 60
ID: 2, Got fuel. Now left: 20
ID:3, No fuel. Waiting...
ID:0, No fuel. Waiting...
ID:1, No fuel. Waiting...
Filled fuel ... 80
ID: 3, Got fuel. Now left: 40
ID: 1, Got fuel. Now left: 0
ID:0, No fuel. Waiting...
Filled fuel ... 60
ID: 0, Got fuel. Now left: 20
```
```80```이 남았을 때 ```thread``` 두개가 바로 ```fuel```을 얻는 것을 확인할 수 있다. 