#include <stdio.h>
#include <iostream>
#include <thread>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

// 쓰레드 예제함수.
void threadTest()
{
    // 반복문을 돌면서 콘솔에 값을 출력한다.
    for (int i = 0; i < 4; i++)
    {
        std::cout << i << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

int main()
{
    // thread pool 내에 thread 최대 갯수를 한개로 제한한다.
    boost::asio::thread_pool* pool = new boost::asio::thread_pool(1);
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

    return 0;
}