/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/diranimations.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "graphic/surface.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"

using namespace std;

namespace  {

// Parses a point from a string like 'p=x y' into p. Throws on error.
void parse_point(const string& def, Point* p) {
	vector<string> split_vector;
	boost::split(split_vector, def, boost::is_any_of(" "));
	if (split_vector.size() != 2)
		throw wexception("Invalid point definition: %s", def.c_str());

	p->x = boost::lexical_cast<int32_t>(split_vector[0]);
	p->y = boost::lexical_cast<int32_t>(split_vector[1]);
}

// Parses a rect from a string like 'p=x y w h' into r. Throws on error.
void parse_rect(const string& def, Rect* r) {
	vector<string> split_vector;
	boost::split(split_vector, def, boost::is_any_of(" "));
	if (split_vector.size() != 4)
		throw wexception("Invalid rect definition: %s", def.c_str());

	r->x = boost::lexical_cast<int32_t>(split_vector[0]);
	r->y = boost::lexical_cast<int32_t>(split_vector[1]);
	r->w = boost::lexical_cast<uint32_t>(split_vector[2]);
	r->h = boost::lexical_cast<uint32_t>(split_vector[3]);
}

/**
 * An Image Implementation that draws a static animation into a surface.
 */
class AnimationImage : public Image {
public:
	AnimationImage
		(const string& ghash, const Animation* anim, const RGBColor& clr)
		: hash_(ghash), anim_(anim), clr_(clr)	{}
	virtual ~AnimationImage() {}

	// Implements Image.
	virtual uint16_t width() const {return anim_->width();}
	virtual uint16_t height() const {return anim_->height();}
	virtual const string& hash() const {return hash_;}
	virtual Surface* surface() const {
		SurfaceCache& surface_cache = g_gr->surfaces();
		Surface* surf = surface_cache.get(hash_);
		if (surf)
			return surf;

		// Blit the animation on a freshly wiped surface.
		surf = Surface::create(width(), height());
		surf->fill_rect(Rect(0, 0, surf->width(), surf->height()), RGBAColor(255, 255, 255, 0));
		anim_->blit(0, Point(0,0), Rect(0,0,width(), height()), &clr_, surf);
		surface_cache.insert(hash_, surf);

		return surf;
	}

private:
	const string hash_;
	const Animation* const anim_;   // Not owned.
	const RGBColor clr_;
};


/**
 * Implements the Animation interface for a packed animation, that is an animation
 * that is contained in a singular image (plus one for player color).
 */
class PackedAnimation : public Animation {
public:
	virtual ~PackedAnimation() {}
	PackedAnimation(const string& directory, Section & s);

	// Implements Animation.
	virtual uint16_t width() const {return width_;}
	virtual uint16_t height() const {return height_;}
	virtual uint16_t nr_frames() const {return nr_frames_;}
	virtual uint32_t frametime() const {return frametime_;}
	virtual const Point& hotspot() const {return hotspot_;}
	virtual const Image& representative_image(const RGBColor& clr) const;
	void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*) const;
	virtual void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const;

private:
	struct Region {
		Point target_offset;
		uint16_t w, h;
		std::vector<Point> source_offsets;  // indexed by frame nr.
	};

	uint16_t width_, height_;
	uint16_t nr_frames_;
	uint32_t frametime_;
	Point hotspot_;
	Point base_offset_;

	const Image* image_;  // Not owned
	const Image* pcmask_;  // Not owned
	std::vector<Region> regions_;
	string hash_;

	/// mapping of soundeffect name to frame number, indexed by frame number .
	map<uint32_t, string> sfx_cues;
};

PackedAnimation::PackedAnimation(const string& directory, Section& s)
		: width_(0), height_(0), nr_frames_(0), frametime_(FRAME_LENGTH), image_(NULL), pcmask_(NULL) {
	hash_ = directory + s.get_name();
	log("\nNOCOM packed animation %s %s\n", directory.c_str(), s.get_name());

	// Read mapping from frame numbers to sound effect names and load effects
	while (Section::Value * const v = s.get_next_val("sfx")) {
		char * parameters = v->get_string(), * endp;
		unsigned long long int const value = strtoull(parameters, &endp, 0);
		const uint32_t frame_number = value;
		try {
			if (endp == parameters or frame_number != value)
				throw wexception("expected %s but found \"%s\"", "frame number", parameters);
			parameters = endp;
			force_skip(parameters);
			g_sound_handler.load_fx(directory, parameters);
			map<uint32_t, string>::const_iterator const it =
				sfx_cues.find(frame_number);
			if (it != sfx_cues.end())
				throw wexception
					("redefinition for frame %u to \"%s\" (previously defined to "
					 "\"%s\")",
					 frame_number, parameters, it->second.c_str());
		} catch (const _wexception & e) {
			throw wexception("sfx: %s", e.what());
		}
		sfx_cues[frame_number] = parameters;
	}

	const int32_t fps = s.get_int("fps");
	if (fps < 0)
		throw wexception("fps is %i, must be non-negative", fps);
	if (fps > 0)
		frametime_ = 1000 / fps;

	hotspot_ = s.get_Point("hotspot");

	// Load the graphis
	string pic_fn = directory + s.get_safe_string("pics");
	image_ = g_gr->images().get(pic_fn);
	boost::replace_all(pic_fn, ".png", "");
	if (g_fs->FileExists(pic_fn + "_pc.png")) {
		pcmask_ = g_gr->images().get(pic_fn + "_pc.png");
	}

	// Parse dimensions.
	Point p;
	parse_point(s.get_string("dimensions"), &p);
	width_ = p.x;
	height_ = p.y;

	// Parse base_offset.
	parse_point(s.get_string("base_offset"), &base_offset_);

	// Parse regions
	NumberGlob glob("region_??");
	string region_name;
	while (glob.next(&region_name)) {
		string value=s.get_string(region_name.c_str(), "");
		if (value.empty())
			break;

		boost::trim(value);
		vector<string> split_vector;
		boost::split(split_vector, value, boost::is_any_of(":"));
		if (split_vector.size() != 2)
			throw wexception("%s: line is ill formatted. Should be <rect>:<offsets>", region_name.c_str());

		vector<string> offset_strings;
		boost::split(offset_strings, split_vector[1], boost::is_any_of(";"));
		if (nr_frames_ && nr_frames_ != offset_strings.size())
			throw wexception
				("%s: region has different number of frames than previous (%i != %"PRIuS").",
				 region_name.c_str(), nr_frames_, offset_strings.size());
		nr_frames_ = offset_strings.size();

		Rect region_rect;
		parse_rect(split_vector[0], &region_rect);

		Region r;
		r.target_offset.x = region_rect.x;
		r.target_offset.y = region_rect.y;
		r.w = region_rect.w;
		r.h = region_rect.h;

		BOOST_FOREACH(const string& offset_string, offset_strings) {
			parse_point(offset_string, &p);
			r.source_offsets.push_back(p);
		}
		regions_.push_back(r);
	}

	if (!regions_.size())  // No regions? Only one frame then.
		nr_frames_ = 1;

	// NOCOM Write to new file
	// 		std::unique_ptr<FileSystem> out_filesystem = initialize(output_path);
	//LayeredFileSystem* fs = new LayeredFileSystem();
	//fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));

	//FileSystem* out_filesystem(&FileSystem::create("tribes"));
	//boost::scoped_ptr<FileSystem> fs
	//		(g_fs->CreateSubFileSystem("NOCOM", FileSystem::DIR));
/*
	std::string complete_filename = "tribes";
	complete_filename            += "/";
	complete_filename            += "NOCOM";
	g_fs->EnsureDirectoryExists(complete_filename);
	boost::scoped_ptr<FileSystem> fs
			(g_fs->CreateSubFileSystem(complete_filename, FileSystem::DIR));

	FileWrite fw;
	fw.CString("test");
	*/
	//fw.Write(*fs, "NOCOM");
	//fw.Flush();
	//fw.Clear();

	BOOST_FOREACH(const Region& region, regions_) {
		log("  NOCOM %d %d - ", region.target_offset.x, region.target_offset.y);
		log("NOCOM %d %d\n", region.w, region.h);
		BOOST_FOREACH(const Point& source_offset, region.source_offsets) {
			log("  NOCOM %d %d\n", source_offset.x, source_offset.y);
		}
	}
	/*
	 * 	struct Region {
		Point target_offset;
		uint16_t w, h;
		std::vector<Point> source_offsets;  // indexed by frame nr.
	};
	*/
}

void PackedAnimation::trigger_soundfx
	(uint32_t time, uint32_t stereo_position) const {
	const uint32_t framenumber = time / frametime_ % nr_frames();
	const map<uint32_t, string>::const_iterator sfx_cue = sfx_cues.find(framenumber);
	if (sfx_cue != sfx_cues.end())
		g_sound_handler.play_fx(sfx_cue->second, stereo_position, 1);
}

const Image& PackedAnimation::representative_image(const RGBColor& clr) const {
	const string hash =
		(boost::format("%s:%02x%02x%02x:animation_pic") % hash_ % static_cast<int>(clr.r) %
		 static_cast<int>(clr.g) % static_cast<int>(clr.b))
			.str();

	ImageCache& image_cache = g_gr->images();
	if (image_cache.has(hash))
		return *image_cache.get(hash);

	return *image_cache.insert(new AnimationImage(hash, this, clr));
}

void PackedAnimation::blit
	(uint32_t time, const Point& dst, const Rect& srcrc, const RGBColor* clr, Surface* target) const
{
	assert(target);
	const uint32_t framenumber = time / frametime_ % nr_frames();

	const Image* use_image = image_;
	if (clr && pcmask_) {
		use_image = ImageTransformations::player_colored(*clr, image_, pcmask_);
	}

	target->blit
		(dst, use_image->surface(), Rect(base_offset_.x + srcrc.x, base_offset_.y + srcrc.y, srcrc.w, srcrc.h));

	for (const Region& r : regions_) {
		Rect rsrc = Rect(r.source_offsets[framenumber], r.w, r.h);
		Point rdst = dst + r.target_offset - srcrc;

		if (srcrc.x > r.target_offset.x) {
			rdst.x += srcrc.x - r.target_offset.x;
			rsrc.x += srcrc.x - r.target_offset.x;
			rsrc.w -= srcrc.x - r.target_offset.x;
			if (rsrc.w > r.w)
				continue;
		}
		if (srcrc.y > r.target_offset.y) {
			rdst.y += srcrc.y - r.target_offset.y;
			rsrc.y += srcrc.y - r.target_offset.y;
			rsrc.h -= srcrc.y - r.target_offset.y;
			if (rsrc.h > r.h)
				continue;
		}
		if (r.target_offset.x + rsrc.w > srcrc.x + srcrc.w) {
			rsrc.w = srcrc.x + srcrc.w - r.target_offset.x;
		}
		if (r.target_offset.y + rsrc.h > srcrc.y + srcrc.h) {
			rsrc.h = srcrc.y + srcrc.h - r.target_offset.y;
		}

		target->blit(rdst, use_image->surface(), rsrc);
	}

}

// Parses an array { 12, 23 } into a point.
void get_point(const LuaTable& table, Vector2i* p) {
	std::vector<int> pts = table.array_entries<int>();
	if (pts.size() != 2) {
		throw wexception("Expected 2 entries, but got %" PRIuS ".", pts.size());
	}
	p->x = pts[0];
	p->y = pts[1];
}

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	virtual ~NonPackedAnimation() {
	}
	NonPackedAnimation(const LuaTable& table);

	// Implements Animation.
	uint16_t width() const override;
	uint16_t height() const override;
	uint16_t nr_frames() const override;
	uint32_t frametime() const override;
	const Vector2i& hotspot() const override;
	Image* representative_image(const RGBColor* clr) const override;
	const std::string& representative_image_filename() const override;
	virtual void blit(uint32_t time,
	                  const Rectf& dstrc,
	                  const Rectf& srcrc,
	                  const RGBColor* clr,
	                  Surface*) const override;
	void trigger_sound(uint32_t framenumber, uint32_t stereo_position) const override;

private:
	// Loads the graphics if they are not yet loaded.
	void ensure_graphics_are_loaded() const;

	// Load the needed graphics from disk.
	void load_graphics();

	uint32_t current_frame(uint32_t time) const;

	uint32_t frametime_;
	Vector2i hotspot_;
	bool hasplrclrs_;
	std::vector<std::string> image_files_;
	std::vector<std::string> pc_mask_image_files_;

	vector<const Image*> frames_;
	vector<const Image*> pcmasks_;

	// name of sound effect that will be played at frame 0.
	// TODO(sirver): this should be done using play_sound in a program instead of
	// binding it to the animation.
	string sound_effect_;
	bool play_once_;
};

NonPackedAnimation::NonPackedAnimation(const LuaTable& table)
   : frametime_(FRAME_LENGTH), hasplrclrs_(false), play_once_(false) {
	try {
		get_point(*table.get_table("hotspot"), &hotspot_);

		if (table.has_key("sound_effect")) {
			std::unique_ptr<LuaTable> sound_effects = table.get_table("sound_effect");

			const std::string name = sound_effects->get_string("name");
			const std::string directory = sound_effects->get_string("directory");
			sound_effect_ = directory + g_fs->file_separator() + name;
			g_sound_handler.load_fx_if_needed(directory, name, sound_effect_);
		}

		if (table.has_key("play_once")) {
			play_once_ = table.get_bool("play_once");
		}

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
			boost::replace_all(image_file, ".png", "_pc.png");
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

uint16_t NonPackedAnimation::width() const {
	ensure_graphics_are_loaded();
	return frames_[0]->width();
}

uint16_t NonPackedAnimation::height() const {
	ensure_graphics_are_loaded();
	return frames_[0]->height();
}

uint16_t NonPackedAnimation::nr_frames() const {
	ensure_graphics_are_loaded();
	return frames_.size();
}

uint32_t NonPackedAnimation::frametime() const {
	return frametime_;
}

const Vector2i& NonPackedAnimation::hotspot() const {
	return hotspot_;
}

Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	assert(!image_files_.empty());
	const Image* image = g_gr->images().get(image_files_[0]);

	if (!hasplrclrs_ || clr == nullptr) {
		// No player color means we simply want an exact copy of the original image.
		const int w = image->width();
		const int h = image->height();
		Texture* rv = new Texture(w, h);
		rv->blit(Rectf(0, 0, w, h), *image, Rectf(0, 0, w, h), 1., BlendMode::Copy);
		return rv;
	} else {
		return playercolor_image(clr, image, g_gr->images().get(pc_mask_image_files_[0]));
	}
}

const std::string& NonPackedAnimation::representative_image_filename() const {
	return image_files_[0];
}

uint32_t NonPackedAnimation::current_frame(uint32_t time) const {
	if (nr_frames() > 1) {
		return (play_once_ && time / frametime_ > static_cast<uint32_t>(nr_frames() - 1)) ?
		          static_cast<uint32_t>(nr_frames() - 1) :
		          time / frametime_ % nr_frames();
	}
	return 0;
}

void NonPackedAnimation::trigger_sound(uint32_t time, uint32_t stereo_position) const {
	if (sound_effect_.empty()) {
		return;
	}

	const uint32_t framenumber = current_frame(time);

	if (framenumber == 0) {
		g_sound_handler.play_fx(sound_effect_, stereo_position, 1);
	}
}

void NonPackedAnimation::blit(uint32_t time,
                              const Rectf& dstrc,
                              const Rectf& srcrc,
                              const RGBColor* clr,
                              Surface* target) const {
	assert(target);

	const uint32_t idx = current_frame(time);
	assert(idx < nr_frames());

	if (!hasplrclrs_ || clr == nullptr) {
		target->blit(dstrc, *frames_.at(idx), srcrc, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(dstrc, *frames_.at(idx), *pcmasks_.at(idx), srcrc, *clr);
	}
}

}  // namespace

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

const Animation& AnimationManager::get_animation(uint32_t id) const {
	if (!id || id > animations_.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *animations_[id - 1].get();
}

const Image* AnimationManager::get_representative_image(uint32_t id, const RGBColor* clr) {
	if (representative_images_.count(id) != 1) {
		representative_images_.insert(std::make_pair(
		   id,
		   std::unique_ptr<Image>(g_gr->animations().get_animation(id).representative_image(clr))));
	}
	return representative_images_.at(id).get();
}
