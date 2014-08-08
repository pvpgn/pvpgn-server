
/*
 * Copyright (C) Anton Burdinuk
 */

#ifdef WITH_LUA
#include "luawrapper.h"

namespace lua
{
	invoke_type invoke;
	m_invoke_type m_invoke;
	release_type end;
}


void lua::throw_lua_exception(lua_State* st, const std::string& addinfo) throw(std::exception)
{
	size_t len = 0;

	const char* p = lua_tolstring(st, -1, &len);

	std::string s(p, len);

	if (addinfo.length())
	{
		s.append(" ", 1);
		s.append(addinfo);
	}

	lua_pop(st, 1);

	throw(exception(s));
}

void lua::vm::initialize(void) throw(std::exception)
{
	done();

	st = lua_open();

	if (!st)
		throw(exception("can`t create lua virtual machine instance"));

	luaL_openlibs(st);
}

void lua::vm::load_file(const char* file) throw(std::exception)
{
	if (!st)
		throw(exception("lua virtual machine is not ready"));

	if (luaL_loadfile(st, file) || lua_pcall(st, 0, 0, 0))
		throw_lua_exception(st);
}


void lua::vm::eval(const std::string& stmt, int offset) throw(std::exception)
{
	enum { max_chunk_name_len = 64 };

	if (offset > stmt.length())
		offset = stmt.length();

	const char* p = stmt.c_str() + offset;

	int n = stmt.length() - offset;

	if (luaL_loadbuffer(st, p, n, n > max_chunk_name_len ? (std::string(p, max_chunk_name_len) + "...").c_str() : p) || lua_pcall(st, 0, 0, 0))
		throw_lua_exception(st);
}


void lua::bind::lookup(const char* name) throw()
{
	if (!refuse)
	{
		if (lua_type(st, cur_index) != LUA_TNIL)
		{
			lua_getfield(st, cur_index, name);
			cur_index = size();
		}
	}
}

void lua::bind::end(void) throw()
{
	if (!refuse && mutex)
	{
		int n = size() - st_top;
		if (n > 0)
			lua_pop(st, n);

		mutex = 0;

		args_number = retvals_number = 0;
		cur_index = LUA_GLOBALSINDEX;
	}
}

void lua::bind::invoke(void) throw(std::exception)
{
	int function_index = -(args_number + 1);

	if (!refuse && lua_isfunction(st, function_index))
	{
		int n = size();

		if (lua_pcall(st, args_number, LUA_MULTRET, 0))
			throw_lua_exception(st);

		retvals_number = size() + args_number + 1 - n;
	}
}

void lua::bind::m_invoke(void) throw(std::exception)
{
	if (!refuse)
	{
		int function_index = -(args_number + 1);
		int this_index = function_index - 1;

		if (lua_isfunction(st, function_index) && lua_istable(st, this_index))
		{
			lua_pushvalue(st, this_index);
			args_number++;
			lua_insert(st, -args_number);


			int n = size();

			if (lua_pcall(st, args_number, LUA_MULTRET, 0))
				throw_lua_exception(st);

			retvals_number = size() + args_number + 1 - n;
		}
	}
}
#endif