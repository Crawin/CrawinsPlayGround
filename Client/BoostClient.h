#pragma once
#include <boost/asio.hpp>
using namespace boost::asio;
using namespace boost::asio::ip;


void send_callback(const boost::system::error_code& error, size_t bytes_transferred);

void run_client(const std::string& server_ip, unsigned short server_port);

int start();
