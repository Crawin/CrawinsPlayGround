#include <iostream>
#include <thread>
#include "lua.hpp"
#include "BoostAsio.h"

#pragma comment(lib,"lua54.lib")

int main()
{

	boost::asio::io_context io_context;
	Server server(io_context);

	std::vector<std::thread> threads;
	for (unsigned int i = 0; i < 1/*std::thread::hardware_concurrency()*/; ++i) {
		threads.emplace_back([&io_context]() {io_context.run(); });
	}

	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}