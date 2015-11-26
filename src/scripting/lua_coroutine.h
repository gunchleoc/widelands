/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_SCRIPTING_LUA_COROUTINE_H
#define WL_SCRIPTING_LUA_COROUTINE_H

#include <string>

#include <stdint.h>

#include "scripting/lua.h"

class FileRead;
class FileWrite;

namespace Widelands {
class Player;
class BuildingDescr;
class WareDescr;
class WorkerDescr;
struct Coords;
}  // namespace Widelands

// Easy handling of function objects and coroutines.
class LuaCoroutine {
public:
	// The state of the coroutine, which can either be yielded, i.e. it expects
	// to be resumed again or done which means that it will not do any more work
	// and can be deleted.
	enum {
		DONE = 0,
		YIELDED = LUA_YIELD
	};

	LuaCoroutine(lua_State* L);
	virtual ~LuaCoroutine();

	// Returns either 'DONE' or 'YIELDED'.
	int get_status();

	// Resumes the coroutine and returns it's state after it did its execution.
	int resume();

	// Push the given arguments onto the Lua stack, so that a Coroutine can
	// receive them. This is for example used in the initialization scripts or
	// in hooks.
	void push_arg(const Widelands::Player*);
	void push_arg(const Widelands::Coords&);
	void push_arg(const Widelands::BuildingDescr*);
	void push_arg(const Widelands::WareDescr*);
	void push_arg(const Widelands::WorkerDescr*);
	void push_arg(const std::string&);

	// Accesses the returned values from the run of the coroutine.
	uint32_t pop_uint32();
	std::string pop_string();

private:
	friend class LuaGameInterface;

	// Input/Output for coroutines. Do not call directly, instead use
	// LuaGameInterface methods for this.
	void write(FileWrite&);
	void read(lua_State*, FileRead&);

	lua_State* m_L;
	uint32_t m_idx;
	uint32_t m_nargs;
	uint32_t m_ninput_args;
	uint32_t m_nreturn_values;
};

#endif  // end of include guard: WL_SCRIPTING_LUA_COROUTINE_H
