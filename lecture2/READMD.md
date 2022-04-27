# Race condition
**race condition 또는 race hazard**는 시스템의 실직적인 행동(실행 동작)이 제어 불가능한 이벤트의 타이밍 또는 순서에 의해 달라지는 경우를 말한다. 

가령, 
```
data = 0
```
A는 ```data```에 ```data = data + 2```을 하고 싶어하고, B는 ```data```에 ```data = data /2 ```을 하고 싶어한다. 만약, A가 먼저 실행되고, B가 실행되면 ```data = 1```이 된다. 반면, B가 먼저 실행되고, A가 먼저 실행되면 ```data = 2```가 된다.

이 처럼, A , B의 실행 순서에 따라 결과가 달라지는데, A, B의 실행 순서를 제어 불가능하다면 결국 결과는 타이밍(또는 순서)에 의존할 수 밖에 없다. 즉, 에측 불가능한 결과가 발생하다는 것이고, 이를 **race condition**으로 부르기로 한 것이다.

한 가지 정말 재밌는 예제가 있는데 바로 다음이다.

```c
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int mails = 0;

void* routine(){
    for(int i = 0; i < 1000000; i++){
        mails++;
        // read mails
        // increment
        // write mails
    }
}

int main(int argc, char *argv[]){
    pthread_t p1, p2;
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
    printf("Number of mails; %d\n", mails);
    return 0;
}
```
두 thread가 for문에서 ```1000000```만큼 ```mail```을 더한다. 따라서 결과는 ```2000000```가 나와야 정상이다. 

```
gcc -o milionplus.o milionplus.c -lpthread; ./milionplus.o
```

그러나 정말 재밌게도, ```2000000```가 아닌 이상한 겂이 나온다. ```1048898```이 나오기도 하고 ```1052943```이 나오기도 한다. 실행 할 때마다 결과값이 달라지는 것을 확인할 수 있다.

이것이 바로 에측 불가능한 결과가 발생하다는 것 **race condition**이다.

그렇다면, 왜 **race condition**이 발생하는 것일까?? 힌트는 바로 주석에 있다.

```c
void* routine(){
    for(int i = 0; i < 1000000; i++){
        mails++;
        // read mails
        // increment
        // write mails
    }
}
```

사실 ```++```연산자는 굉장히 단순해 보이지만 3가지 연산이 작동하고 있다.

1. mail 값을 읽는다. - t1 
2. mail 값을 증가시킨다. - t2
3. 증가 시킨 임시 값을 mail에 넣는다. - t3

정상적인 경우는 ```p1 thread```가 t1, t2, t3를 모두 마친 후에 ```p2 thread```가 t1, t2, t3를 동작시키면 된다.

그런데, 만약 이들의 순서가 섞인다면?? ```p1 thread```가 증가한 새 값을 써도, ```p2 thread```의 연산 값이 써질 것이다.

정말 다음과 같은 연산 순서가 발생한 것인지, 어셈블리를 확인하여 보도록 하자.
```
gcc -S milionplus.c
```

다음의 ```gcc -S``` 명령을 사용하면, ```milionplus.s```라는 어셈블리어 코드가 나오게 된다. (각자의 결과는 다를 수 있다.)

```s
; %bb.3:                                ;
	ldr	w8, [sp, #4]
	add	w8, w8, #1                      ; =1
	str	w8, [sp, #4]
	b	LBB0_1
```
다음 부분이 바로, ```routine```에서 ```mail++```하고 있는 부분이다.

```ldr(load to register)```로 메모리에 있는 값을 cpu register로 옮기는 것이다. ```add```를 통해 1을 더하고, ```str(store to memory)```는 더한 값을 register에서 memory로 옮기는 것이다.

우리에게 c언어에서는 한 줄의 코드가 사실은 3줄의 코드로 작동되고 있었던 것이고, 이는 곧 동시성 프로그래밍에서 ```race condition```의 대상이 된 것이다.

그런데 우리가 ```routine```의 ```for```문을 ```1000000```만큼 반복하는 것이 아닌, ```100, 1000```만큼으로 바뀌어도 문제가 생기지 않는다. 즉, race condition이 발생하지 않는다. 

이는, 굉장히 단순한데, ```p1 thread```가 ```pthread_create```함수를 통해 실행될 될 때, 아직 ```p2 thread```가 생성되지 않고 ```p1의 routine```이 완료되었기 때문이다.

그런데 1000000만큼 ```for```문 반복하면, ```p1 thread```가 먼저 실행되어 ```p1 routine```을 실행시켜도, ```p2 thread```가 시작되기 전까지 완료되지 않아, **race condition**이 발생하는 것이다.

