# 1. pthread_create를 이용하여 매개변수 넘기기
```pthread_create```의 함수 시그니처를 보면 다음과 같다.
```c
int pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void *), void *);
```
총 4개의 매개변수로 이루어져 있는데, 첫번쨰는 **pthread**의 포인터를 넘겨기면 되고, 두 번째는 **pthread의 속성 값인 pthread_attr_t**이다. 세번쨰는 **thread**에서 실행할 함수로 함수 시그니처가 ```void* (*)(void *)```이다. 즉, 리턴 값도, 입력값도 ```void*```이라는 것이다. 마지막 매개변수가 바로, thread 함수에 입력으로 들어갈 매개변수이다.

즉, 4번쨰 매개변수 값이 **thread**함수의 입력 매개변수로 들어가는 것이다.

이를 활용하여 10개의 thread가 서로 다른 10개의 소수를 출력하는 코드를 만들어보자.

- prime.c
```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int prime[10] = {2, 3, 5, 7, 11, 13, 17, 19,23, 29};

void* routine(void* arg){
    int index = *(int *)arg;
    printf("%d ",prime[index]);
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 10; i++){
        if(pthread_create(&th[i], NULL, routine, &i) != 0){
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
간단하게 만들면 다음과 같을 것이다. ```int prime[10]```은 서로 다른 10개의 소수값을 가진다. 

```c
pthread_t th[10];
for(int i = 0; i < 10; i++){
    if(pthread_create(&th[i], NULL, routine, &i) != 0){
        perror("Failed to create thread");
    }
}
```
10개의 thread를 실행하되, index인 ```i```를 thread 함수의 매개변수로 넣어준다. 

```c
void* routine(void* arg){
    int index = *(int *)arg;
    printf("%d ",prime[index]);
}
```
thread함수인 **routine**은 입력 매개변수로 ```i```가 ```arg```에 들어오고, 이를 ```int index```로 값만 가져왔다. 이 ```index```를 통해서 ```prime[index]```의 값을 출력하도록 하면, 순서대로 서로 다른 소수값들이 출력될 것이다.

그러나, 현실은 녹록치 않다.

실제 출력해보면 엉뚱한 값이 나온다.

```
11 5 7 17 13 19 23 0 0 %
```
실행해보면 매번 다른 값이 나올 것이다. 왜 이런 일이 나오는가?? 

이유는 바로 **포인터(주소)**를 매개변수로 넘겨주었기 떄문이다.

우리가 예상하는 상황은 다음과 같다.

1. **i = 0**일떄, **pthread_create**에 **0번쨰 thread**와 **매개변수로 0**이 들어간다. 출력으로는 **prime[0]인 2가** 나온다.
2. **i = 1**일떄, **pthread_create**에 **1번쨰 thread**와 **매개변수로 1**이 들어간다. 출력으로는 **prime[1]인 3가** 나온다.
3. 반복되어... **i = 9**일떄, **pthread_create**에 **9번쨰 thread**와 **매개변수로 9**이 들어간다. 출력으로는 **prime[9]인 29가** 나온다.

이것이 우리가 예상한 시나리오이다. 그러나, 현실은 다음과 같다.
1. **i = 0**일떄, **pthread_create**에 **0번쨰 thread**와 **매개변수로 0**이 들어간다. 그러나 아직 **0번쨰 thread**는 시작하지 않았다.
2. **i = 1**일떄, **pthread_create**에 **1번쨰 thread**와 **매개변수로 1**이 들어간다. 그러나 아직 **1번쨰 thread**는 시작하지 않았다.
3. **i = 2**일떄, **pthread_create**에 **2번쨰 thread**와 **매개변수로 2**이 들어간다. 그러나 아직 **2번쨰 thread**는 시작하지 않았다.
4. 이떄! **1번쨰 thread**가 시작한다. **매개변수는 포인터**이므로 이전에는 **값이 1**였지만 이제는 **값이 2**이다. 따라서, 출력이 **5**가 된다.
5. 이떄! **0번쨰 thread**가 시작한다. **매개변수는 포인터**이므로 이전에는 **값이 0**였지만 이제는 **값이 2**이다. 따라서, 출력이 **5**가 된다.
6. ... 반복

이렇게 되는 것이다. 즉 **thread** 함수에 들어가는 매개변수는 포인터이고, **thread**는 **pthread_create**함수가 실행되어도 바로 시작되는게 아니기 떄문에 생각보다 늦게 **thread**가 실행되면 매개변수 포인터의 값이 달라지고, 우리가 예상한 결과와 달라지게 되는 것이다.

가장 쉽게 이 문제를 눈으로 확인할 수 있는 방법은 ```routine```함수에 ```sleep(1)```을 주어 ```thread```의 실행을 강제로 늦추는 것이다. **thread**들은 실행을 1초간 멈추게 되는데, 이 때 동안 ```for```의```i```는 이미 ```for```이 종료되어 값을 10으로 유지하고 변수는 사라지게 된다. 그래서 쓰레기 값이 나오거나, 0이 나오게 된다.

```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int prime[10] = {2, 3, 5, 7, 11, 13, 17, 19,23, 29};

void* routine(void* arg){
    sleep(1);
    int index = *(int *)arg;
    printf("%d ",prime[index]);
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 10; i++){
        if(pthread_create(&th[i], NULL, routine, &i) != 0){
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
실행해보면 다음의 결과가 나온다.
```
0 0 0 0 0 0 0 0 0 0 
```

이렇듯, **thread**는 매개변수를 포인터로 다루기 떄문에 동시성 문제가 발생한다.

이를 해결하는 방법으로는 동적할당을 이용하는 방법이 있다. 즉 매개변수로 메모리를 넘겨주어야 한다면, 동적할당으로 그 떄마다의 메모리를 넘겨주는 방법이다.

- prime.c
```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int prime[10] = {2, 3, 5, 7, 11, 13, 17, 19,23, 29};

void* routine(void* arg){
    int index = *(int *)arg;
    printf("%d ",prime[index]);
    free(arg);
}

int main(int argc, char* argv[]){
    pthread_t th[10];
    for(int i = 0; i < 10; i++){
        int* index = malloc(sizeof(int));
        *index = i;
        if(pthread_create(&th[i], NULL, routine, index) != 0){
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
```int* index = malloc(sizeof(int));``` 코드를 통해 동적할당으로 메모리 영역을 가지는 포인터를 만들어, **thread**의 매개변수로 넘겨준다. 이렇게 되면, 해당 **thread**의 시작이 느려져서 **i**값이 증가한다더라도, 다른 메모리 영역이기 떄문에 영향을 받지 않는다.

```
2 3 5 7 11 13 17 19 23 29 
```
잘 해결된 듯 보이지만,

이렇게 코드를 만들면 매우매우 복잡해진다. **malloc**과 **free**가 있는 위치를 보자 **malloc**은 **main**에 있는 반면에, **free**는 **routine**함수 안에 있다. 이러한 코드는 시스템 전반적으로 매우 위험한 문제를 초래할 수 있다. 

그렇다면 어떻게 동적할당을 사용하지 않고 이러한 문제를 해결할 수 있을까?? 생각보다 쉬운 방법으로 이러한 문제를 해결할 수 있는데 다음 시간에 살펴보도록 하자.