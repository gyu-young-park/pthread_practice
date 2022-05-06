# pthread_exit
```pthread_exit```은 현 실행 중인 ```thread```의 실행을 중지하도록 한다. 

- main.c
```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

void* routine(void* arg){
    printf("hello routine!\n");
    pthread_exit(NULL);
    printf("bye routine!\n");
}

int main(){
    pthread_t th;

    if(pthread_create(&th, NULL, routine, NULL) != 0){
        perror("Failed to create thread");
    }

    if(pthread_join(th, NULL) != 0){
        perror("Failed to join thread");
    }

    printf("Done process exit\n");
}
```
- 결과
```
hello routine!
Done process exit
```
간단한 예제로, ```pthread_exit```을 사용하여 ```thread```의 실행을 중지하도록 하였다. ```thread```는 ```"bye routine"```도 출력못하고 종료하게 된다.
그런데 사실 이는 ```return```과 별반 다를바 없다.

```c
void* routine(void* arg){
    printf("hello routine!\n");
    return 0;
    printf("bye routine!\n");
}
```
다음의 코드로 변경하여 실행해보도록 하자. 아마 같은 결과가 나올 것이다. 그렇다면 ```pthread_exit```은 어디에 사용해야할까??

```pthread_exit```은 굳이 사용할 필요가 없긴하다. ```thread```의 종료는 ```return```으로도 충분히 가능하기 때문이다. 그러나 한 가지 ```pthread_exit```이 쓰일 수 있는 곳이 있다.

만약, ```main```코드에서 문제가 생겼다고 하자. 이때 ```main```은 종료하고, ```thread```들은 나머지 실행을 완료하도록 하기위해서는 어떻게해야할까??

- main.c
```c
int main(){
    pthread_t th;
    if(pthread_create(&th, NULL, routine, NULL) != 0){
        perror("Failed to create thread");
    }
    return 0;
    if(pthread_join(th, NULL) != 0){
        perror("Failed to join thread");
    }
    printf("Done process exit\n");
}
```
```main```코드에서 다음과 같이 ```thread```의 ```join```직전에 ```return```을 하여 종료하도록 하자. 즉, ```thread```의 완료를 기다리지 않고 종료하는 것이다. 이렇게 되었을 때 ```thread```가 채 실행되기도 전에 ```main```코드가 완료되어 ```process```가 다운된다.

중요한 것은 ```process```라는 것이다. 즉, ```main process```가 다운되어 ```thread```의 실행이 미처 완료되지 못하고 끝나버리는 것이다. ```return, exit(0)```은 ```main```에서 쓰일 때 현재 ```process```를 종료하는 기능을 한다. 그러나, ```pthread_exit```을 쓰면 좀 다르다.

사실 ```main```코드는 ```main thread```에서 구동되고 있는 것이다. 즉, ```main thread```만 죽일 수 있다는 것이다. 그렇게 되면 나머지 ```thread```들은 ```process```가 죽지않아 실행을 완료하고 죽게될 것이다. 그래서 ```pthread_exit```이 사용되는 것이다. ```pthread_exit```은 ```process```를 다운시키는 것이 아니라 ```thread```를 종료시키기 때문이다.

- main.c
```c
int main(){
    pthread_t th;
    if(pthread_create(&th, NULL, routine, NULL) != 0){
        perror("Failed to create thread");
    }
    pthread_exit(0);
    if(pthread_join(th, NULL) != 0){
        perror("Failed to join thread");
    }
    printf("Done process exit\n");
}
```
```pthread_exit```덕분에 ```main thread```만 종료되고, ```thread```는 실행되어 결과로 ```hello routine!```이 나오는 것을 볼 수 있다. 즉 ```main thread```는 ```thread```가 종료될 때까지 기다리지 않고 종료되었지만, ```process```가 죽지 않아 다른 ```thread```가 자신의 실행을 완료하고 ```process```가 종료되는 것을 확인할 수 있던 것이다.

**pthread_exit**은 process가 아닌 thread들이 종료시킨다는 사실을 명심해두자.
