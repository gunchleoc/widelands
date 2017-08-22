/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "graphic/animation.h"

#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_table.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

namespace {

// Parses an array { 12, 23 } into a point.
void get_point(const LuaTable& table, Vector2i* p) {
	std::vector<int> pts = table.array_entries<int>();
	if (pts.size() != 2) {
		throw wexception("Expected 2 entries, but got %" PRIuS ".", pts.size());
	}
	p->x = pts[0];
	p->y = pts[1];
}

// Parses an array { 89, 0, 23, 25 } into a rectangle.
void get_rect(const LuaTable& table, Recti* r) {
	std::vector<int> pts = table.array_entries<int>();
	if (pts.size() != 4) {
		throw wexception("Expected 4 entries, but got %" PRIuS ".", pts.size());
	}
	r->x = pts[0];
	r->y = pts[1];
	r->w = pts[2];
	r->h = pts[3];
}

/**
 * Implements the Animation interface for a packed animation, that is an animation
 * that is contained in a singular image (plus one for player color).
 */
class PackedAnimation : public Animation {
public:
	virtual ~PackedAnimation() {
	}
	PackedAnimation(const std::string& name, const LuaTable& table);

	// Implements Animation.
	float width() const override {
		return rectangle_.w / scale_;
	}
	float height() const override {
		return rectangle_.h / scale_;
	}
	uint16_t nr_frames() const override {
		return nr_frames_;
	}

	Rectf source_rectangle(int percent_from_bottom) const override {
		float h = percent_from_bottom * rectangle_.h / 100;
		return Rectf(0.f, rectangle_.h - h, rectangle_.w, h);
	}

	const Image* representative_image(const RGBColor* clr) const override;
	const std::string& representative_image_filename() const override;
	void blit(uint32_t time,
	          const Rectf& source_rect,
	          const Rectf& destination_rect,
	          const RGBColor* clr,
	          Surface*) const override;
private:
	struct Region {
		Vector2i target_offset = Vector2i::zero();
		uint16_t w, h;
		std::vector<Vector2i> source_offsets;  // indexed by frame nr.
	};
	std::vector<Region>* make_regions(const LuaTable& regions_table);
	const Image* image_for_frame(uint32_t framenumber, const RGBColor* clr) const;

	Recti rectangle_;
	uint16_t nr_frames_;

	const Image* image_;   // Not owned
	const Image* pcmask_;  // Not owned
	std::string representative_image_filename_;
	std::vector<Region> regions_;
	std::vector<Region> pc_regions_;
	std::string hash_;
};

PackedAnimation::PackedAnimation(const std::string& name, const LuaTable& table)
   : Animation(table),
	  rectangle_(0, 0, 0, 0),
     nr_frames_(1), // We'll have 1 frame if there is no region
     image_(nullptr),
     pcmask_(nullptr) {
	try {


		std::string image = table.get_string("image");
		if (!g_fs->file_exists(image)) {
			throw wexception("Packed animation %s - spritemap image %s does not exist.", hash_.c_str(),
			                 image.c_str());
		}
		image_ = g_gr->images().get(image);
		hash_ = image + ":" + name;
		boost::replace_all(image, ".png", "");
		const std::string pc_mask_filename = image + "_pc.png";
		if (g_fs->file_exists(pc_mask_filename)) {
			pcmask_ = g_gr->images().get(pc_mask_filename);
		}

		// We need to define this for idle animations so we can use it in richtext image tags
		if (table.has_key("representative_image")) {
			representative_image_filename_ = table.get_string("representative_image");
		}

		get_rect(*table.get_table("rectangle"), &rectangle_);

		if (table.has_key("regions")) {
			std::unique_ptr<LuaTable> regions_table = table.get_table("regions");
			if (table.has_key("fps")) {
				std::unique_ptr<LuaTable> region_table = regions_table->get_table(1);
				region_table->do_not_warn_about_unaccessed_keys();
				std::unique_ptr<LuaTable> offsets_table = region_table->get_table("offsets");
				offsets_table->do_not_warn_about_unaccessed_keys();
				const auto offsets_keys = offsets_table->keys<int>();
				if (offsets_keys.size() < 2) {
					throw wexception(
					   "Packed animation with one picture %s must not have 'fps'", hash_.c_str());
				}
				frametime_ = 1000 / get_positive_int(table, "fps");
			}
			regions_ = *make_regions(*regions_table.get());
			if (pcmask_) {
				pc_regions_ = *make_regions(*table.get_table("playercolor_regions").get());
			}
		} else if (table.has_key("fps")) {
			throw wexception(
				"Packed animation with one picture %s must not have 'fps'", hash_.c_str());
		}
	} catch (const LuaError& e) {
		throw wexception("Error in packed animation table: %s", e.what());
	}
}

// Parse regions from LuaTable.
std::vector<PackedAnimation::Region>* PackedAnimation::make_regions(const LuaTable& regions_table) {
	std::vector<Region>* result = new std::vector<Region>();
	for (const int region_key : regions_table.keys<int>()) {
		std::unique_ptr<LuaTable> region_table = regions_table.get_table(region_key);
		Region region;
		Recti region_rect;
		get_rect(*region_table->get_table("rectangle"), &region_rect);
		region.target_offset.x = region_rect.x;
		region.target_offset.y = region_rect.y;
		region.h = region_rect.h;
		region.w = region_rect.w;

		std::unique_ptr<LuaTable> offsets_table = region_table->get_table("offsets");
		const auto offsets_keys = offsets_table->keys<int>();
		const uint16_t no_of_offsets = offsets_keys.size();
		if (nr_frames_ != 1 && nr_frames_ != no_of_offsets) {
			throw wexception("Packed animation '%s': region has different number of frames than "
			                 "previous (%i != %i).",
			                 hash_.c_str(), nr_frames_, no_of_offsets);
		}
		nr_frames_ = no_of_offsets;

		for (const int offset_key : offsets_keys) {
			std::unique_ptr<LuaTable> offset_table = offsets_table->get_table(offset_key);
			Vector2i p =  Vector2i::zero();
			get_point(*offset_table, &p);
			region.source_offsets.push_back(p);
		}
		result->push_back(region);
	}
	return result;
}

const Image* PackedAnimation::representative_image(const RGBColor* clr) const {
	return image_for_frame(0, clr);
}

const std::string& PackedAnimation::representative_image_filename() const {
	return representative_image_filename_;
}
// NOCOM This blinks occasionally when blitting - needs some texture atlas fu.
const Image* PackedAnimation::image_for_frame(uint32_t framenumber, const RGBColor* clr) const {
	const int w = image_->width();
	const int h = image_->height();
	std::unique_ptr<Texture> image(new Texture(w, h));
	image->blit(Rectf(0, 0, w, h), *image_, Rectf(0, 0, w, h), 1., BlendMode::Copy);
	for (const Region& region : regions_) {
		Rectf rsrc = Rectf(region.source_offsets[framenumber], region.w, region.h);
		Rectf rdst = Rectf(region.target_offset, region.w, region.h);
		image->blit(rdst, *image.get(), rsrc, 1., BlendMode::UseAlpha);
	}

	Texture* target = new Texture(rectangle_.w, rectangle_.h);

	if (pcmask_ && clr) {
		std::unique_ptr<Texture> pc_image(new Texture(w, h));
		pc_image->blit(Rectf(0, 0, w, h), *pcmask_, Rectf(0, 0, w, h), 1., BlendMode::Copy);
		for (const Region& region : pc_regions_) {
			Rectf rsrc = Rectf(region.source_offsets[framenumber], region.w, region.h);
			Rectf rdst = Rectf(region.target_offset, region.w, region.h);
			pc_image->blit(rdst, *pc_image.get(), rsrc, 1., BlendMode::UseAlpha);
		}
		// NOCOM code duplication
		target->blit(Rectf(0, 0, rectangle_.w, rectangle_.h), *playercolor_image(
							 *clr, (boost::format("packedanim:%s:%d:pc%s") % hash_ % framenumber % clr->hex_value()).str(), image.get(), pc_image.get()),
						 Rectf(rectangle_.x, rectangle_.y, rectangle_.w, rectangle_.h), 1.,
						 BlendMode::UseAlpha);
	} else {
		target->blit(Rectf(0, 0, rectangle_.w, rectangle_.h), *image.get(),
						 Rectf(rectangle_.x, rectangle_.y, rectangle_.w, rectangle_.h), 1.,
						 BlendMode::UseAlpha);
	}
	return target;
}

void PackedAnimation::blit(uint32_t time,
									const Rectf& source_rect,
                           const Rectf& destination_rect,
                           const RGBColor* clr,
                           Surface* target) const {
	assert(target);
	const Image* blitme = image_for_frame(time / frametime_ % nr_frames(), clr);
	target->blit(destination_rect, *blitme, source_rect, 1., BlendMode::UseAlpha);
	trigger_sound(time, 128);
}

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	virtual ~NonPackedAnimation() {
	}
	explicit NonPackedAnimation(const LuaTable& table);

	// Implements Animation.
	float height() const override;
	float width() const override;
	Rectf source_rectangle(int percent_from_bottom) const override;
	uint16_t nr_frames() const override;

	virtual void blit(uint32_t time,
	                  const Rectf& source_rect,
	                  const Rectf& destination_rect,
	                  const RGBColor* clr,
	                  Surface* target) const override;

	std::vector<const Image*> images() const override;
	std::vector<const Image*> pc_masks() const override;
	const Image* representative_image(const RGBColor* clr) const override;
	const std::string& representative_image_filename() const override;

private:
	// Loads the graphics if they are not yet loaded.
	void ensure_graphics_are_loaded() const;

	// Load the needed graphics from disk.
	void load_graphics();

	bool hasplrclrs_;
	std::vector<std::string> image_files_;
	std::vector<std::string> pc_mask_image_files_;
	std::vector<const Image*> frames_;
	std::vector<const Image*> pcmasks_;
};

NonPackedAnimation::NonPackedAnimation(const LuaTable& table)
   : Animation(table), hasplrclrs_(false) {
	try {
		image_files_ = table.get_table("pictures")->array_entries<std::string>();

		if (image_files_.empty()) {
			throw wexception("Animation without pictures. The template should look similar to this:"
			                 " 'directory/idle_??.png' for 'directory/idle_00.png' etc.");
		} else if (table.has_key("fps")) {
			if (image_files_.size() == 1) {
				throw wexception(
				   "Animation with one picture %s must not have 'fps'", image_files_[0].c_str());
			}
			frametime_ = 1000 / get_positive_int(table, "fps");
		}

		for (std::string image_file : image_files_) {
			boost::replace_last(image_file, ".png", "_pc.png");
			if (g_fs->file_exists(image_file)) {
				hasplrclrs_ = true;
				pc_mask_image_files_.push_back(image_file);
			} else if (hasplrclrs_) {
				throw wexception("Animation is missing player color file: %s", image_file.c_str());
			}
		}

		assert(!image_files_.empty());
		assert(pc_mask_image_files_.size() == image_files_.size() || pc_mask_image_files_.empty());

	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
}

void NonPackedAnimation::ensure_graphics_are_loaded() const {
	if (frames_.empty()) {
		const_cast<NonPackedAnimation*>(this)->load_graphics();
	}
}

void NonPackedAnimation::load_graphics() {
	if (image_files_.empty())
		throw wexception("animation without pictures.");

	if (pc_mask_image_files_.size() && pc_mask_image_files_.size() != image_files_.size())
		throw wexception("animation has %" PRIuS " frames but playercolor mask has %" PRIuS " frames",
		                 image_files_.size(), pc_mask_image_files_.size());

	for (const std::string& filename : image_files_) {
		const Image* image = g_gr->images().get(filename);
		if (frames_.size() &&
		    (frames_[0]->width() != image->width() || frames_[0]->height() != image->height())) {
			throw wexception("wrong size: (%u, %u), should be (%u, %u) like the first frame",
			                 image->width(), image->height(), frames_[0]->width(),
			                 frames_[0]->height());
		}
		frames_.push_back(image);
	}

	for (const std::string& filename : pc_mask_image_files_) {
		// TODO(unknown): Do not load playercolor mask as opengl texture or use it as
		//     opengl texture.
		const Image* pc_image = g_gr->images().get(filename);
		if (frames_[0]->width() != pc_image->width() || frames_[0]->height() != pc_image->height()) {
			// TODO(unknown): see bug #1324642
			throw wexception("playercolor mask has wrong size: (%u, %u), should "
			                 "be (%u, %u) like the animation frame",
			                 pc_image->width(), pc_image->height(), frames_[0]->width(),
			                 frames_[0]->height());
		}
		pcmasks_.push_back(pc_image);
	}
}

float NonPackedAnimation::height() const {
	ensure_graphics_are_loaded();
	return frames_[0]->height() / scale_;
}

float NonPackedAnimation::width() const {
	ensure_graphics_are_loaded();
	return frames_[0]->width() / scale_;
}

uint16_t NonPackedAnimation::nr_frames() const {
	ensure_graphics_are_loaded();
	return frames_.size();
}

std::vector<const Image*> NonPackedAnimation::images() const {
	ensure_graphics_are_loaded();
	return frames_;
}

std::vector<const Image*> NonPackedAnimation::pc_masks() const {
	ensure_graphics_are_loaded();
	return pcmasks_;
}

const Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	assert(!image_files_.empty());
	const Image* image = (hasplrclrs_ && clr) ? playercolor_image(*clr, image_files_[0]) :
	                                            g_gr->images().get(image_files_[0]);

	const int w = image->width();
	const int h = image->height();
	Texture* rv = new Texture(w / scale_, h / scale_);
	rv->blit(
	   Rectf(0.f, 0.f, w / scale_, h / scale_), *image, Rectf(0.f, 0.f, w, h), 1., BlendMode::Copy);
	return rv;
}

// TODO(GunChleoc): This is only here for the font renderers.
const std::string& NonPackedAnimation::representative_image_filename() const {
	return image_files_[0];
}

Rectf NonPackedAnimation::source_rectangle(const int percent_from_bottom) const {
	ensure_graphics_are_loaded();
	float h = percent_from_bottom * frames_[0]->height() / 100;
	return Rectf(0.f, frames_[0]->height() - h, frames_[0]->width(), h);
}

void NonPackedAnimation::blit(uint32_t time,
                              const Rectf& source_rect,
                              const Rectf& destination_rect,
                              const RGBColor* clr,
                              Surface* target) const {
	ensure_graphics_are_loaded();
	assert(target);
	const uint32_t idx = current_frame(time);
	assert(idx < nr_frames());
	if (!hasplrclrs_ || clr == nullptr) {
		target->blit(destination_rect, *frames_.at(idx), source_rect, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   destination_rect, *frames_.at(idx), *pcmasks_.at(idx), source_rect, *clr);
	}
	// TODO(GunChleoc): Stereo position would be nice.
	trigger_sound(time, 128);
}

}  // namespace

/*
==============================================================================

Common Animation functions

==============================================================================
*/

Animation::Animation(const LuaTable& table) : scale_(1.0f), frametime_(FRAME_LENGTH), play_once_(false) {
	try {
		get_point(*table.get_table("hotspot"), &hotspot_);

		if (table.has_key("scale")) {
			scale_ = table.get_double("scale");
			if (scale_ <= 0.0f) {
				// NOCOM identify the animation somehow
				throw wexception("Animation scale needs to be > 0.0f, but it is %f.",
									  static_cast<double>(scale_));
			}
		}
		assert(scale_ > 0);

		if (table.has_key("sound_effect")) {
			std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");

			const std::string name = sound_effects->get_string("name");
			const std::string directory = sound_effects->get_string("directory");
			sound_effect_ = directory + g_fs->file_separator() + name;
			g_sound_handler.load_fx_if_needed(directory, name, sound_effect_);

			if (table.has_key("play_once")) {
				play_once_ = table.get_bool("play_once");
			}
		}
	} catch (const LuaError& e) {
		throw wexception("Error in animation table: %s", e.what());
	}
}

const Vector2i& Animation::hotspot() const {
	return hotspot_;
}

uint32_t Animation::frametime() const {
	return frametime_;
}

Rectf Animation::destination_rectangle(const Vector2f& position,
                                                const Rectf& source_rect,
                                                const float scale) const {
	return Rectf(position.x - (hotspot_.x - source_rect.x / scale_) * scale,
	             position.y - (hotspot_.y - source_rect.y / scale_) * scale,
	             source_rect.w * scale / scale_, source_rect.h * scale / scale_);
}

uint32_t Animation::current_frame(uint32_t time) const {
	if (nr_frames() > 1) {
		return (play_once_ && time / frametime_ > static_cast<uint32_t>(nr_frames() - 1)) ?
		          static_cast<uint32_t>(nr_frames() - 1) :
		          time / frametime_ % nr_frames();
	}
	return 0;
}

void Animation::trigger_sound(uint32_t time, uint32_t stereo_position) const {
	if (sound_effect_.empty()) {
		return;
	}

	if (current_frame(time) == 0) {
		Notifications::publish(NoteSound(sound_effect_, stereo_position, 1));
	}
}

/*
==============================================================================

DirAnimations IMPLEMENTAION

==============================================================================
*/

DirAnimations::DirAnimations(
   uint32_t dir1, uint32_t dir2, uint32_t dir3, uint32_t dir4, uint32_t dir5, uint32_t dir6) {
	animations_[0] = dir1;
	animations_[1] = dir2;
	animations_[2] = dir3;
	animations_[3] = dir4;
	animations_[4] = dir5;
	animations_[5] = dir6;
}

/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/

uint32_t AnimationManager::load(const LuaTable& table) {
	animations_.push_back(std::unique_ptr<Animation>(new NonPackedAnimation(table)));
	return animations_.size();
}

uint32_t AnimationManager::load_packed(const std::string& name, const LuaTable& table) {
	animations_.push_back(std::unique_ptr<Animation>(new PackedAnimation(name, table)));
	return animations_.size();
}

const Animation& AnimationManager::get_animation(uint32_t id) const {
	if (!id || id > animations_.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *animations_[id - 1].get();
}

const Image* AnimationManager::get_representative_image(uint32_t id, const RGBColor* clr) {
	const auto hash = std::make_pair(id, clr);
	if (representative_images_.count(hash) != 1) {
		representative_images_.insert(std::make_pair(
		   hash, std::unique_ptr<const Image>(
		            std::move(g_gr->animations().get_animation(id).representative_image(clr)))));
	}
	return representative_images_.at(hash).get();
}
