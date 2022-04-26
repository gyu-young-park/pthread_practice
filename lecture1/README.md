## 1. process와 thread의 차이
https://www.youtube.com/watch?v=IKG1P4rgm54&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2&index=2

정말 좋은 영상이 있어, 해당 내용을 정리하여 동시성 프로그래밍을 정리하려고 한다.

먼저 **parent process**에서 **child process**를 만드는 **fork()**를 통해 프로세스의 특징을 알아보자.

- process
```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char* argv[]){
    int pid = fork();
    if(pid == -1){
        return -1;
    }
    printf("Process id %d\n", getpid());
    if(pid != 0){
        wait(NULL);
    }
    return 0;
}
```
위 코드는 **parent process**가 **fork**를 통해서 **child process**를 생성해내고, **getpid()**를 통해서 이들 간의 **pid**를 비교하는 것이다. 참고로 **pid**는 process id로 프로세스의 고유한 번호이다.

```c
if(pid != 0){
    wait(NULL);
}
```
위 코드는 **parent process**의 동작을 block시키는 코드이다. **fork**시에는 **child process**가 동일하게 **main**코드를 동작시키는데, 이때 **child process**는 새로운 **child**를 생성하지 않고, 자기 자신이 **child**이기 떄문에 **0**을 리턴한다. 

그래서, **pid != 0**이면 **parent process**를 말하는 것이고, 이 조건문 안의 **wait(NULL)**이 실행되면 **parent process**는 **block**되어, **child process**가 끝날 때까지 가디리게 된다. **child process**가 동작을 완료하면, 그제서야 block이 풀리고 **parent process**가 동작하게 된다.

위 코드를 빌드하여 실행해보면
```
gcc -o process.o  process.c; process.o
```

```
Process id 1802
Process id 1803
```
다음과 같은 결과가 나온다. 첫번째로 나온것이 **parent process**의 pid이고, 두번째가 **child process**의 pid이다. 따라서, 이들은 서로 다른 완전히 독립적인 프로세스를 구동하고 있다는 것을 확인할 수 있다.

그렇다면 **thread**의 경우에는 어떻게 **process id**가 형성되는 지 알아보자.

- thread.c
```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* routine(){
    printf("Process id %d\n", getpid());
}

int main(int argc,char *argv[]){
    printf("Process id %d\n", getpid());
    pthread_t t1, t2;
    if(pthread_create(&t1, NULL, &routine, NULL)){
        return 1;
    }
    if(pthread_create(&t2, NULL, &routine, NULL)){
        return 2;
    }
    if(pthread_join(t1, NULL)){
        return 3;
    }
    if(pthread_join(t2, NULL)){
        return 3;
    }
    return 0;
}
```
```pthread_t t1, t2;```로 thread_id 두 개를 만들어 놓고,

```c
if(pthread_create(&t1, NULL, &routine, NULL)){
    return 1;
}
if(pthread_create(&t2, NULL, &routine, NULL)){
    return 2;
}
```
두 개의 ```thread```를 나란히 실행하는 코드이다. 이때 ```pthread_join```를 통해서 현재의 ```process```와 ```join```을 해주어야 한다. 이렇게 해주지 않으면, ```thread```가 실행을 완료하기도 전에 ```process```가 실행을 끝내버려 결과를 확인하지 못한다. 즉, ```process```가 ```main```코드를 전부 실행하기 전까지 ```thread```가 실행이 완료되지 못하는 경우가 생긴다는 것이다.

실행하는 명령어는 다음과 같다. 참고로 ```pthread```를 사용하려면, ```-lpthread```옵션을 추가해주어야 컴파일된다.
```
gcc -o thread.o thread.c -lpthread; ./thread.o
```

결과는 다음과 같다.

```
Process id 2239
Process id 2239
Process id 2239
```

**process id가**가 동일한 것을 확인할 수 있다. 현재 **부모 프로세스**, **thread1**, **thread2**의 프로세스 id가 3개가 모두 같다는 것은, 이들이 **같은 프로세스 내에서 구동 중이라는 의미이다.**

이전에 말했듯이, ```thread```는 프로세스 내에 존재하고, 프로세스의 자원을 공유한다. 공유하는 부분은 프로세스 메모리 영역 중 **데이터 영역, 텍스트(코드) 영역, 힙 영역**이다. 따라서, thread는 process의 힙에 있는 데이터도 가져다 쓸 수 있으며, 전역 변수를 담당하는 데이터 영역도 접근이 가능해 **전역 변수**에도 접근할 수 있다.

유일하게 process의 **스택 영역**만 **thread**들 끼리 공유하지 못하고, **thread** 자신들 만의 **스택 영역**이 존재하는데, 사실 이는 반은 맞고, 반은 틀리다. 맘만 먹으면 **process의 스택 프레임 포인터 레지스터**를 가져와 스택에 있는 데이터를 꺼낼 수 있다. 즉, **프로세스의 스택 영역** 역시도 thread들 사이에 공유되고 있다는 것이다. process들의 로컬 변수들이 담기는 **스택 영역**을 **thread**들끼리 공유할 필요가 없다. 자신들 만의 독립적인 실행을 위해서 **스택과 범용 레지스터, pc(program counter)**들이 필요하기 때문에, 이들은 **thread** 각각이 독립적으로 갖고 있으면 되기 때문이다.

## 2. 자원을 공유하는 thread
그렇다면, **thread**와 **process**의 차이점 중, 일부 자원을 공유한다는 측면을 코드로 보도록 하자.

- process.c
```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
    int x = 2;
    int pid = fork();
    if (pid == -1) {
        return 1;
    }
    
    if (pid == 0) {
        x++;
    }
    sleep(2);
    printf("Value of x: %d\n", x);
    if (pid != 0) {
        wait(NULL);
    }
    return 0;
}
```
```fork```하여 **child process**를 만들면, 

아래의 코드를 통해, **child process**는 **x**를 증가시키고, **parent process**는 가만히 두라는 것이다.
```c
if (pid == 0) {
    x++;
}
```

결과는 다음과 같다.
```
Value of x: 3
Value of x: 2
```

**x**값이 **3**으로 나온 것은 **child process**이고, **2**로 나온 것은 **parent process**이다. 

해당 결과를 통해서, **fork()**로 또 다른 프로세스를 만드는 것은 이들 간의 자원을 공유한다는 것을 의미하는 것이 아니며, 이들은 서로 간의 자원을 독립적으로 구성하는 완전히 다른 프로세스라는 것이다.

그렇다면, **thread**의 경우는 어떻게 되는가? **thread**들은 **process**의 메모리 영역을 공유하기 때문에, 전역 변수로 **x**값을 설정하고, x에 대한 연산을 **thread**에서 수행하면 이 결과들이 모든 **thread**들이 공유하고, 반영하는 결과가 나오게 될 것이다.

- thread.c
```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int x = 2;

void* routine() {
    x += 5;
    sleep(2);
    printf("Value of x: %d\n", x);
}

void* routine2() {
    sleep(2);
    printf("Value of x: %d\n", x);
}

int main(int argc, char* argv[]) {
    pthread_t t1, t2;
    if (pthread_create(&t1, NULL, &routine, NULL)) {
        return 1;
    }
    if (pthread_create(&t2, NULL, &routine2, NULL)) {
        return 2;
    }
    if (pthread_join(t1, NULL)) {
        return 3;
    }
    if (pthread_join(t2, NULL)) {
        return 3;
    }
    return 0;
}
```
위 코드는 **thread 1, 2**가 있고, 서로 수행하는 함수가 다르다. **thread1**은 **routine** 함수를 실행하여, 전역 변수 **x** 값에 5를 더하고, 2초간 sleep후 **x**를 print한다. **thread2**는 2초간 sleep후에 전역 변수 **x**값을 printf한다.

결과는 다음과 같다.

```
Value of x: 7
Value of x: 7
```
더한 것은 **thread1**이지만, **thread2** 역시도 전역 변수 **x**의 결과가 **2**가 아닌 **7**로 나온다. 이는 **thread1, 2**가 서로 전역 변수 **x**의 영역을 공유하고 있었기 때문에 가능하다. 즉, **thread**는 **process** 내부에서 동작하여, **process**의 메모리 영역 일부를 공유하기 때문에 가능한 것이다.

하지만, 이러한 **공유 자원** 문제는 엄청난 결과를 초래하는데, 이를 해결하는 것이 **동시성** 프로그래밍의 핵심이다. 앞으로 배울 **mutex, condition variable** 등 다양한 알고리즘들을 배울 것이다.