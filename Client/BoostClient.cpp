#include <iostream>
#include <string>
#include "Packets.h"
#include "BoostClient.h"

using namespace boost::asio;
using namespace boost::asio::ip;

void send_callback(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error) {
        std::cout << "Message sent: " << bytes_transferred << " bytes" << std::endl;
    }
    else {
        std::cout << "Error during sending message: " << error.message() << std::endl;
    }
}

void run_client(const std::string& server_ip, unsigned short server_port) {
    try {
        // Boost.Asio I/O context
        io_context io_context;

        // Resolve server address and port
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(server_ip, std::to_string(server_port));

        // Create a socket
        tcp::socket socket(io_context);

        // Connect to the server
        connect(socket, endpoints);

        std::cout << "Connected to the server at " << server_ip << ":" << server_port << std::endl;

        while (true) {
            int a;
            std::cin >> a;
            if (a == 1) {
                cs_test_packet test;
                async_write(socket, buffer(&test, sizeof(test)), send_callback);
            }
            if (a == 2) {
                cs_help_packet help;
                async_write(socket, buffer(&help, sizeof(help)), send_callback);
            }
            if (a == 3) {
                socket.close();
            }
            io_context.run();

        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int start() {
    const std::string server_ip = "152.67.205.188"; /*"127.0.0.1";*/ // Change to the server IP address if needed
    const unsigned short server_port = 4280;  // Change to match your server's port

    std::cout << "Starting client..." << std::endl;
    run_client(server_ip, PORT_NUMBER);

    return 0;
}


//#include <boost/asio.hpp>
//#include <iostream>
//#include <thread>
//
//using namespace boost::asio;
//using namespace boost::asio::ip;
//
//class Client {
//public:
//    Client(io_context& io_context, const std::string& host, short port)
//        : socket_(io_context), resolver_(io_context) {
//        auto endpoints = resolver_.resolve(host, std::to_string(port));
//        connect(socket_, endpoints);
//    }
//
//    void write(const std::string& message) {
//        boost::asio::write(socket_, buffer(message + "\n"));
//    }
//
//    void read() {
//        boost::asio::read_until(socket_, boost::asio::dynamic_buffer(response_), '\n');
//        std::cout << "Server: " << response_;
//        response_.clear();
//    }
//
//private:
//    tcp::socket socket_;
//    tcp::resolver resolver_;
//    std::string response_;
//};
//
//int main() {
//    try {
//        io_context io_context;
//        Client client(io_context, "127.0.0.1", 2222);
//
//        std::thread io_thread([&io_context]() { io_context.run(); });
//
//        std::string message;
//        while (std::getline(std::cin, message)) {
//            client.write(message);
//            //client.read();
//        }
//
//        io_thread.join();
//    }
//    catch (std::exception& e) {
//        std::cerr << "Exception: " << e.what() << std::endl;
//    }
//
//    return 0;
//}
