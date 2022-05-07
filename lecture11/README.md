# pthread_detach
이전에 ```pthread```는 자원 반환을 위해서 ```pthread_join```을 호출하거나 ```detached```모드가 설정되어야 한다고 했다. 즉, **join이나, detached모드 설정이 안되면 pthread는 실행이 종료되어도 자원을 반환하지 않는다**라는 것이다.

즉, ```process```가 진행되면서 수많은 ```thread```들이 만들어지고 생성되는데 자원이 반환되지 않게된다는 것이다. 이런 일을 해결하기위하여 ```join과 detach```가 존재하는 것이다.

그러나, ```join```은 실제 프로그램을 만들 때 그렇게 자주쓰이지는 않는다. 왜냐하면 ```thread```를 만드는 이유 대부분이 ```main thread```가 아닌, ```thread```들이 다른 일을 병렬적으로 실행하여 결과를 비동기적으로 ```main thread```에 결과를 반환하기 바라기 때문이다. 만약 ```join```된다면 ```main thread```는 다른 ```thread```의 결과를 받기위해 실행을 잠시 멈추게된다. 그래서 ```join```을 실제 프로그램을 만들 때 자주 사용하지는 않는다.

그렇다면 ```detach```모드를 사용하는 것이 좋다. 어떻게 ```detach```모드를 설정하여 ```thread```를 설정할 수 있을까??

```c
pthread_detach(pthread_t th);
```
```pthread_detach```를 사용하면 해결된다. 이는 기존의 ```thread```를 ```detach mode```로 변경해주는 코드이다. 

- pthread_detach.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

void* routine(void* arg){
    sleep(1);
    printf("hello routine start\n");
}

int main(int argc, char* argv[]){
    pthread_t th[2];

    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], NULL, &routine, NULL) != 0){
            perror("Failed to create thread");
        }
        pthread_detach(th[i]);
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
}
```
다음의 코드를 실행해보면, 에러가 발생할 것이다.

```c
Failed to join thread: Undefined error: 0
Failed to join thread: Undefined error: 0
```
위의 결과는 ```join```이 실패했다는 것을 알려주는 것이다. 왜 ```join```이 실패했는가? 그것은 ```thread```가 ```detach```모드가 되어서, ```main thread```에서는 더이상 ```join```이 안되기 때문이다. 즉, **detach mode가 되면 thread는 더이상 join되지 않는다.**

그런데, 왜 ```routine```에 있는 ```printf("hello routine start\n");```가 실행되지 않은 걸까?? 그것은 ```main thread```가 ```routine```을 실행하는 ```thread```들 보다 먼저 종료되어 ```process```가 종료되었기 때문이다. 

```main thread```가 먼저 종료되어도 ```detach```된 ```thread```들이 정상 종료할 때까지 ```process```를 잡고있으려면 이전에 배운 ```pthread_exit```을 사용하면 된다.

```main```함수 코드를 다음과 같이 변경하도록 하자.
```c
int main(int argc, char* argv[]){
    pthread_t th[2];

    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], NULL, &routine, NULL) != 0){
            perror("Failed to create thread");
        }
        pthread_detach(th[i]);
    }
    pthread_exit(0);
}
```
결과는 다음과 같다.
```
hello routine start
hello routine start
```
```pthread_exit```덕분에 ```main thread```가 종료되어도 다른 ```thread```들이 실행되어 종료될 때까지 ```process```가 종료되지 않는다.

그런데 만약, ```pthread_detach```가 되기도 전에 ```pthread_exit```이 발생하면 어떻게 될까??

```c
int main(int argc, char* argv[]){
    pthread_t th[2];

    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], NULL, &routine, NULL) != 0){
            perror("Failed to create thread");
        }
        pthread_exit(0);
        pthread_detach(th[i]);
    }
}
```
정상적으로 실행된 것처럼 보이지만 사실은 아니다. ```detach```모드가 아닌 ```thread```는 종료되어도 자원을 완벽히 반환하지 않는다. 즉, ```thread id```등의 정보가 아직도 남아있다는 것이다. 이 문제가 발생하는 가장 큰 이유는 ```detach```를 설정하는 ```pthread_detach```함수가 ```pthread_create```함수가 실행한 다음 실행되기 떄문이다. **정리하자면, pthread_detach는 detach모드가 아닌 thread를 detach로 바꾸는 것이기 때문에 바꾸기 전에 종료되면 자원을 반환할 방법이 없다.**

그렇다면, ```pthread_create```함수가 실행되기 전에 ```detach```모드를 설정할 방법이 없는가? 그것이 바로 ```pthread_create```의 두 번째 매개변수인 ```attributes```이다. 

```attr```를 설정하는 방법은 다음과 같다.
```c
pthread_attr_t attr;
pthread_attr_init(pthread_attr_t*);
pthread_attr_setdetachstate(pthread_attr_t*, int);
...
pthread_attr_destroy(pthread_attr_t*);
```
먼저 ```pthread_attr_t```타입의 변수를 선언하고, ```pthread_attr_init```로 초기화를 해준다. 그 다음 ```pthread_attr_setdetachstate```로 ```int```형 셋팅을 해주는 것이다. macro로 정의되어있는데 ```detach```는 ```PTHREAD_CREATE_DETACHED```이다. 그리고 다 사용했다면 반드시 ```pthread_attr_destroy```로 자원을 반환해야하는 것을 잊지 말도록 하자.

이 ```attribute```를 ```pthread_create```의 두번째 매개변수에 넣어주면 된다.

- pthread_detach.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

void* routine(void* arg){
    sleep(1);
    printf("hello routine start\n");
}

int main(int argc, char* argv[]){       
    pthread_t th[2];
    pthread_attr_t detachedThread;
    pthread_attr_init(&detachedThread);
    pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);

    for(int i = 0; i < 2; i++){
        if(pthread_create(&th[i], &detachedThread, &routine, NULL) != 0){
            perror("Failed to create thread");
        }
    }

    for(int i = 0; i < 2; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    pthread_attr_destroy(&detachedThread);
    pthread_exit(0);
}
```
다음의 코드를 실행해보자.

```
Failed to join thread: Undefined error: 0
Failed to join thread: Undefined error: 0
hello routine start
hello routine start
```
```detach```모드이기 때문에 ```join```이 실패하는 것을 확인할 수 있다. 이렇게 ```detach```모드를 먼저 설정하고 ```pthread_create```로 ```thread```를 만들 수 있다.

실제로는 ```main thread```에서 ```join```을 사용하기 보다는 ```detach```모드로 ```thread```를 만들어서 ```main thread```는 따로 동작하고, ```thread```가 죽을 때면 자원을 알아서 반환하도록 한다.