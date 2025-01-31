#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include "Packets.h"
#include "BoostAsio.h"
#include "lua.hpp"

#pragma comment(lib,"lua54.lib")

using boost::asio::ip::tcp;
typedef unsigned short CLIENT_PORT;

class Session {
private:
	tcp::socket m_socket;
	char m_buffer[BUFFER_SIZE];
public:
	explicit Session(tcp::socket socket) : m_socket(std::move(socket)) {
		Read();
	}
	
	void recv_callback(const boost::system::error_code& ec, const unsigned short recv_length) {
		if (!ec) {
			process_packet(recv_length);
			Read();
		}
		else
		{
			std::cout <<"recv_callback: "<< ec.message() << std::endl;
			m_socket.close();
		}
	}

private:
	void Read() {
		m_socket.async_read_some(boost::asio::buffer(m_buffer, BUFFER_SIZE),
			[this](boost::system::error_code ec, short recv_length) {
				if (!ec) {
					process_packet(recv_length);
					Read();
				}
			});
		//m_socket.async_read_some(boost::asio::buffer(m_buffer), boost::bind(&Session::recv_callback,this,boost::placeholders::_1,boost::placeholders::_2));
			//[this](boost::system::error_code ec, short recv_length) {
			//if (!ec) {
			//	process_packet(recv_length);
			//	read();
			//	Send(recv_length);
			//}
			//else
			//	delete this;
			//});

		//m_socket.async_read_some(boost::asio::buffer(m_buffer), boost::bind(&Session::recv_callback, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void Send(short length) {
		//boost::asio::async_write(m_socket,boost::asio::buffer(m_buffer,length))
	}

	void process_packet(const short recv_length) {
		short curr_length = 0;
		while (curr_length < recv_length-1)
		{
			short packet_size = m_buffer[curr_length];
			if (curr_length + packet_size > BUFFER_SIZE) break;

			switch (m_buffer[curr_length+1]) {
			case PacketType::TEST:
			{
				std::cout << "TEST Packet Received" << std::endl;
			}
				break;
			case PacketType::HELP:
			{
			}
				break;
			}
			curr_length += packet_size;
		}
		if (recv_length < BUFFER_SIZE) {	// All Packets are received
		}
	}
};

class LoginUsers
{
	std::mutex m_mutex;
	std::unordered_map<CLIENT_PORT, Session> m_um_login_users;
public:
	bool try_emplace(CLIENT_PORT& num, tcp::socket& socket)
	{
		m_mutex.lock();
		if (m_um_login_users.try_emplace(num, Session(std::move(socket))).second) {
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