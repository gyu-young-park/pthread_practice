# Pthread
최근 현업을 진행하면서, **thread**를 사용할 일이 생겼는데, 생각보다 쉽지 않았습니다. 또한, 대부분의 저서나 강의는 해당 언어의 동시성 프로그래밍을 어떻게 사용하는 지에 대한 메뉴얼만 제공할 뿐, 개념적인 이해에 대해서는 잘 알기 힘들었습니다.

이에 따라, 병렬 프로그래밍, 동시성 프로그래밍의 가장 대표적인 라이브러리인 **POSIX**의 **pthread**를 알아보고, 이를 토대로 **동시성 프로그래밍**의 개념과 사용 방법에 대해서 알아보도록 합니다.

## 참고한 사이트, 저서
1. [CodeVault's pthread lecture](https://www.youtube.com/watch?v=IKG1P4rgm54&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2&index=2)
2. book: Programming with POSIX ® Threads By David R. Butenhof
3. [cs.cmu.edu](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html)