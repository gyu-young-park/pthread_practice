## 1. pthread_join
```pthread_join```은 **main thread(process)**와 **thread**를 **join** 연결해주는 역할을 한다. 여기서 **join**이라는 것은, thread끼리는 원래 **병렬**하게 동작하는데 **join**을 하게되면 **A thread**가 **B thread**가 종료되기 까지를 기다리게 된다는 것이다.

가령, **main thread**와 **join**하지않고, **thread**를 실행하면 어떻게 되는 지 알아보도록 하자.

- rolldice.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

void* roll_dice(){
    int value = (rand() % 6 + 1);
    printf("%d\n", value);
}

int main(int argc, char* argv[]){
    srand(time(NULL));
    pthread_t th;
    if(pthread_create(&th, NULL, &roll_dice, NULL) != 0){
        return 1;
    }
    return 0;
}
```
**pthread_t th**는 **roll_dice**라는 함수를 thread로 실행하는데, **roll_dice**는 1~6까지의 수를 랜덤으로 출력하는 **thread**이다. 그런데, 다음의 코드를 아무리 실행해도 결과는 보이지 않는다.

이유는 **pthread_t th**를 **pthread_create**로 생성하여 실행한다해도 **main thread(process즉, main 함수)**가 더 먼저 끝나버려서 **process**가 작업을 종료해버리기 때문이다.

그래서 이전까지의 예제에서 계속해서 **pthread_join**을 사용한 것이다. **main thread**에 **thread**를 **join**시켜버리면 **thread**가 실행이 완료되기까지 **main thread**는 기다리게되고, 결과를 확인할 수 있기 떄문이다.

```c
int main(int argc, char* argv[]){
    srand(time(NULL));
    pthread_t th;
    if(pthread_create(&th, NULL, &roll_dice, NULL) != 0){
        return 1;
    }
    if(pthread_join(th, NULL) != 0){
        return 2;
    }
    return 0;
}
```
**main 코드**를 다음과 같이 변경해주면 문제없이 결과가 출력될 것이다.

**pthread_join**함수의 시그니처를 알아보면 다음과 같다.
```c
int pthread_join(pthread_t, void **)
```
첫번쨰 매개변수는 **pthread_t**이다. 포인터 타입도 아니라서 그 자체를 넘겨주기만 하면된다.

두번쨰 매개변수는 **thread**의 리턴값이다. 리턴값은 ```void**```타입이다. 

**pthread_join**은 **thread**가 종료되기 전까지 **main thread**의 실행을 멈춘다. 그렇기 떄문에 **thread**의 **return value**도 가져다 줄 수 있는 것이다. 한 번 **return value**를 받아오는 코드를 만들어보도록 하자.

- rolldice.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

void* roll_dice(){
    int value = (rand() % 6 + 1);
    int *result = malloc(sizeof(int));
    *result = value;
    printf("%d\n", value);
    return (void*)result;
}

int main(int argc, char* argv[]){

    int* ret;
    srand(time(NULL));
    pthread_t th;
    if(pthread_create(&th, NULL, &roll_dice, NULL) != 0){
        return 1;
    }
    if(pthread_join(th, (void**)&ret) != 0){
        return 2;
    }
    printf("result is : %d\n",*ret);
    free(ret);
    return 0;
}
```
```roll_dice``` 함수에서 동적할당을 통해 ```int *result ``` 변수를 하나만들고 거기에 ```value```값을 넣어주었다. 리턴할 떄는 타입에 맞게 리턴해주기 위해서 ```(void*)result```로 리턴해준다. 

```main```함수에서는 ```int* ret;```변수를 통해서 ```pthread_join(th, (void**)&ret)```의 리턴값을 매개변수로 받아왔다. 그래서 출력해보면 ```value```와 ```ret```의 값이 같을 것이다.

단, 이는 동적할당을 사용했기 떄문에 ```free()```을 해주어야 한다. 안그러면 메모리 leak가 발생하는 위험이 있다. 지금이야 단순한 코드이기 떄문에 별 문제 없어보이지만, 코드가 복잡할 수록 이렇게 코드를 만드는 것은 큰 위험이 있다.

일반적으로 동적할당은 생성과 삭제(malloc - free)는 한 블럭안에 있어야 한다. 그러나 지금은 ```main```과 ```roll_dice```라는 두 함수를 거치고 있어 매우 위험하다. 시스템이 복잡해질 수록 동적할당으로 발생하는 에러는 더욱 많아질 것이고 치명적인 손상을 입힐 것이다.

이러한 문제를 해결하는 방법은 **thread** 함수의 매개변수로 포인터를 넘겨주어 값을 변경하도록 해주는 방법이 있다.