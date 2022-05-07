# pthread_t, pthread_create
이전까지는 **pthread**로 thread를 만드는 방법을 그냥 넘어가고, 결과만 확인하였다. 이번 시간에는 그냥 넘어갔던 **pthread_create** 부분들을 다시 되짚고, 알아보도록 하자.

**pthread**를 생성하는 방법은 매우 간단하다.

1. **pthread_t** 타입을 통해서, thread 인스턴스를 만든다. 이 안에는 **thread** 식별자가 들어있는데, 이를 통해서 해당 thread를 제어할 수 있다. 참고로 **pthread_t** 타입으로 thread 변수만 선언한다해서, thread가 작동하지않는다. 
2. **pthread_create**를 통해서 thread를 동작 시킬 수 있다. 인자로는 4가지 파라미터를 가지는데, **pthread_t 주소, attribute, 함수, 함수 입력 값**이다. 리턴값이 만약 0이 아니면 에러를 의미한다. 참고로 함수의 시그니처는 ```void* func(void*)```이다.  
3. **pthread_join**는 process(main thread)와 인자로 주어진 **pthread_t**를 연결해주어, 해당 **thread_t**가 종료되기 까지 동작을 멈춘다. 즉, **main thread**가 대기 상태에 들어간다. **pthread_join**연산을 해주지 않으면 **thread**들이 함수에 있는 일들을 다 실행하기도 전에 **main**함수 부분이 다 끝나버려 **process**가 종료된다.

참고로, **pthread**는 자원을 반환해야하는데, 만약 **pthread_join**로 실행된 경우는 **pthread**가 실행을 종료하고 자동으로 자원을 반납한다. 마찬가지로 **detach**모드로 실행되는 경우도 자동으로 자원을 반납한다. 참고로, **detach**모드는 **pthread_create**에서 **attributes**를 넘겨주어야 설정된다. 

만약 아무것도 설정하지 않은 경우, 즉 **detach** 모드도 아니고 **pthread_join**도 설정하지 않으면 자원을 반납하지 않으니 조심하도록 하자.

## 1. pthread_t
```pthread_t```은 ```opaque data type```으로 정보가 노출되어 있지않다. 한 가지 재밌는 것은 ```phtread_t```는  ```unsigned long```으로 하나의 값이다. 이 값은 ```thread```의 **id**를 의미한다. 그래서 출력해보면 다음의 값들을 갖는 것을 확인할 수 있다.

- pthread_id.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

void* routine(void* arg){
    pthread_t id = pthread_self();
    printf("thread[%lu] start!\n", id);
}

int main(int argc, char* argv[]){
    pthread_t th[2];
    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], NULL, routine, NULL) != 0){
            perror("Failed to create thread");
        }
        printf("pthread_t value:[%lu]\n",th[i]);
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
}
```
다음의 코드를 보면, ```pthread_t```를 직접 ```printf```해서 실행하는 코드가 있고, ```routine```에서는 현재 실행 중인 ```thread```의 ```id```를 받아오는 ```pthread_self()```를 사용하여 ```id```를 출력하도록하였다. 두 결과를 살펴보면 다음과 같다.

```
pthread_t value:[6136459264]
thread[6136459264] start!
pthread_t value:[6137032704]
thread[6137032704] start!
```
```main```에서 ```pthread_t```를 출력한 값과 ```routine```에서 ```pthread_self```로 얻어온 ```id```값이 동일한 것을 확인할 수 있다. 이를 통해, ```thread```들은 ```id```로 관리된다는 것을 확인할 수 있다.

한 가지 재밌는 것은 ```linux system thread api```로 ```thread id```를 가져온 값과 ```pthread_t```의 값이 다르다는 것이다. ```main```함수에서 ```printf("pthread_t value:[%lu]\n",th[i]);```를 지우고, ```thread```함수인 ```routine```에 다음의 코드를 넣어보도록 하자. (*주의 mac에서는 아래의 코드가 구동되지 않을 수 있다. 이는 mac의 아키텍처에 따라 다르다.*)

```c
void* routine(void* arg){
    pthread_t id = pthread_self();
    printf("pthread[%lu] start!\n", id);
    printf("thread[%d] start!\n", (pid_t)syscall(SYS_gettid));
}
```
```(pid_t)syscall(SYS_gettid)```을 통해서 우리는 현재 실행 중인 ```thread```가 ```linux os```상에서 어떤 ```id```값으로 제어되고 있는 지 확인할 수 있다. 결과를 확인하면

```
pthread[22952182613760] start!
thread[8508] start!
pthread[22952184715008] start!
thread[8507] start!
```
```pthread_t```와 ```syscall```을 통해 얻은 ```linux thread```의 ```id```값이 다르다는 것을 확인할 수 있다. 이유는 아주 간단한데, 우리의 program은 api로 pthread를 사용한다. 이 ```pthread```안에서 ```thread```를 구분하고 제어하기위해 사용하는 것이 바로 ```pthread_t```이다. 이 ```pthread_t```의 ```id```값을 이용하여 ```thread```를 만들고 제어하는데, 실제 ```thread```가 만들어지고 작동하고 있는 곳은 ```os```쪽이다. 따라서, ```pthread```는 ```os```의 ```thread```생성, 제어를 도와주는 하나의 ```api```인데, ```pthread```에서 추상적으로 ```os thread```를 구분하기위해 사용하는 것인 ```pthread_t```이다. ```os thread```는 ```pthread```와는 다른 ```thread id```를 사용하고 있고, 이를 호출하기 위해 사용한 것이 ```syscall(SYS_gettid)```이다.

```
program <--pthread_t--> pthread(lib) <--tid--> os
```

즉, ```program```단에서 ``os linux```를 쉽게 사용하기위해 ```pthread```가 사용되고, ```pthread```는 하나의 추상화 계층으로 실제 ```thread id```를 사용하는 것이 아닌, 자신들만의 ```pthread_t```를 구별자로 쓰며 이를 ```os thread id```와 맵핑한 것이다. 그래서 ```pthread```와 ```os thread```의 ```id```가 달랐던 것이다.

## 2. pthread_create와 pthread_join 사용을 조심해야하는 경우
이전 강의에서 사용했던 코드를 최적화시켜보자.

```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int mails = 0;
pthread_mutex_t mutex;

void* routine(){
    for(int i = 0; i < 1000000; i++){
        pthread_mutex_lock(&mutex);
        mails++;
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[]){
    pthread_t p1, p2;
    pthread_mutex_init(&mutex, NULL);
    if(pthread_create(&p1, NULL, &routine, NULL) != 0){
        return 1;
    }
    if(pthread_create(&p2, NULL, &routine, NULL) != 0){
        return 2;
    }
    if(pthread_join(p1, NULL) != 0){
        return 3;
    }
    if(pthread_join(p2, NULL) != 0){
        return 4;
    }
    pthread_mutex_destroy(&mutex);
    printf("Number of mails; %d\n", mails);
    return 0;
}
```

p1, p2 ```pthread_t```가 따로 ```pthread_create```되는 것이 맘에 들지 않는다. 이를 ```for문```으로 묶어 주도록 하자.

```c
int main(int argc, char *argv[]){
    pthread_t p[2];
    pthread_mutex_init(&mutex, NULL);
    int i = 0;
    for(i = 0; i < 2; i++){
        if(pthread_create(p+i, NULL, &routine, NULL) != 0){
            return 1;
        }
        if(pthread_join(p+i, NULL) != 0){
            return 2;
        }
    }
    pthread_mutex_destroy(&mutex);
    printf("Number of mails: %d\n", mails);
    return 0;
}
```

다음과 같이 **pthread_t p[2]** 배열을 만들고, 이를 반복문 **for**문 안으로 **pthread_create**와 **pthread_join**을 엮어내었다. 

그리고 이를 실행해보면

```
Number of mails: 2000000
```
라는 원하는 결과를 얻게 된다. 

그러나, 이는 사실 **pthread**를 잘못 쓴 사례에 속한다. 무엇이 잘못되었는가? thread를 사용하여 병렬 프로그램, 동시성 프로그램을 구현하고 있는 것처럼 보이지만, 사실은 그렇지 않다. 완전히 동기적인 프로그램이 만들어진 것이다.

```c
int main(int argc, char* argv[]) {
    pthread_t th[2];
    int i;
    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < 2; i++) {
        if (pthread_create(th + i, NULL, &routine, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
        printf("Thread %d has started\n", i);
        if (pthread_join(th[i], NULL) != 0) {
            return 2;
        }
        printf("Thread %d has finished execution\n", i);
    }
   
    pthread_mutex_destroy(&mutex);
    printf("Number of mails: %d\n", mails);
    return 0;
}
```
다음과 같이 ```pthread_create```로 **thread**가 생성된 이루에 ```rintf("Thread %d has started\n", i);```로그를 찍도록 하고, ```pthread_join```로 thread 종료 알림이 오면 ```printf("Thread %d has finished execution\n", i);```을 찍도록 하자.

우리가 원하는 것은 이들이 동시에, 병렬적으로 작동하므로, 두 thread가 실행하고, 두 thread가 종료하기를 예상한다.

그러나 결과는 다음과 같다.

```
Thread 0 has started
Thread 0 has finished execution
Thread 1 has started
Thread 1 has finished execution
Number of mails: 20000000
```

이는 **thread 0**이 먼저 실행되고, 종료된 다음 **thread 1**이 실행되고 종료된 것으로 보인다. 즉, 이들은 병렬적으로 작동하고 있는 것이 아니라, 완전히 동기적으로, 순차적으로 작동하고 있는 것이다.

왜 이럴까?? 이는 **pthread_join**이 **pthread_create**으로 **thread**를 생성한 다음 바로 실행되기 때문이다. 즉, **thread 0**이 생성되고, **main thread**는 **pthread_join**으로 **thread 0**이 실행을 완료하기까지 기다린다. 그리고 **thread 0**이 실행을 완료하면, **thread 1**이 **pthread_create**에 의해 실행되고 **main thread**는 **thread 1**에 **pthread_join**되기 때문에 **thread 2**가 실행을 완료할 때까지 block되어 있는다.

따라서, 이를 해결하려면 **pthread_create**와 **pthread_join**을 나누어 for문을 작동시켜주면 된다. 즉, **thread**를 쭉 실행시켜주고, 쭉 **pthread_join**시켜주면 된다.

```c
int main(int argc, char* argv[]) {
    pthread_t th[4];
    int i;
    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < 4; i++) {
        if (pthread_create(th + i, NULL, &routine, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
        printf("Thread %d has started\n", i);
    }

    for (i = 0; i < 4; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            return 2;
        }
        printf("Thread %d has finished execution\n", i);
    }
   
    pthread_mutex_destroy(&mutex);
    printf("Number of mails: %d\n", mails);
    return 0;
}
```

다음과 같이 ```main```코드를 변경해주도록 하자. 위 코드는 **thread**를 4개 까지 늘렸고, 이들을 생성하는 **pthread_create**와 이들을 main thread(process)와 join하는 **pthread_join**의 for 반복문을 나누어 사용한 코드이다.

위와 같이 for문을 나누면 한번에 thread들이 쭉 생성되어 병렬적으로 연산을 진행하고, 이를 **join**을 통해서 하나씩 종료를 기다리는 것이다.

```
Thread 0 has started
Thread 1 has started
Thread 2 has started
Thread 3 has started
Thread 0 has finished execution
Thread 1 has finished execution
Thread 2 has finished execution
Thread 3 has finished execution
Number of mails: 40000000
```
잘 실행되었음을 확인할 수 있다.

