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
#include "graphic/image.h"
#include "graphic/image_io.h"
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
		write_string((boost::format("%s = ") % key).str(), use_indent);
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
		write_string((boost::format("%s = %s") % key % quoted_value).str(), use_indent);
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

// Find trimmed rect according to transparent pixels
// Lock texture before you call this function.
Recti find_trim_rect(Texture* texture) {
	Recti result = Recti(0, 0, texture->width(), texture->height());
	// Find left margin
	bool found = false;
	for (int x = 0; x < texture->width() && !found; ++x) {
		for (int y = 0; y < texture->height() && !found; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				log("NOCOM pixel %d %d\n", x, y);
				result.x = std::max(0, x - 1);
				found = true;
			}
		}
	}
	// Find right margin
	found = false;
	for (int x = texture->width() - 1; x >= 0 && !found; --x) {
		for (int y = 0; y < texture->height() && !found; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				log("NOCOM pixel %d %d\n", x, y);
				result.w = std::min(texture->width(), x + 1) - result.x;
				found = true;
			}
		}
	}
	// Find top margin
	found = false;
	for (int y = 0; y < texture->height() && !found; ++y) {
		for (int x = result.x; (x < result.x + result.w) && !found; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				log("NOCOM pixel %d %d\n", x, y);
				result.y = std::max(0, y - 1);
				found = true;
			}
		}
	}
	// Find bottom margin
	found = false;
	for (int y = texture->height() - 1; y >= 0 && !found; --y) {
		for (int x = result.x; (x < result.x + result.w) && !found; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				log("NOCOM pixel %d %d\n", x, y);
				result.h = std::min(texture->height(), y + 1) - result.y;
				found = true;
			}
		}
	}
	return result;
}

std::set<int> find_empty_rows(Texture* texture) {
	std::set<int> result;
	bool had_a_filled_pixel = false;
	for (int y = 0; y < texture->height(); ++y) {
		for (int x = 0; x < texture->width(); ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				had_a_filled_pixel = true;
				break;
			} else if (x == texture->width() - 1 && had_a_filled_pixel) {
				result.insert(y);
				had_a_filled_pixel = false;
				break;
			}
		}
	}
	return result;
}

std::set<int> find_empty_columns(Texture* texture) {
	std::set<int> result;
	bool had_a_filled_pixel = false;
	for (int x = 0; x < texture->width(); ++x) {
		for (int y = 0; y < texture->height(); ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				had_a_filled_pixel = true;
				break;
			} else if (y == texture->height() - 1 && had_a_filled_pixel) {
				result.insert(x);
				had_a_filled_pixel = false;
				break;
			}
		}
	}
	return result;
}

void write_animation(EditorGameBase& egbase, FileSystem* out_filesystem) {
	LuaFileWrite lua_fw;

	const std::string anim_name = "idle"; // NOCOM

	egbase.mutable_tribes()->postload();  // Make sure that all values have been set.
	const Tribes& tribes = egbase.tribes();
	const TribeDescr* barbarians = tribes.get_tribe_descr(tribes.tribe_index("barbarians"));
	const BuildingDescr* building = barbarians->get_building_descr(barbarians->headquarters());
	const Animation& animation = g_gr->animations().get_animation(building->get_animation(anim_name));
	const Vector2i& hotspot = animation.hotspot();

	std::vector<const Image*> images = animation.images();
	std::vector<const Image*> pc_masks = animation.pc_masks();
	log("NOCOM animation has %lu pictures\n", images.size());

	// Only create spritemap if animation has more than 1 frame.
	if (images.size() > 1) {
		const uint16_t w = images[0]->width();
		const uint16_t h = images[0]->height();
		log("dimension %d, %d\n", w, h);
		std::unique_ptr<Texture> main_texture(new Texture(w, h));
		main_texture->blit(Rectf(0, 0, w, h), *images[0], Rectf(0, 0, w, h), 1., BlendMode::Copy);
		main_texture->lock();
		std::unique_ptr<Texture> main_pc_mask(new Texture(w, h));
		main_pc_mask->blit(Rectf(0, 0, w, h), *pc_masks[0], Rectf(0, 0, w, h), 1., BlendMode::Copy);
		main_pc_mask->lock();

		std::unique_ptr<Texture> cookie_cutter(new Texture(w, h));
		cookie_cutter->lock();
		for (uint16_t x = 0; x < w; ++x) {
			for (uint16_t y = 0; y < h; ++y) {
				cookie_cutter->set_pixel(x, y, RGBAColor(0, 0, 0, 0));
			}
		}

		// Make pixels in main texture transparent if any of the other images differs
		for (size_t i = 1; i < images.size(); ++i) {
			const Image* image = images[i];
			Texture* current_texture = new Texture(w, h);
			current_texture->blit(Rectf(0, 0, w, h), *image, Rectf(0, 0, w, h), 1., BlendMode::Copy);
			current_texture->lock();
			for (uint16_t x = 0; x < w; ++x) {
				for (uint16_t y = 0; y < h; ++y) {
					RGBAColor pixel = main_texture->get_pixel(x, y);
					RGBAColor compareme = current_texture->get_pixel(x, y);
					if (pixel != compareme) {
						main_texture->set_pixel(x, y, RGBAColor(0, 0, 0, 0));
						main_pc_mask->set_pixel(x, y, RGBAColor(0, 0, 0, 255));
						cookie_cutter->set_pixel(x, y, RGBAColor(0, 0, 0, 255));
					}
				}
			}
			current_texture->unlock(Texture::Unlock_Update);
		}
		log("NOCOM cookie_cutter1: %d %d %d %d \n", 0, 0, cookie_cutter->width(), cookie_cutter->height());
		Recti cookie_cutter_rect = find_trim_rect(cookie_cutter.get());
		log("NOCOM cookie_cutter2: %d %d %d %d \n", cookie_cutter_rect.x, cookie_cutter_rect.y, cookie_cutter_rect.w, cookie_cutter_rect.h);

		main_texture->unlock(Texture::Unlock_Update);
		main_pc_mask->unlock(Texture::Unlock_Update);
		cookie_cutter->unlock(Texture::Unlock_Update);

		std::unique_ptr<Texture> trimmed_cookie_cutter(new Texture(cookie_cutter_rect.w, cookie_cutter_rect.h));
		trimmed_cookie_cutter->blit(Rectf(0, 0, cookie_cutter_rect.w, cookie_cutter_rect.h), *cookie_cutter, cookie_cutter_rect.cast<float>(), 1., BlendMode::Copy);

		trimmed_cookie_cutter->lock();
		std::set<int> columns = find_empty_columns(trimmed_cookie_cutter.get());
		for (int col : columns) {
			log("NOCOM empty column at %d\n", col);
		}
		std::set<int> rows = find_empty_rows(trimmed_cookie_cutter.get());
		for (int row : rows) {
			log("NOCOM empty row at %d\n", row);
		}

		trimmed_cookie_cutter->unlock(Texture::Unlock_Update);

		FileWrite image_fw;
		save_to_png(main_texture.get(), &image_fw, ColorType::RGBA);
		log("NOCOM %s\n", out_filesystem->get_working_directory().c_str());
		image_fw.write(*out_filesystem, "main_texture.png");
		save_to_png(main_pc_mask.get(), &image_fw, ColorType::RGBA);
		image_fw.write(*out_filesystem, "main_texture_pc.png");
		save_to_png(cookie_cutter.get(), &image_fw, ColorType::RGBA);
		image_fw.write(*out_filesystem, "cookie_cutter.png");
		save_to_png(trimmed_cookie_cutter.get(), &image_fw, ColorType::RGBA);
		image_fw.write(*out_filesystem, "trimmed_cookie_cutter.png");


		// Now write the Lua file
		lua_fw.open_table(anim_name, true, true);

		lua_fw.write_key("image", true);
		lua_fw.write_string("path.dirname(__file__) .. \"" + std::string("NOCOM") + "\"");
		lua_fw.close_element(0, 2, true);
		lua_fw.write_key("representative_image", true);
		lua_fw.write_string("path.dirname(__file__) .. \"" + std::string(g_fs->fs_filename(animation.representative_image_filename().c_str())) + "\"");
		lua_fw.close_element(0, 2, true);

		lua_fw.open_table("rectangle", false, true);
		lua_fw.write_value_int(0);
		lua_fw.close_element();
		lua_fw.write_value_int(0);
		lua_fw.close_element();
		lua_fw.write_value_int(animation.width());
		lua_fw.close_element();
		lua_fw.write_value_int(animation.height());
		lua_fw.close_element(0, 0);
		lua_fw.close_table();
		lua_fw.write_string("\n");

		lua_fw.open_table("hotspot", false, true);
		lua_fw.write_value_int(hotspot.x);
		lua_fw.close_element();
		lua_fw.write_value_int(hotspot.y);
		lua_fw.close_element(0, 0);
		lua_fw.close_table();
		lua_fw.write_string("\n");

		if (animation.nr_frames() > 1) {
			uint32_t frametime = animation.frametime();
			if (frametime > 0) {
				lua_fw.write_key_value_int("fps", 1000 / animation.frametime(), true);
				lua_fw.close_element();
			}
		}

		lua_fw.close_table(0, 2, true);

		// NOCOM just for testing
		lua_fw.open_table("foo", true, true);
		lua_fw.open_table("ints", true);
		lua_fw.write_value_int(hotspot.x, true);
		lua_fw.close_element(0, 2, true);
		lua_fw.write_value_int(hotspot.y, true);
		lua_fw.close_element(0, 0, true);
		lua_fw.close_table(0, 2, true);

		lua_fw.open_table("strings", true);
		lua_fw.write_value_string("foo", true);
		lua_fw.close_element(0, 2, true);
		lua_fw.write_value_string("bar", true);
		lua_fw.close_element(0, 0, true);
		lua_fw.close_table(0, 2, true);

		lua_fw.open_table("floats", false, true);
		lua_fw.write_value_double(1.0f);
		lua_fw.close_element(0, 2);
		lua_fw.write_key_value_double("double", 5.302430);
		lua_fw.close_element(0, 0);
		lua_fw.close_table(0, 2, false, true);

		lua_fw.write_key_value_string("1", "bar", true);
		lua_fw.close_element(0, 2, true);
		lua_fw.write_key_value_string("2", "baz", true);
		lua_fw.close_element(0, 0, true);
		lua_fw.close_table(0, 0, true);
		lua_fw.write(*out_filesystem, "test.lua");
	}
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
