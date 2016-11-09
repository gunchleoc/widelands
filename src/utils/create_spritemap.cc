/*
 * Copyright (C) 2016 by the Widelands Development Team
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

#include <memory>

#include <SDL.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "config.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/world.h"
#include "sound/sound_handler.h"

using namespace Widelands;

namespace {

/*
 ==========================================================
 SETUP
 ==========================================================
 */

// Setup the static objects Widelands needs to operate and initializes systems.
std::unique_ptr<FileSystem> initialize(const std::string& output_path) {
	i18n::set_locale("en");

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw wexception("Unable to initialize SDL: %s", SDL_GetError());
	}

	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));

	std::unique_ptr<FileSystem> out_filesystem(&FileSystem::create(output_path));

	// We don't really need graphics or sound here, but we will get error messages
	// when they aren't initialized
	g_gr = new Graphic();
	g_gr->initialize(Graphic::TraceGl::kNo, 1, 1, false);

	g_sound_handler.init();
	g_sound_handler.nosound_ = true;
	return out_filesystem;
}

// Cleanup before program end
void cleanup() {
	g_sound_handler.shutdown();

	if (g_gr) {
		delete g_gr;
		g_gr = nullptr;
	}

	if (g_fs) {
		delete g_fs;
		g_fs = nullptr;
	}

	SDL_Quit();
}

/*
 ==========================================================
 SPECIALIZED FILEWRITE
 ==========================================================
 */

// Defines some convenience writing functions for the JSON format
class LuaFileWrite : public FileWrite {
public:
	LuaFileWrite() : FileWrite(), level_(0) {
	}

	void write_string(const std::string& s, bool use_indent = false) {
		std::string writeme = s;
		if (use_indent) {
			for (int i = 0; i < level_; ++i) {
				writeme = (boost::format("   %s") % writeme).str();
			}
		}
		data(writeme.c_str(), writeme.size());
	}
	void write_key(const std::string& key, bool use_indent = false) {
		write_string((boost::format("\"%s\" = ") % key).str(), use_indent);
	}
	void write_value_string(const std::string& value, bool use_indent = false) {
		write_string((boost::format("\"%s\"") % value).str(), use_indent);
	}
	void write_value_int(const int value, bool use_indent = false) {
		write_string((boost::format("%d") % value).str(), use_indent);
	}

	std::string format_double(const double value) {
		std::string result = (boost::format("%f") % value).str();
		boost::regex re("(\\d+)\\.(\\d+?)[0]+$");
		result = regex_replace(result, re, "\\1.\\2", boost::match_default);
		return result;
	}
	void write_value_double(const double value, bool use_indent = false) {
		write_string(format_double(value), use_indent);
	}
	void write_key_value(const std::string& key,
	                     const std::string& quoted_value,
	                     bool use_indent = false) {
		write_string((boost::format("\"%s\" = %s") % key % quoted_value).str(), use_indent);
	}
	void write_key_value_string(const std::string& key,
	                            const std::string& value,
	                            bool use_indent = false) {
		std::string quoted_value = value;
		boost::replace_all(quoted_value, "\"", "\\\"");
		write_key_value(key, "\"" + value + "\"", use_indent);
	}
	void write_key_value_int(const std::string& key, const int value, bool use_indent = false) {
		write_key_value(key, boost::lexical_cast<std::string>(value), use_indent);
	}
	void
	write_key_value_double(const std::string& key, const double value, bool use_indent = false) {
		write_key_value(key, format_double(value), use_indent);
	}
	void
	open_table(const std::string& name, bool newline = false, bool indent_for_preceding = false) {
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
	void close_table(int current = -2,
	                 int total = 0,
	                 bool newline = false,
	                 bool indent_for_following = false) {
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
		if (newline || indent_for_following) {
			write_string("\n");
		}
	}

	// No final comma makes for cleaner code. This defaults to having a comma.
	void close_element(int current = -2, int total = 0, bool newline = false) {
		if (current < total - 1) {
			if (newline) {
				write_string(",\n");
			} else {
				write_string(", ");
			}
		}
	}

private:
	int level_;
};

void write_animation(EditorGameBase& egbase, FileSystem* out_filesystem) {
	LuaFileWrite fw;

	const std::string anim_name = "idle"; // NOCOM

	egbase.mutable_tribes()->postload();  // Make sure that all values have been set.
	const Tribes& tribes = egbase.tribes();
	const TribeDescr* barbarians = tribes.get_tribe_descr(tribes.tribe_index("barbarians"));
	const BuildingDescr* building = barbarians->get_building_descr(barbarians->headquarters());
	const Animation& animation = g_gr->animations().get_animation(building->get_animation(anim_name));
	const Vector2i& hotspot = animation.hotspot();

	std::vector<const Image*> images = animation.images();
	std::vector<const Image*> pc_masks = animation.pc_masks();

	fw.open_table(anim_name, true, true);

	fw.write_key("image", true);
	fw.write_string("path.dirname(__file__) .. \"" + std::string("NOCOM") + "\"");
	fw.close_element(0, 2, true);
	fw.write_key("representative_image", true);
	fw.write_string("path.dirname(__file__) .. \"" + std::string(g_fs->fs_filename(animation.representative_image_filename().c_str())) + "\"");
	fw.close_element(0, 2, true);

	fw.open_table("rectangle", false, true);
	fw.write_value_int(0);
	fw.close_element();
	fw.write_value_int(0);
	fw.close_element();
	fw.write_value_int(animation.width());
	fw.close_element();
	fw.write_value_int(animation.height());
	fw.close_element(0, 0);
	fw.close_table();
	fw.write_string("\n");

	fw.open_table("hotspot", false, true);
	fw.write_value_int(hotspot.x);
	fw.close_element();
	fw.write_value_int(hotspot.y);
	fw.close_element(0, 0);
	fw.close_table();
	fw.write_string("\n");

	if (animation.nr_frames() > 1) {
		fw.write_key_value_int("fps", animation.frametime() / 1000, true);
		fw.close_element();
	}

	fw.close_table(0, 2, true);

	// NOCOM just for testing
	fw.open_table("foo", true, true);
	fw.open_table("ints", true);
	fw.write_value_int(hotspot.x, true);
	fw.close_element(0, 2, true);
	fw.write_value_int(hotspot.y, true);
	fw.close_element(0, 0, true);
	fw.close_table(0, 2, true);

	fw.open_table("strings", true);
	fw.write_value_string("foo", true);
	fw.close_element(0, 2, true);
	fw.write_value_string("bar", true);
	fw.close_element(0, 0, true);
	fw.close_table(0, 2, true);

	fw.open_table("floats", false, true);
	fw.write_value_double(1.0f);
	fw.close_element(0, 2);
	fw.write_key_value_double("double", 5.302430);
	fw.close_element(0, 0);
	fw.close_table(0, 2, false, true);

	fw.write_key_value_string("1", "bar", true);
	fw.close_element(0, 2, true);
	fw.write_key_value_string("2", "baz", true);
	fw.close_element(0, 0, true);
	fw.close_table(0, 0, true);
	fw.write(*out_filesystem, "test.lua");
}

}  // namespace

/*
 ==========================================================
 MAIN
 ==========================================================
 */

int main(int argc, char** argv) {
	if (argc != 2) {
		log("Usage: %s <existing-output-path>\n", argv[0]);
		return 1;
	}

	const std::string output_path = argv[argc - 1];

	try {
		std::unique_ptr<FileSystem> out_filesystem = initialize(output_path);
		EditorGameBase egbase(nullptr);
		write_animation(egbase, out_filesystem.get());
	} catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		cleanup();
		return 1;
	}
	cleanup();
	return 0;
}
