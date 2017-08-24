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

#include "io/filewrite.h"

/*
 ==========================================================
 SPECIALIZED FILEWRITE
 ==========================================================
 */

// Defines some convenience writing functions for the Lua format
class LuaFileWrite : public FileWrite {
public:
	LuaFileWrite();

	void write_string(const std::string& s, bool use_indent = false);
	void write_key(const std::string& key, bool use_indent = false);
	void write_value_string(const std::string& value, bool use_indent = false);
	void write_value_int(const int value, bool use_indent = false);

	std::string format_double(const double value);
	void write_value_double(const double value, bool use_indent = false);
	void write_key_value(const std::string& key,
	                     const std::string& quoted_value,
	                     bool use_indent = false);
	void write_key_value_string(const std::string& key,
	                            const std::string& value,
	                            bool use_indent = false);
	void write_key_value_int(const std::string& key, const int value, bool use_indent = false);
	void write_key_value_double(const std::string& key, const double value, bool use_indent = false);
	void open_table(const std::string& name, bool newline = false, bool indent_for_preceding = false);
	// No final comma makes for cleaner code. This defaults to having a comma.
	void close_table(int current = -2,
	                 int total = 0,
	                 bool newline = false,
	                 bool indent_for_following = false) ;

	// No final comma makes for cleaner code. This defaults to having a comma.
	void close_element(int current = -2, int total = 0, bool newline = false);

private:
	int level_;
};
