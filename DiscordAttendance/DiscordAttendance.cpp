#include <iostream>
#include <thread>
#include <string>
#include <unordered_map>
#include <functional>
#include "BoostAsio.h"

void server_commands(Server& server, boost::asio::io_context& io_context) {
	std::cout << "-------- Loading Commands... -------\n";
	std::unordered_map < std::string, std::function<void()> > commands;
	commands["stop"] = [&io_context]() {io_context.stop(); };
	commands["reload"] = [&server]() {server.reload_lua(); std::cout << "Lua Reload Complete\n"; };
	commands["packettypes"] = [&server]() {server.packet_types_lua(); };
	commands["help"] = [&commands]() {
		for (const auto& c : commands) {
			std::cout << c.first << '\n';
		}
		};
	std::cout << "------ Commands Load Complete ------\n";

	std::string command;

	while (true) {
		std::cin >> command;
		if (commands.find(command) != commands.end()) {
			commands[command]();
			if (command == "stop") break;
		}
		else {
			std::cout << "Undefined Command!\n";
		}
	}
	
}

int main()
{
	boost::asio::io_context io_context;
	Server server(io_context);
	std::vector<std::thread> threads;
	for (unsigned int i = 0; i < /*1*/std::thread::hardware_concurrency(); ++i) {
		threads.emplace_back([&io_context]() {io_context.run(); });
	}

	server_commands(server, io_context);

	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}