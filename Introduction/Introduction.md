# Introduction

## 1. Thread의 의미
현실 세계는 한 번에 한 가지 일을 하지 않기 때문에, 컴퓨터 역시도 현실을 반영하는 능력을 가져야만 했다. **multi-programming, time sharing, multiprocessing 그리고 thread 개념**이 나오게 된다. 

```thread```는 컴퓨터 공학 용어로 기계 내에서 **연속성(continuousness)과 순서(sequence)**를 나타내는 **속성들의 집합**이다. 즉, ```thread```는 일련의 기계 명령을 실행하는 데 필요한 기계 상태(현재 명령어의 위치, machine address, data register 등)으로 구성된다.

```UNIX```에서 ```process```는 **thread, 주소 공간, file descriptor 등 기타 데이터 모음**으로 생각할 수 있다. 요점은 여러 ```thread```가  ```process```의 주소 공간을 공유하여 다른 작업을 수행할 수 있다는 것이다. 또한, ```다중 프로세서(multi-processor)```에서 ```process```의 ```thread```는 **동시에 다른 작업을 수행**할 수 있다.

## 2. 비동기
```비동기```란 강제 종속성이 없는 한 독립적으로 발생함을 의미한다. 즉, 특정 이벤트가 순차적(동기적)으로 발생하는 것이 아니라, 임의의 시간에 발생하는 것을 말한다. 사실, **비동기적** 이라는 말은 **동시성(concureency), 병렬성(parallelism)**과 별다른 관련이 없다. 어떤 task를 수행하는 데 있어, 무한정 wait하거나 실행이 block되지 않고 프로그램을 수행할 수 있다는 의미이다.  

## 3. 동시성(concureency)
사전에서 동시에 일어나는 일을 가리키는 **동시성(concureency)**는, 컴퓨터 용어로는 동시에 일어나는 것처럼 보이지만, **순차적(serially)**으로 발생한 일을 말한다. **동시성(concureency)**는 단일 프로세서 시스템(uniprocessor system)에서 threads 또는 processes의 실행(행동)을 의미한다. **POSIX**에서 동시성의 실행의 정의는 "호출된 thread의 실행을 일시 중단하는 함수가 다른 thread의 실행을 무기한 일시 중단하지 않아야 한다." 이다.

**동시성(concurrency)**는 동시에 연산이 수행 중 임을 의미하지 않는다. 그럼에도 불구하고, **동시성(concurrency)**는 application이 비동기성의 장점을 얻도록 하고, 독립적인 연산이 진행중일 때, 작업을 수행할 수 있도록 한다.

**POSIX**는 동시성(concurrency)와 비동기성을 모두 제공하며 효율적인 프로그램을 쉽게 작성하는 데 필요한 것이다. 

## 4. 단일 프로세서 및 다중 프로세서
**단일 프로세서(uniprocessor)**는 프로그래머가 단일 프로세서(processor, cpu)를 사용하는 컴퓨터로 프로그래밍을 할 때 사용한다. **다중 프로세서(multi-processor)**는 동일한 물리적 메모리에 접근하는 둘 이상의 프로세서가 있는 컴퓨터를 의미한다.

## 5. 병렬성(parallelism)
**병렬처리**는 동시에 진행되는 동시 sequence를 의미한다. 즉, 소프트웨어의 **병렬성(parallelism)**은 아주 이상하게도 영어 사전의 **동시성(concurrency)**와 동일하고, 소프트웨어의 **동시성(parallelism)**과는 다르다. 

진정한 **병렬처리**는 **다중 프로세서 시스템**에서만 발생할 수 있지만 **동시성**은 **단일 프로세서와 다중 프로세서 시스템** 모두에서 발생할 수 있다. **동시성**은 단일 프로세서에서 발생할 수 있는데, 이는 **동시성**이 **병렬성**의 일면(또는 환상)이기 때문이다. **병렬성**은 프로그램이 한 번에 두 가지 계산(computations)을 수행할 수 있어야 하지만, 동시성은 프로그래머가 프로그램이 한 번에 두가지 일이 발생할 수 있는 '척'을 한다고 생각하면 된다.

## 6. Thread safety와 reentrancy
**Thread-safe**란 다중 thread 상황에서 문제되는 결과없이 code가 호출될 수 있다는 것을 의미한다. 이는 다중 thread 상황에서 **효율적**으로 코드를 작동시키겠다는 의미가 아니라, **안전**하게 코드를 작동시키겠다는 의미이다. **POSIX**는 대표적으로 **mutexes, condition variables 등 thread-specific data**를 제공한다. 

**Thread-safe**는 함수를 직렬화하여 여러 thread가 접근하여도, 하나의 thread만이 실행할 수 있도록 만들 수 있고, 일부만 critical section으로 두어 보호할 수도 있다. 

**reentrancy(재진입)**은 한 thread의 함수 invocation(호출)이 끝나지도 않았는데, 다른 thread가 해당 함수를 invocation하는 경우, 첫번째 action이 코드에 영향을 주지 않도록 하는 것이다. 즉, **reentrancy** code는 정적 데이터에 의존하는 것을 피하며(왜냐하면 정적 데이터에 의존하면 호출할 때마다 변하니까), 이상적으로는 thread간 모든 형태의 동기화에 의존하지 않아야 한다. 이에 따라, **reentrancy**를 "효율적인 thread-afe 기법"이라고도 부른다.

```UNIX```에서는 각 디렉터리 항목을 순서대로 반환하는 ```readdir, readdir_r```함수가 있는데, ```readdir```은 ```mutex```를 사용하여 **thread-safe**를 보장한다. ```readdir_r```은 **reentrancy**을 구현한 것으로 ```mutex```를 사용하지 않고, ```context```를 사용한다. 이에 따라 함수에 lock를 걸지 않고 ```context```정보를 유지하여 안전성을 보장한다. 경우에 따라서는 ```context```의 정보만에 lock을 걸고 정보를 보호할 수 있다.

## 7. 동시성 제어 기능(concurrency control functions)
"concurrent system"은 반드시 application 또는 library에서 어떻게 작동할 지 제어하거나 'concurrent execution contexts(실행 컨텍스트)'를 만들 수 있는 필수적인 기능들을 제공해야 한다. 필수적인 기능들은 다음과 같다.

1. **Execution context(실행 컨텍스트)**: '실행 컨텍스트'는 동시성 entity의 '상태(status)'를 말한다. concurrent system은 실행 컨텍스트 생성 및 삭제, 상태를 독립적으로 유지하는 방법을 제공해야 한다. 가령, 외부 이벤트를 기다려야 하는 경우와 같이 다양한 시간에 한 컨텍스트의 상태를 저장하고 다른 컨텍스트로 전달할 수 있어야 한다.

2. **Scheduling(스케줄링)**: '스케줄링'은 어떤 컨텍스트(또는 context set)를 실행해야 하는 지 결정하고 필요할 때 컨텍스트 간에 전환을 한다.

3. **Synchronization(동기화)**: '동기화'는 동시 실행 컨텍스트(concurrent execution contexts)가 공유 리소스 사용을 조정하기 위한 매커니즘을 제공한다. 즉, 동시에 실행되는 상황을 '동시에 발생하지 않도록 방지'하는 것을 의미한다. 

|   | 실행 컨텍스트  | 스케줄링  | 동기화  |
|---|---|---|---|
| 신호등(현실)  | 자동차  | 신호등 불빛과 신호  | 신호 turn이 바뀜  |
| UNIX(thread 등장 이전) | process  | 우선순위  | wait and pipes  |
| Pthreads  | thread  | policy, priority  | condition variables and mutexes  |

다양한 매커니즘을 사용하여 **동기화(synchronization)**를 제공할 수 있다. 가장 일반적인 것들은 **mutex, condition variable, semaphore, event**등이 있다. 또한, UNIX의 pipe, socket, MQ 또는 비동기 프로세스 간의 통신을 위한 기타 프로토콜과 같은 메시지 전달 매커니즘을 동일한 시스템 또는 네트워크를 통해 사용할 수 있다. 모든 형태의 통신 프로토콜에는 동기화가 포함되어있다. 동기화 없이 데이터를 전달하면 통신이 제대로 되지 않는 문제가 발생하기 때문이다.

중요한 것은 **thread, mutex, condition variable**이다. thread는 **실행 컨텍스트**로서 실행 가능하고, mutex를 통해 thread끼리 예기치 않는 충돌이 발생하지 않도록 방지할 수 있다. 또한, condition varialbe을 사용하면 thread가 이러한 충돌을 피한 후 안전하게 진행할 수 있을 때까지 기다릴 수 있다. 즉, mutex와 condition variable은 thread 작업을 동기화(synchronization)하는데 사용된다.

## 8. 비동기 프로그램
UNIX는 기본적으로 비동기 프로그래밍으로 만들어 졌다. shell에 명령을 입력하면 실제로 독립 프로그램이 시작된다. 가령, terminal에 ```ls```라고 명령어를 입력하면 ```ls```라는 프로그램을 실행하고, terminal은 그 응답을 기다린다. 
다.

**UNIX 파이프 및 파일은 동기화 메커니즘이 될 수 있다.**

```ls | more ```라는 명령어를 shell에 입력하면 ```ls```의 결과값이 ```more```의 input으로 들어가기 전까지 ```more```는 실행을 대기한다. 사실 다중 프로세서에서는 ```ls, more```가 병렬적으로 실행된다. 하지만 ```more```는 ```ls```로 부터 입력값을 받지 못했으므로 ```ls```의출력값만을 기다린다. ```ls```의 출력값은 pipe를 통해 ```more```에 들어가고, ```more```는 중지했던 실행을 다시 재개하는 것이다. UNIX는 pipe를 통해 이런 식으로 비동기 작업을 동기 작업으로 만드는 것이다.

**thread**는 코드를 실행하는 데 필요한 **프로세스**의 '일부'이다. 대부분의 컴퓨터에서 각 thread는 1. 현재 명령어에 대한 포인터(Program Counter, PC), 2. thread 스택의 맨 위에 대한 포인터(Stack pointer), 3. 범용 레지스터(general register) 등으로 구성되어있다. 

또한, thread는 process가 들고 있는 대부분은 갖고 있지않으며 공유하여 사용하는데, 가령 thread는 자기 자신의 file descriptors 또는 address space를 갖지 않는다. 한 process안에 있는 모든 thread는 process의 모든 file들과 process의 text, data segments를 포함한 memory를 공유한다. (참고로 memory는 data(bss-초기화되지않은 전역변수), text(code), heap, stack) 으로 구성되어있다.

**thread는 process보다 훨씬 'simple'하다**

thread는 사실 process의 일부를 '제거한 process'이다. 시스템은 process 간에 context switching 할 수 있는 것보다 process 내의 두 thread 간의 context switching을 훨씬 더 빠르게 작동시킬 수 있다. 이 점의 대부분은 process 내의 thread가 process memory의 text(code), data(+bss),heap 등의 주소 공간을 공유한다는 사실에서 비롯된다. stack은 share하지 않지만, process stack의 정보를 가져다 쓸 수 있다(ex, local variable) 단지, thread간의 독립적인 stack들 끼리 share하지 않는다는 말이다.

또한, 각 process에는 별도의 virtual memory address space가 있지만, 동일한 process 내에서 실행되는 thread는 virtual memory address space와 다른 모든 process data를 공유한다.

정리하자면 다음과 같다.

1. thread들은 process의 대부분 공유한다. 그렇기 때문에 context switching이 훨씬 더 빠르고 효율적이다. 
2. thread들은 process의 text, data, heap 영역을 공유하지만 stack 영역은 서로 공유하지않아 독립적인 stack이 있다.
3. 각 thread는 PC, stack frame pointer, general register 등을 갖고 있다.

## 9. POSIX thread concepts
위에서 계속 언급했던 **POSIX**라는 말은 **Portable Operating System Interface**라는 의미로, 유닉스의 API이다. **Pthreads**는 **POSIX**에서 사용하는 **thread**를 의미하며, **POSIX**는 위에서 언급한 thread를 위한 동기화, thread 생성, 삭제 등 다양한 API를 제공해준다.

즉, **POSIX**는 위에서 **thread system**의 필수적인 요소 3개인 **실행 컨텍스트(thread)**, **스케줄링** , **동기화**를 코드 상에서 구현할 수 있는 API를 제공해준다. 

```pthread_create``` 함수를 호출하면 ```실행 컨텍스트(thread)```를 생성할 수 있다. ```pthread_exit```함수를 호출하면 ```실행 컨텍스트(thread)```를 종료할 수 있다. 또는 ``return```으로 종료할 수 있다.

```Pthread```의 주요 동기화 모델은 thread간 충돌 보호를 위해 ```mutexes```를 사용하고, communication을 위해 ```condition variables```을 사용한다. 물론, ```MQ, semaphores, pipes``` 등의 다양한 동기화 기술도 사용할 수 있다. ```mutex```는 하나의 thread가 shared data를 다른 thread의 방해없이 온전히 사용할 수 있도록, lock 기능을 제공해준다. ```condition variable```은 shared data를 기다리는 thread에게 shared data에 접근할 수 있는 상태로 변경해주는 기능을 한다. (가령, queue에 데이터가 있다. 또는 resource를 사용가능하다 등)

### 9.1 types and interfaces
**Pthread**는 아래의 표와 같은 data types와 rule을 가진다. 이들이 어떻게 동작하고 interact할지는 뒤에서 더 알아보도록 하자.

| Types  | Description  |
|---|---|
| pthread_t  | thread 식별자  |
| pthread_mutext_t  | mutex  |
| pthread_cond_t  | condition variable  |
| pthread_key_t  | 특정 thread-specific data에 대한 "access key"  |
| pthread_attr_t  | thread 속성 object |
| pthread_mutexattr_t  | mutex 속성 object  |
| pthread_condattr_t  | condition variable 속성 object  |
| pthread_once_t  | "one time initialization" control context  |

```pthread```의 이러한 types들은 반드시 표준(standard)로 알려진 방법으로만 사용해야 한다. 즉, 정해진 룰과 type에 맞게 사용해야만 한다는 것이다. 

### 9.2 Checking for errors
```Pthread```는 기존의 UNIX와 C언어의 전통적인 ```error status``` 표현방식을 떼어냈다. 전통적으로 함수들은 자신들의 logic이 성공하면 적절한 return 값을 반환하는게 관례였다. 가령, 성공하면 0보다 큰 숫자 또는 의미있는 값, 실패하면 음수를 반환하는 것처럼 말이다. 이에 따라, ```pthread```역시도 error가 발생하면 return으로 음수를 반환한다. 다만 어떤 문제를 인지를 알려주는 ```errno```이라는 것을 정의하였는데, 이는 global value로 error type을 정의하기 위한 code였다. 즉, ```errno```는 반환되는 값은 아니고, ```pthread```안에서 설정하는 전역 변수 값이었다.

그러나 이러한 방식은 문제를 가졌는데,  ```errno```가 ```extern int```이기 때문에, 이 변수는 한 번에 하나의 값만 가질 수 있었다. 이는 process안의 여러 thread들이 각자의 ```errno```를 설정해도, 남는 것은 오직 single stream 뿐이었다는 것이다. 즉, 여러 thread가 errno를 보내도 남는 건 하나라는 것이다.

그래서 ```Pthread``` 함수는 ```errno```를 설정하지 않기로 하였다.

대신, ```Pthread```는 ```error status``` value로 ```return``` 한다. 그리고 global variable인 ```errno```를 설정하지 않는다. 

