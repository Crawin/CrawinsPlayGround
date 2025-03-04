#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include "Packets.h"
#include "lua.hpp"

#pragma comment(lib,"lua54.lib")

using boost::asio::ip::tcp;
typedef unsigned short CLIENT_PORT;

class LoginUsers;

class Session {
private:
	tcp::socket m_socket;
	char m_buffer[BUFFER_SIZE];
	LoginUsers* m_loginUsers;
public:
	explicit Session(tcp::socket socket, LoginUsers* loginusers);
	Session();

	void start() { Read(); }

private:
	void Read();
	void Send(short length);
	void process_packet(const short recv_length);
};

class LoginUsers
{
	std::mutex m_mutex;
	std::unordered_map<CLIENT_PORT, Session> m_um_login_users;
public:
	bool try_emplace(CLIENT_PORT& num, tcp::socket& socket)
	{
		m_mutex.lock();
		auto result = m_um_login_users.try_emplace(num, Session(std::move(socket), this));
		if (result.second == true) {
			result.first->second.start();
			m_mutex.unlock();
			std::cout << "Client connected: " << num << std::endl;
			for (auto it = m_um_login_users.begin(); it != m_um_login_users.end(); ++it)
			{
				std::cout << it->first <<" logined" << std::endl;
			}
			return true;
		}
		else {
			m_mutex.unlock();
			std::cout << "Port [" << num << "] is used" << std::endl;
			return false;
		}
	}
	void erase(const CLIENT_PORT& num) {
		m_mutex.lock();
		m_um_login_users.erase(num);
		m_mutex.unlock();
	}
};

class Server {
private:
	tcp::acceptor m_acceptor;
	LoginUsers m_loginUsers;
private:
	void listen() {
		m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
			if (!ec) {
				unsigned short port_num = socket.remote_endpoint().port();
				m_loginUsers.try_emplace(port_num, socket);
			}
			else {
				std::cout << "Accept Error: " << ec.message() << std::endl;
			}
			listen();
		});
	}
public:
	Server(boost::asio::io_context& io_context) :
		m_acceptor(io_context, tcp::endpoint(tcp::v4(), PORT_NUMBER)) {
		listen();
	}
};