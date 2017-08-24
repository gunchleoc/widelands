/*
 * Copyright (C) 2016-2017 by the Widelands Development Team
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

#include "utils/lua_filewrite.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>


/*
 ==========================================================
 SPECIALIZED FILEWRITE
 ==========================================================
 */

// Defines some convenience writing functions for the Lua format
LuaFileWrite::LuaFileWrite() : FileWrite(), level_(0) {
}

void LuaFileWrite::write_string(const std::string& s, bool use_indent) {
	std::string writeme = s;
	if (use_indent) {
		for (int i = 0; i < level_; ++i) {
			writeme = (boost::format("   %s") % writeme).str();
		}
	}
	data(writeme.c_str(), writeme.size());
}
void LuaFileWrite::write_key(const std::string& key, bool use_indent) {
	write_string((boost::format("%s = ") % key).str(), use_indent);
}
void LuaFileWrite::write_value_string(const std::string& value, bool use_indent) {
	write_string((boost::format("\"%s\"") % value).str(), use_indent);
}
void LuaFileWrite::write_value_int(const int value, bool use_indent) {
	write_string((boost::format("%d") % value).str(), use_indent);
}

std::string LuaFileWrite::format_double(const double value) {
	std::string result = (boost::format("%f") % value).str();
	boost::regex re("(\\d+)\\.(\\d+?)[0]+$");
	result = regex_replace(result, re, "\\1.\\2", boost::match_default);
	return result;
}
void LuaFileWrite::write_value_double(const double value, bool use_indent) {
	write_string(format_double(value), use_indent);
}
void LuaFileWrite::write_key_value(const std::string& key,
							const std::string& quoted_value,
							bool use_indent) {
	write_string((boost::format("%s = %s") % key % quoted_value).str(), use_indent);
}
void LuaFileWrite::write_key_value_string(const std::string& key,
									 const std::string& value,
									 bool use_indent) {
	std::string quoted_value = value;
	boost::replace_all(quoted_value, "\"", "\\\"");
	write_key_value(key, "\"" + value + "\"", use_indent);
}
void LuaFileWrite::write_key_value_int(const std::string& key, const int value, bool use_indent) {
	write_key_value(key, boost::lexical_cast<std::string>(value), use_indent);
}
void
LuaFileWrite::write_key_value_double(const std::string& key, const double value, bool use_indent) {
	write_key_value(key, format_double(value), use_indent);
}
void
LuaFileWrite::open_table(const std::string& name, bool newline, bool indent_for_preceding) {
	if (newline) {
		if (indent_for_preceding) {
			write_string("\n");
		}
		if (name.empty()) {
			write_string("{\n", newline || indent_for_preceding);
		} else {
			write_string(name + " = {\n", newline || indent_for_preceding);
		}
	} else {
		if (name.empty()) {
			write_string(" {", newline || indent_for_preceding);
		} else {
			write_string(name + " = { ", newline || indent_for_preceding);
		}
	}
	++level_;
}
// No final comma makes for cleaner code. This defaults to having a comma.
void LuaFileWrite::close_table(int current,
					  int total,
					  bool newline,
					  bool indent_for_following) {
	--level_;
	if (newline) {
		write_string("\n");
	} else {
		write_string(" ");
	}
	if (current < total - 1) {
		write_string("},", newline);
	} else {
		write_string("}", newline);
	}
	if (indent_for_following) {
		write_string("\n");
	}
}

// No final comma makes for cleaner code. This defaults to having a comma.
void LuaFileWrite::close_element(int current, int total, bool newline) {
	if (current < total - 1) {
		if (newline) {
			write_string(",\n");
		} else {
			write_string(", ");
		}
	}
}
