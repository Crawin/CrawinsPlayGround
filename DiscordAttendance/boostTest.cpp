#include <stdio.h>
#include <iostream>
#include <crtdbg.h>
// boost의 asio라이브러리 크로스 플렛폼에 관계된 라이브러리
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#if _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
using namespace std;
using namespace boost::asio;
using namespace this_thread;
using namespace chrono;
// 쓰레드 예제함수.
void threadTest()
{
    // 반복문을 돌면서 콘솔에 값을 출력한다.
    for (int i = 0; i < 4; i++)
    {
        cout << i << endl;
        sleep_for(microseconds(1));
    }
}

int main()
{
    // thread pool 내에 thread 최대 갯수를 한개로 제한한다.
    thread_pool* pool = new thread_pool(1);
    // 쓰레드 실행
    post(*pool, threadTest);
    // 쓰레드 실행
    post(*pool, threadTest);
    // 쓰레드 실행
    post(*pool, threadTest);

    // pool 내의 모든 쓰레드가 종료할 때까지 기다린다.
    pool->join();
    // 메모리 해제
    delete pool;
    // 메모리 릭체크 함수.
    _CrtDumpMemoryLeaks();
    return 0;
}