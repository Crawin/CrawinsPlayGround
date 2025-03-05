#pragma once

struct lua_State;

class Lua
{
	lua_State* m_p_lua;
public:
	Lua();
	void reset();
	void print_packet_types();
};

