#include "lua.hpp"
#include "PlayGroundLua.h"
#include <iostream>
#pragma comment(lib,"lua54.lib")

Lua::Lua()
{
	std::cout << "---------- Loading Lua... ----------\n";
	m_p_lua = nullptr;
	reset();
	std::cout << "-------- Lua Load Complete ---------\n";
}

void Lua::reset()
{
	if (m_p_lua) {
		lua_close(m_p_lua);
	}
	m_p_lua = luaL_newstate();
	luaL_openlibs(m_p_lua);
	luaL_loadfile(m_p_lua, "../DiscordAttendance/ProcessCommands.lua");
	if (0 != lua_pcall(m_p_lua, 0, 0, 0)) {
		std::cout << "Lua Error: " << lua_tostring(m_p_lua, -1);
		lua_close(m_p_lua);
		return;
	}
}

void Lua::print_packet_types()
{
	lua_getglobal(m_p_lua, "PacketType");
	if (lua_istable(m_p_lua, -1)) {
		// ���̺��� ��ȸ�ϸ� ���� ���
		lua_pushnil(m_p_lua);  // ���̺��� ù ��° Ű�� �̵�
		while (lua_next(m_p_lua, -2)) {
			// Ű�� ���� ���
			std::string key = lua_tostring(m_p_lua, -2);
			int value = lua_tointeger(m_p_lua, -1);
			std::cout << key << ": " << value << std::endl;

			lua_pop(m_p_lua, 1); // �� POP, key�� �����־�� ���� key�� ã�� �� �ִ�.
		}
	}
	else {
		std::cerr << "PacketType is not a table!" << std::endl;
	}
}
