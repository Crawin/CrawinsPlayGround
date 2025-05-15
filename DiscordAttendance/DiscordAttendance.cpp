//#include <iostream>
//#include <thread>
//#include <string>
//#include <unordered_map>
//#include <functional>
#include "stdafx.h"
#include "BoostAsio.h"
#include "Bot.h"

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
	commands["users"] = [&server]() {
		const std::unordered_map<CLIENT_PORT, Session>& LoginUsers = server.getLoginUsers();
		int cnt = 1;
		for (const auto& user : LoginUsers) {
			std::cout << cnt++ << ". " << user.first << '\n';
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
	for (unsigned int i = 0; i < /*1*/std::thread::hardware_concurrency() / 2; ++i) {
		threads.emplace_back([&io_context]() {io_context.run(); });
	}
	Bot bot;
	threads.emplace_back([&bot]() {bot.Run(); });

	server_commands(server, io_context);

	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}