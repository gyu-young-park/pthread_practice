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