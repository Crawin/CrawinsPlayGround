#include <stdio.h>
#include <iostream>
#include <thread>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

// ������ �����Լ�.
void threadTest()
{
    // �ݺ����� ���鼭 �ֿܼ� ���� ����Ѵ�.
    for (int i = 0; i < 4; i++)
    {
        std::cout << i << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

int main()
{
    // thread pool ���� thread �ִ� ������ �Ѱ��� �����Ѵ�.
    boost::asio::thread_pool* pool = new boost::asio::thread_pool(1);
    // ������ ����
    post(*pool, threadTest);
    // ������ ����
    post(*pool, threadTest);
    // ������ ����
    post(*pool, threadTest);

    // pool ���� ��� �����尡 ������ ������ ��ٸ���.
    pool->join();
    // �޸� ����
    delete pool;

    return 0;
}