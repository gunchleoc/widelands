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

#include <cassert>
#include <memory>
#include <vector>

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
#include "graphic/texture_atlas.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/world.h"
#include "sound/sound_handler.h"

using namespace Widelands;

namespace {
constexpr int kMaximumSizeForTextures = 2048;
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

// Defines some convenience writing functions for the Lua format
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
		if (indent_for_following) {
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


/*
 ==========================================================
 IMAGE SEGMENTATION
 ==========================================================
 */

#define assert_texture_contains_rect(texture, rect) { \
	assert(rect.x >= 0); \
	assert(rect.y >= 0); \
	assert(rect.w <= texture->width()); \
	assert(rect.h <= texture->height()); \
} \


// Find trimmed rect according to transparent pixels.
// Lock texture before you call this function.
Recti find_trim_rect(Texture* texture, const Recti& source_rect) {
	assert_texture_contains_rect(texture, source_rect);
	Recti result = source_rect;
	int max_x = result.x + result.w;
	const int max_y = result.y + result.h;
	// Find left margin
	bool found = false;
	for (int x = result.x; x < max_x && !found; ++x) {
		for (int y = result.y; y < max_y && !found; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				result.x = std::max(result.x, x - 1);
				found = true;
			}
		}
	}
	// Find right margin
	found = false;
	for (int x = max_x - 1; x >= 0 && !found; --x) {
		for (int y = 0; y < max_y && !found; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				result.w = std::min(max_x, x + 1 - result.x);
				found = true;
			}
		}
	}
	max_x = result.x + result.w;
	// Find top margin
	found = false;
	for (int y = result.y; y < max_y && !found; ++y) {
		for (int x = result.x; x < max_x && !found; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				result.y = std::max(result.y, y - 1);
				found = true;
			}
		}
	}
	// Find bottom margin
	found = false;
	for (int y = max_y - 1; y >= 0 && !found; --y) {
		for (int x = result.x; x < max_x && !found; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				result.h = std::min(max_y, y + 1 - result.y);
				found = true;
			}
		}
	}
	return result;
}

// Find all lines that separate regions with content in them horizontally.
// Lock texture before you call this function.
std::vector<int> find_empty_rows(Texture* texture, const Recti& region) {
	assert_texture_contains_rect(texture, region);
	const int max_x = region.x + region.w;
	const int max_y = region.y + region.h;
	std::vector<int> result;
	bool had_a_filled_pixel = false;
	for (int y = region.y; y < max_y; ++y) {
		for (int x = region.x; x < max_x; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				had_a_filled_pixel = true;
				break;
			} else if (x == max_x - 1 && had_a_filled_pixel) {
				result.push_back(y);
				had_a_filled_pixel = false;
				break;
			}
		}
	}
	return result;
}

// Find all lines that separate regions with content in them vertically.
// Lock texture before you call this function.
std::vector<int> find_empty_columns(Texture* texture, const Recti& region) {
	assert_texture_contains_rect(texture, region);
	const int max_x = region.x + region.w;
	const int max_y = region.y + region.h;
	std::vector<int> result;
	bool had_a_filled_pixel = false;
	for (int x = region.x; x < max_x; ++x) {
		for (int y = region.y; y < max_y; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				had_a_filled_pixel = true;
				break;
			} else if (y == max_y - 1 && had_a_filled_pixel) {
				result.push_back(x);
				had_a_filled_pixel = false;
				break;
			}
		}
	}
	return result;
}

// Uses empty lines to split a rect into rectangular stripes.
// Lock texture before you call this function.
// Assumes that 'lines' is not empty.
std::vector<Recti> split_region(const Recti& source, std::vector<int> lines, bool vertical) {
	std::vector<Recti> result;
	Recti current = source;
	for (size_t i = 0; i < lines.size(); ++i) {
		if (vertical) {
			current.w = lines[i] - current.x;
			result.push_back(Recti(current.x, current.y, current.w, current.h));
			current.x = lines[i];
		} else {
			current.h = lines[i] - current.y;
			result.push_back(Recti(current.x, current.y, current.w, current.h));
			current.y = lines[i];
		}
	}
	if (vertical) {
		result.push_back(Recti(current.x, current.y, source.w - current.x, current.h));
	} else {
		result.push_back(Recti(current.x, current.y, current.w, source.h - current.y));
	}
	return result;
}

// Recursively finds and adds trimmed regions to 'result'
// If the bool in 'pending' is true, we have a vertical split; horizontal otherwise.
// We alternate vertical and horizontal split.
// Lock texture before you call this function.
void make_regions(Texture* texture, std::vector<std::pair<Recti, bool>> pending, std::vector<Recti>* result) {
	// All done
	if (pending.empty()) {
		return;
	}

	// Grab a rectangle to process
	std::pair<Recti, bool> splitme = pending.back();
	pending.pop_back();
	// Find an empty line. If no line is found, this rect is done and can be added to the result
	std::vector<int> lines = splitme.second ? find_empty_columns(texture, splitme.first) : find_empty_rows(texture, splitme.first);
	if (lines.empty()) {
		splitme.first = find_trim_rect(texture, splitme.first);
		if (splitme.first.w > 0 && splitme.first.h > 0) {
			result->push_back(splitme.first);
		}
	} else {
		// Use the lines to split the current rectangle into regions, then add them to the todo-list.
		std::vector<Recti> regions = split_region(splitme.first, lines, splitme.second);
		for (const auto& region : regions) {
			pending.push_back(std::make_pair(region, !splitme.second));
		}
	}
	// Now recurse
	make_regions(texture, pending, result);
}

// Returns a new texture with the trimmed contents defined by 'rect'.
std::unique_ptr<Texture> trim_texture(const Image* texture, const Recti& rect) {
	std::unique_ptr<Texture> result(new Texture(rect.w, rect.h));
	result->blit(Rectf(0, 0, rect.w, rect.h), *texture, rect.cast<float>(), 1., BlendMode::Copy);
	return result;
}


/*
 ==========================================================
 BUILDING TEXTURE ATLAS
 ==========================================================
 */

// A string identifier to reference the image for the given 'region' and 'frame'
std::string region_name(size_t region, size_t frame) {
	return (boost::format("r_%lu_f_%lu.png") % region % frame).str();
}

// Data about the animation Texture Atlas that we need to record in our Lua file.
struct SpritemapData {
	SpritemapData(const Recti& init_rectangle) : textures_in_atlas(new std::map<std::string, std::unique_ptr<Texture>>()), rectangle(init_rectangle), regions(new std::vector<Recti>()) {}
	std::map<std::string, std::unique_ptr<Texture>>* textures_in_atlas;
	const Recti rectangle;
	std::vector<Recti>* regions;
};

// Creates a spritemap from the images, writes it to a .png file and returns information about its
// Texture Atlas.
const SpritemapData* make_spritemap(std::vector<const Image*> images, const std::string& image_filename, FileSystem* out_filesystem) {
	const uint16_t w = images[0]->width();
	const uint16_t h = images[0]->height();

	// The main texture contains the pixels that all animation frames have in common.
	std::unique_ptr<Texture> main_texture(new Texture(w, h));
	main_texture->blit(Rectf(0, 0, w, h), *images[0], Rectf(0, 0, w, h), 1., BlendMode::Copy);
	main_texture->lock();

	// The cookie cutter is a mask to define the pixels that differ between frames.
	std::unique_ptr<Texture> cookie_cutter(new Texture(w, h));
	cookie_cutter->lock();
	for (uint16_t x = 0; x < w; ++x) {
		for (uint16_t y = 0; y < h; ++y) {
			cookie_cutter->set_pixel(x, y, RGBAColor(0, 0, 0, 0));
		}
	}

	// Make pixels in main texture transparent if any of the other images differ
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
					cookie_cutter->set_pixel(x, y, RGBAColor(0, 0, 0, 255));
				}
			}
		}
		current_texture->unlock(Texture::Unlock_Update);
	}

	const SpritemapData* result = new SpritemapData(find_trim_rect(main_texture.get(), Recti(0, 0, main_texture->width(), main_texture->height())));

	main_texture->unlock(Texture::Unlock_Update);
	cookie_cutter->unlock(Texture::Unlock_Update);

	std::vector<std::pair<std::string, std::unique_ptr<Texture>>> to_be_packed;
	// NOCOM Prime suspect for player color alignment error
	std::unique_ptr<Texture> cropped_main_texture(trim_texture(main_texture.get(), result->rectangle));
	to_be_packed.push_back(std::make_pair("main_texture.png", std::move(cropped_main_texture)));

	// Split into regions
	cookie_cutter->lock();
	std::vector<std::pair<Recti, bool>> splitme;
	splitme.push_back(std::make_pair(result->rectangle, true));
	make_regions(cookie_cutter.get(), splitme, result->regions);

	for (size_t i = 0; i < result->regions->size(); ++i) {
		Recti region = result->regions->at(i);
		for (size_t frame_index = 0; frame_index < images.size(); ++frame_index) {
			std::unique_ptr<Texture> frame(trim_texture(images[frame_index], region));
			frame->lock();
			// We want transparent pixels according to the cookie cutter.
			for (uint16_t x = 0; x < frame->width(); ++x) {
				for (uint16_t y = 0; y < frame->height(); ++y) {
					RGBAColor mask = cookie_cutter->get_pixel(region.x + x, region.y + y);
					if (mask.a == 0) {
						frame->set_pixel(x, y, RGBAColor(0, 0, 0, 0));
					}
				}
			}
			frame->unlock(Texture::Unlock_Update);
			to_be_packed.push_back(std::make_pair(region_name(i, frame_index), std::move(frame)));
		}
	}

	cookie_cutter->unlock(Texture::Unlock_Update);

	// Build Texture Atlas
	TextureAtlas atlas;
	for (auto& pair : to_be_packed) {
		atlas.add(*pair.second);
	}

	std::vector<std::unique_ptr<Texture>> texture_atlases;
	std::vector<TextureAtlas::PackedTexture> packed_textures;
	atlas.pack(kMaximumSizeForTextures, &texture_atlases, &packed_textures);

	for (size_t i = 0; i < to_be_packed.size(); ++i) {
		result->textures_in_atlas->insert(
			std::make_pair(to_be_packed[i].first, std::move(packed_textures[i].texture)));
	}

	if (texture_atlases.size() != 1) {
		log("ABORTING. Textures didn't fit in 1 atlas, we have %lu!\n", texture_atlases.size());
		return nullptr;
	}

	// Write Texture Atlas image file
	std::unique_ptr<::StreamWrite> sw(out_filesystem->open_stream_write(image_filename));
	save_to_png(texture_atlases[0].get(), sw.get(), ColorType::RGBA);

	return result;
}


/*
 ==========================================================
 WRITING ANIMATION
 ==========================================================
 */

// Add Texture Atlas regions to Lua file
void write_regions(LuaFileWrite* lua_fw, std::map<std::string, std::unique_ptr<Texture>>* textures_in_atlas, std::vector<Recti>* regions, size_t no_of_frames) {
	for (size_t i = 0; i < regions->size(); ++i) {
		// Rectangle
		lua_fw->open_table("", true, i > 0);
		const Recti& dest_rect = regions->at(i);
		lua_fw->open_table("rectangle", false, true);
		lua_fw->write_value_int(dest_rect.x);
		lua_fw->close_element();
		lua_fw->write_value_int(dest_rect.y);
		lua_fw->close_element();
		lua_fw->write_value_int(dest_rect.w);
		lua_fw->close_element();
		lua_fw->write_value_int(dest_rect.h);
		lua_fw->close_element(0, 0);
		lua_fw->close_table(0, 2, false, true);

		// Offsets
		lua_fw->open_table("offsets", false, true);
		for (size_t frame_index = 0; frame_index < no_of_frames; ++frame_index) {
			lua_fw->open_table("");
			const Recti& source_rect = textures_in_atlas->at(region_name(i, frame_index))->blit_data().rect.cast<int>();
			lua_fw->write_value_int(source_rect.x);
			lua_fw->close_element();
			lua_fw->write_value_int(source_rect.y);
			lua_fw->close_element(0, 0);
			lua_fw->close_table(frame_index, no_of_frames);
		}
		lua_fw->close_table(0, 0); // Offsets
		lua_fw->close_table(i, regions->size(), true); // Region
	}
}

// Reads animation data from engine and then creates spritemap data.
void write_animation(EditorGameBase& egbase, FileSystem* out_filesystem) {
	LuaFileWrite lua_fw;

	const std::string anim_name = "idle"; // NOCOM Add selection of which animation(s) to process to command line.

	egbase.mutable_tribes()->postload();  // Make sure that all values have been set.
	const Tribes& tribes = egbase.tribes();
	log("==========================================\n");

	const TribeDescr* barbarians = tribes.get_tribe_descr(tribes.tribe_index("barbarians"));
	const BuildingDescr* building = barbarians->get_building_descr(barbarians->headquarters());
	const Animation& animation = g_gr->animations().get_animation(building->get_animation(anim_name));
	const Vector2i& hotspot = animation.hotspot();

	// NOCOM We should probably have a test whether the animation is nonpacked.
	std::vector<const Image*> images = animation.images();
	log("Parsing '%s' animation for '%s'\nIt has %lu pictures\n", anim_name.c_str(), building->name().c_str(), images.size());

	// Only create spritemap if animation has more than 1 frame.
	if (images.size() < 2) {
		log("Animation has less than 2 images and doesn't need a spritemap.\n");
		return;
	}
	const SpritemapData* spritemap = make_spritemap(images, anim_name + ".png", out_filesystem);
	if (spritemap) {
		// Now write the Lua file
		lua_fw.open_table(anim_name, true, true);

		lua_fw.write_key("image", true);
		lua_fw.write_string("path.dirname(__file__) .. \"" + anim_name + ".png\"");
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
		lua_fw.write_value_int(hotspot.x - spritemap->rectangle.x);
		lua_fw.close_element();
		lua_fw.write_value_int(hotspot.y - spritemap->rectangle.y);
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

		lua_fw.open_table("regions", true, true);
		write_regions(&lua_fw, spritemap->textures_in_atlas, spritemap->regions, images.size());

		std::vector<const Image*> pc_masks = animation.pc_masks();
		if (!pc_masks.empty()) {
			lua_fw.close_table(0, 2, true); // Regions
			const SpritemapData* pc_spritemap = make_spritemap(pc_masks, anim_name + "_pc.png", out_filesystem);
			lua_fw.open_table("playercolor_regions", true, true);
			write_regions(&lua_fw, pc_spritemap->textures_in_atlas, pc_spritemap->regions, pc_masks.size());

		}
		lua_fw.close_table(0, 0, true); // Regions

		lua_fw.close_table(0, 2, true); // Animation
		lua_fw.write_string("\n");
		lua_fw.write(*out_filesystem, "new_spritemaps.lua");
	}
	log("Done!\n");
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
