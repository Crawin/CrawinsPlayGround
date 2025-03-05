#pragma once
constexpr unsigned short BUFFER_SIZE = 1024;
constexpr unsigned short PORT_NUMBER = 2222;
#pragma pack(1)

enum PacketType : unsigned char{
	TEST,
	HELP,
	HELLO,
	BYE,
	FUNNY,
	LOL
};

class packet_base {
	unsigned char Size;
protected:
	PacketType Type;
public:
	packet_base(const unsigned char& packetSize) : Size(packetSize) { std::cout << (int)Size << std::endl; }
};

class cs_test_packet : packet_base {
	int temp;
	int test;
public:
	cs_test_packet() : packet_base(sizeof(*this)) {
		Type = PacketType::TEST;
		temp = 10;
		test = 22;
	}
};

class cs_help_packet : packet_base {
public:
	cs_help_packet() :packet_base(sizeof(*this)) {
		Type = PacketType::HELP;
	}
};

#pragma pack()