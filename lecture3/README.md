# Mutex
이전 시간에 동시성 프로그래밍은 공유 자원에 대한 **race condition** 문제가 발생한다는 것을 확인하였다. 이를 문제를 해결하기 위한, **mutex**를 알아보자.

공유 자원이 **race condition**이 발생하는 부분을 **critical section**이라고 한다. 한국말로는 **임계 구역**이라고 번역되기도 한다.

**critical section(임계 구역)**에서 공유 자원에 대한 **race condition**이 발생하는 이유는 서로 다른 thread들끼리 연산 순서가 뒤엉켜 제어가 불가능하기 때문이다.

그렇다면, 공유 자원에 접근하기 위해서 어떠한 제약을 걸어야 하는데, 동시에 여러 thread가 접근하지 못하도록 하는 것이다. 이것을 **mutual exclusion**, 한국말로 **상호 배제**라고 한다. 

**critical section**에 대한 **mutual exclusion**는 하나의 thread가 공유 자원을 점유하면, 다른 thread는 공유 자원을 점유하지 못하고, 기다리게 된다. 이는 공유 자원에 대한 베타적인 접근성을 보장하는데, 이를 통해서 **race condition**문제를 해결한 것이다.

즉, 여러 thread가 달라 붙어서 공유 자원에 대한 **race condition**문제를 발생시키는 **critical section**에 **mutual exclusive**한 잠금 장치를 걸어, 한 번에 하나의 thread만이 **critical section**에서 자신의 로직을 동작시킬 수 있도록 하는 것이다. 그리고 여기에 **mutual exclusion**을 보장하기위한 잠금 장치가 바로 **mutex**이다. 참고로 **mutex**는 **'mut'ual 'ex'clusion**에서 앞 글자만 본따 만든 단어이다.

## 1. pthread_mutex
**pthread**에서 **mutex**를 제공해주는데, 의아할 수도 있다. 그냥 **int lock=1** 이런 변수를 하나 선언해서, 잠금 장치처럼 사용하면 안되는 것인가?? 

**mutex**는 **atomic**한 연산이 보장되어야 한다. 즉, **mutex**가 작동할 때에는 **context switching**를 작동하지 말아야한다는 것이다. 일반 변수인 **int lock**을 하나의 **mutex**처럼 사용하려면 **int lock**이라는 변수를 **atomic**하게 만들어야 하는데, 이는 상당히 복잡한 작업이 필요하다. 따라서, **pthread**에서 제공하는 **mutex**를 사용하면 기본적으로 **atomicity**를 보장하므로, 이를 사용하는 것이 좋다.

- 기본적인 **pthread_mutex** 연산은 다음과 같다.
1. **pthread_mutex_t**를 통해서 **mutex**를 변수를 만들어 낼 수 있다.
2. **pthread_mutex_init**로 **mutex**를 초기화 시킬 수 있다. 만약 **mutex** 변수를 선언할 떄 **PTHREAD_MUTEX_INITIALIZER**로 초기화해주었다면 굳이 사용하지 않아도 된다.
3. **pthread_mutex_destroy**로 **mutex**의 메모리를 해제할 수 있다.
4. **pthread_mutex_lock**를 통해서 **lock**을 동작하게 할 수 있다. **lock**이 동작하면 이 이후부터는 **mutual exclusion**이 보장되어, **lock**을 점유하지 못한 다른 thread들은 자원을 기다리며 block된다.
5. **pthread_mutex_unlock**로 **lock**을 해제할 수 있다. **lock**이 해제되면 자원을 기다리던 **thread**들이 **lock**을 얻는 동작을 실행한다.

이를 이용하여, 이전의 예제를 고쳐보도록 하자.

- milionplus.c
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

다음의 명령어를 통해서 구동시켜주도록 하자.
```
gcc -o milionplus.o milionplus.c -lpthread; ./milionplus.o
```
결과를 확인하면 

```
Number of mails; 2000000
```

가 나온다. 즉, 올바른 정답이 나오는 것이다. 다만, 시간이 굉장히 오래걸리는 데 이는 **mutex**를 사용하는 것이 연산량이 많아 그런 것이다. 즉 **mutex**를 사용하는 것은 시스템의 부하를 가져올 수 있다는 것이다. 그렇지만, thread들의 행동을 제어할 수 있어 원하는 결과를 가져올 수 있다는 장점이 있다.

참고로, 지금은 ```thread_mutex_init(&mutex, NULL);```으로 ```mutex```를 초기화하였는데, 이것을 사용하지 않고, ```pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;```로 초기화할 수도 있다. 만약 둘 중 하나의 초기화 작업을 해주지 않으면 **mutex**가 작동하지 않으니 주의하도록 하자.