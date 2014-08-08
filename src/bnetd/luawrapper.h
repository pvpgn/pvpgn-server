/*
 * Copyright (C) Anton Burdinuk
 * https://code.google.com/p/luasp/
 */

#ifdef WITH_LUA
#ifndef __LUAWRAPPER_H
#define __LUAWRAPPER_H

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>
#include <cstring>
#include <stdexcept>
#include <map>
#include <vector>

namespace lua
{
	// lua exception

	class exception : public std::exception
	{
	protected:
		std::string _what;
	public:
		explicit exception(const std::string s) :_what(s) {}

		virtual ~exception(void) throw() {}

		virtual const char* what(void) const throw() { return _what.c_str(); }
	};

	// check Lua VM for errors and throw C++ lua::exception
	void throw_lua_exception(lua_State* st, const std::string& addinfo = std::string()) throw(std::exception);

	// Lua VM instance

	class vm
	{
	protected:
		lua_State* st;

		// mutex for lua::bind
		int mutex;

	public:
		vm(void) :st(0), mutex(0) {}

		explicit vm(lua_State* _st) :st(_st), mutex(0) {}

		~vm(void) throw() { done(); }

		// create new Lua VM instance
		void initialize(void) throw(std::exception);

		// load and execute Lua script from file
		void load_file(const char* file) throw(std::exception);

		// load and execute Lua statement from string
		void eval(const std::string& stmt, int offset = 0) throw(std::exception);

		// register external C function
		void reg(const char* name, lua_CFunction func) throw() { lua_register(st, name, func); }
		void reg(const char* package, const luaL_Reg* members) throw() { luaL_register(st, package, members); }

		// destroy Lua VM instance
		void done(void) throw() { if (st) { lua_close(st); st = 0; } }

		lua_State* get_st(void) const throw() { return st; }

		friend class stack;
		friend class bind;
	};


	// Lua stack wrapper (use in custom CFunctions only)

	class stack
	{
	protected:
		lua_State* st;

		int st_top;

	public:
		// initialize stack
		stack(void) :st(0), st_top(0) {}
		explicit stack(lua_State* _st) :st(_st), st_top(_st ? lua_gettop(_st) : 0) {}
		explicit stack(const vm& _vm) :st(_vm.st), st_top(_vm.st ? lua_gettop(_vm.st) : 0) {}

		// return current stack size
		int size(void) const throw() { return lua_gettop(st); }

		// find field in table
		void find(const char* name, int index = LUA_GLOBALSINDEX) throw() { lua_getfield(st, index, name); }

		// push to stack scalar
		void push(unsigned int v) throw() { lua_pushinteger(st, v); }
		void push(int v) throw() { lua_pushinteger(st, v); }
		void push_boolean(int v) throw() { lua_pushboolean(st, v); }
		void push(double v) throw() { lua_pushnumber(st, v); }
		void push(const std::string& v) throw() { lua_pushlstring(st, v.c_str(), v.length()); }
		void push(const char* v) throw() { lua_pushlstring(st, v, std::strlen(v) ); }

		// push to stack std::map
		template<typename _key, typename _val, typename _comp, typename _alloc>
		void push(const std::map<_key, _val, _comp, _alloc>& v) throw()
		{
			lua_newtable(st);

			for (typename std::map<_key, _val, _comp, _alloc>::const_iterator i = v.begin(); i != v.end(); ++i)
			{
				push(i->first);
				push(i->second);
				lua_rawset(st, -3);
			}
		}

		// push to stack std::vector
		template<typename _val, typename _alloc>
		void push(const std::vector<_val, _alloc>& v) throw()
		{
			lua_newtable(st);

			for (int i = 0; i < v.size(); ++i)
			{
				push(i);
				push(v[i]);
				lua_rawset(st, -3);
			}
		}


		// recv from stack scalar
		void get(unsigned int& v, int index) throw() { v = lua_tointeger(st, index); }
		void get(int& v, int index) throw() { v = lua_tointeger(st, index); }
		void get(bool& v, int index) throw() { v = lua_toboolean(st, index); }
		void get(double& v, int index) throw() { v = lua_tonumber(st, index); }
		void get(const char*& v, int index) throw()
		{
			size_t len = 0;
			v = lua_tolstring(st, index, &len);
		}
		void get(std::string& v, int index) throw()
		{
			size_t len = 0;

			const char* p = lua_tolstring(st, index, &len);
			if (p)
				v.assign(p, len);
		}

		// recv from stack std::map
		template<typename _key, typename _val, typename _comp, typename _alloc>
		void get(std::map<_key, _val, _comp, _alloc>& v, int index) throw()
		{
			if (lua_type(st, index) == LUA_TTABLE)
			{
				if (index < 0)
					index = lua_gettop(st) + 1 + index;

				lua_pushnil(st);

				while (lua_next(st, index))
				{
					_val _v; get(_v, -1);
					_key _k; get(_k, -2);
					v[_k] = _v;
					pop();
				}
			}
		}

		// recv from stack std::vector (use std::map instead)
		template<typename _val, typename _alloc>
		void get(std::vector<_val, _alloc>& v, int index) throw()
		{
			if (lua_type(st, index) == LUA_TTABLE)
			{
				if (index < 0)
					index = lua_gettop(st) + 1 + index;

				lua_pushnil(st);
				while (lua_next(st, index))
				{
					_val _v; get(_v, -1);

					if (lua_type(st, -2) == LUA_TNUMBER)
					{
						int _k = lua_tointeger(st, -2);
						if (_k >= 0)
						{
							if (_k >= v.size())
								v.resize(_k + 1);
							v[_k] = _v;
						}
					}

					pop();
				}
			}
		}


		// get value of stack with index check
		template<typename T>
		void at(int index, T& v) throw()
		{
			if (index > 0 && index <= size())
				get(v, index);
			else
				v = T();
		}

		// pop last values from stack
		void pop(void) throw() { lua_pop(st, 1); }
		void popn(int n) throw() { lua_pop(st, n); }
	};


	// manipulators for lua::bind

	class invoke_type {};
	class m_invoke_type {};
	class release_type {};

	extern invoke_type invoke;		// execute function
	extern m_invoke_type m_invoke;	// execute function as method (insert this pointer before argument list)
	extern release_type end;		// release lua::bind (pop staff from Lua stack)

	class lookup			// find field in last table (or LUA_GLOBALSINDEX if first)
	{
	protected:
		std::string name;
	public:
		lookup(const std::string& _name) :name(_name) {}

		friend class bind;
	};


	// class for direct access to Lua table fields

	class table : protected stack
	{
	protected:
		int index;
	public:
		table(void) :stack(0), index(0) {}
		explicit table(lua_State* _st, int _index) :stack(_st), index(_index) {}
		explicit table(vm& _vm) :stack(_vm.get_st()), index(LUA_GLOBALSINDEX) {}

		// get field value
		template<typename _VAL>
		void query(const char* name, _VAL& val) throw()
		{
			if (st)
			{
				lua_getfield(st, index, name);
				stack::get(val, -1);
				stack::pop();
			}
		}

		// set field value
		template<typename _VAL>
		void update(const char* name, const _VAL& val) throw()
		{
			if (st)
			{
				stack::push(val);
				lua_setfield(st, index, name);
			}
		}
	};


	// Lua transaction class (only one per vm instance)

	class bind : protected stack
	{
	private:
		int& mutex;
		int refuse;

	protected:
		int args_number, retvals_number;
		int cur_index;

	public:
		explicit bind(vm& _vm) :stack(_vm), refuse(0), args_number(0), retvals_number(0),
			cur_index(LUA_GLOBALSINDEX), mutex(_vm.mutex)
		{
			if (mutex) refuse++; else mutex++;
		}

		~bind(void) throw() { end(); }

		// find field in last table
		void lookup(const char* name) throw();

		// return lua::table object for last table
		lua::table table(void) throw()
		{
			if (lua_type(st, -1) == LUA_TTABLE)
				return lua::table(st, lua_gettop(st));
			return lua::table();
		}

		// execute last function
		void invoke(void) throw(std::exception);

		// execute last function as method
		void m_invoke(void) throw(std::exception);

		// end transaction (free Lua stack)
		void end(void) throw();

		// push function argument to Lua stack
		template<typename T> bind& operator<<(const T& v) throw() { if (!refuse) { push(v); args_number++; } return *this; }

		// pop function return value from stack
		template<typename T> bind& operator>>(T& v) throw()
		{
			if (refuse || retvals_number < 1)
				v = T();
			else
			{
				get(v, -retvals_number); retvals_number--;
			}

			return *this;
		}

		// for manipulators
		bind& operator<<(const lua::lookup& f) throw(std::exception) { bind::lookup(f.name.c_str()); return *this; }
		bind& operator<<(const lua::invoke_type&) throw(std::exception) { bind::invoke(); return *this; }
		bind& operator<<(const lua::m_invoke_type&) throw(std::exception) { bind::m_invoke(); return *this; }
		bind& operator<<(const lua::release_type&) throw(std::exception) { bind::end(); return *this; }
	};

	typedef bind transaction;
	typedef lookup function;
	typedef lookup field;
}


#endif
#endif