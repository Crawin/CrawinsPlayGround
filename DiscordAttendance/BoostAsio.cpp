

//int main()
//{
//	lua_State* L = luaL_newstate();
//	luaL_openlibs(L);
//	luaL_loadfile(L, "../DiscordAttendance/ProcessCommands.lua");
//	if (0 != lua_pcall(L, 0, 0, 0)) {
//		std::cout << "Lua Error: " << lua_tostring(L, -1);
//		lua_close(L);
//		return 1;
//	}
//
//	lua_getglobal(L, "PacketType");
//
//    // PacketType이 테이블인지 확인
//    if (lua_istable(L, -1)) {
//        // 테이블을 순회하며 내용 출력
//        lua_pushnil(L);  // 테이블에서 첫 번째 키로 이동
//        while (lua_next(L, -2)) {
//            // 키와 값을 출력
//            std::string key = lua_tostring(L, -2);
//            int value = lua_tointeger(L, -1);
//            std::cout << key <<": "<< value << std::endl;
//
//            lua_pop(L, 1); // 값 POP
//        }
//    }
//    else {
//        std::cerr << "PacketType is not a table!" << std::endl;
//    }
//
//    char buffer[1024] = "";
//    cs_test_packet testpacket;
//    memcpy(buffer, &testpacket, sizeof(testpacket));
//
//    lua_getglobal(L, "ProcessPacketInLua");
//    //lua_getglobal(L, "echo");
//    lua_pushlstring(L, buffer, buffer[0]);
//    
//    lua_pcall(L, 1, 1, 0);
//
//    size_t length = 0;
//    char recv[1024] = "";
//    memcpy(recv, lua_tolstring(L, -1, &length), length);
//    std::cout << recv <<", "<<length << std::endl;
//
//    //int n = lua_tointeger(L, -1);
//
//    //std::cout << "pls " << n << std::endl;
//    lua_pop(L, 1);
//	lua_close(L);
//	return 0;
//}

//#include <boost/asio.hpp>
//#include <iostream>
//#include <memory>
//#include <unordered_map>
//
//using namespace boost::asio;
//using namespace boost::asio::ip;
//
//class Session : public std::enable_shared_from_this<Session> {
//public:
//    Session(tcp::socket socket) : socket_(std::move(socket)) { 
//        start();
//    }
//
//    void start() { do_read(); }
//
//private:
//    tcp::socket socket_;
//    enum { max_length = 1024 };
//    char data_[max_length];
//
//    void do_read() {
//        auto self(shared_from_this());
//        socket_.async_read_some(buffer(data_, max_length),
//            [this, self](boost::system::error_code ec, std::size_t length) {
//                if (!ec) {
//                    for (int i = 0; i < length; ++i) {
//                        std::cout << data_[i];
//                    }
//                    do_read(); }
//            });
//    }
//
//    void do_write(std::size_t length) {
//        auto self(shared_from_this());
//        async_write(socket_, buffer(data_, length),
//            [this, self](boost::system::error_code ec, std::size_t) {
//                if (!ec) { do_read(); }
//            });
//    }
//};
//
//class Server {
//public:
//    Server(io_context& io_context, short port)
//        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
//        do_accept();
//    }
//
//private:
//    tcp::acceptor acceptor_;
//    std::unordered_map<int, Session> login_members;
//    void do_accept() {
//        acceptor_.async_accept(
//            [this](boost::system::error_code ec, tcp::socket socket) {
//                if (!ec) {
//                    //login_members.emplace(socket.remote_endpoint().port(), Session(std::move(socket)));
//                    //login_members[socket.remote_endpoint().port()].start();
//                    std::make_shared<Session>(std::move(socket))/*->start()*/;
//                    std::cout << "Insert";
//                }
//                do_accept();
//            });
//    }
//};
//
//int main() {
//    try {
//        io_context io_context;
//        Server server(io_context, 2222);
//        io_context.run();
//    }
//    catch (std::exception& e) {
//        std::cerr << "Exception: " << e.what() << std::endl;
//    }
//
//    return 0;
//}

#include "BoostAsio.h"

Session::Session(tcp::socket socket, LoginUsers* loginusers) : m_socket(std::move(socket)), m_loginUsers(loginusers)
{ 
	ZeroMemory(m_buffer, BUFFER_SIZE);
}

Session::Session() : m_socket(NULL), m_loginUsers(nullptr)
{
	ZeroMemory(m_buffer, BUFFER_SIZE);
	std::cout << "Default Session Error!\n";
}

void Session::Read()
{
	m_socket.async_read_some(boost::asio::buffer(m_buffer, BUFFER_SIZE),
		[this](boost::system::error_code ec, short recv_length) {
			if (!ec) {
				process_packet(recv_length);
				Read();
			}
			else {
				if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
					CLIENT_PORT port = m_socket.remote_endpoint().port();
					std::cout << '[' << port << "] " << ec.message() << '\n';
					m_loginUsers->erase(port);
				}
				else {
					std::cerr << "Error: " << ec.message() << "\n";
				}
			}
		});
}

void Session::Send(short length)
{
	//boost::asio::async_write(m_socket,boost::asio::buffer(m_buffer,length))
}

void Session::process_packet(const short recv_length)
{
	short curr_length = 0;
	while (curr_length < recv_length - 1)
	{
		short packet_size = m_buffer[curr_length];
		if (curr_length + packet_size > BUFFER_SIZE) break;

		switch (m_buffer[curr_length + 1]) {
		case PacketType::TEST:
		{
			std::cout << "TEST Packet Received" << std::endl;
		}
		break;
		case PacketType::HELP:
		{
			std::cout << "HELP Packet Received" << std::endl;

		}
		break;
		}
		curr_length += packet_size;
	}
	if (recv_length < BUFFER_SIZE) {	// All Packets are received
	}
}