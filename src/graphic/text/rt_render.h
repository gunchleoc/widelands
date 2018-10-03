/*
 * Copyright (C) 2006-2018 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_RT_RENDER_H
#define WL_GRAPHIC_TEXT_RT_RENDER_H

#include <memory>
#include <set>
#include <string>

#include <stdint.h>

#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/text/font_set.h"
#include "graphic/text/rendered_text.h"
#include "graphic/text/rt_parse.h"
#include "graphic/text/texture_cache.h"

namespace RT {

class FontCache;
class Parser;
class RenderNode;

struct RendererStyle {
	RendererStyle(const std::string& font_face_,
	              uint8_t font_size_,
	              uint16_t remaining_width_,
	              uint16_t overall_width_)
	   : font_face(font_face_),
	     font_size(font_size_),
	     remaining_width(remaining_width_),
	     overall_width(overall_width_) {
	}

	std::string font_face;
	uint8_t font_size;
	uint16_t remaining_width;
	uint16_t overall_width;
};

/**
 * This is the rendering engine. The returned images are not owned by the
 * caller.
 */
using TagSet = std::set<std::string>;
class Renderer {
public:
	// Ownership is not taken.
	Renderer(ImageCache* image_cache, TextureCache* texture_cache, const UI::FontSets& fontsets);
	~Renderer();

	// Render the given string in the given width. Restricts the allowed tags to
	// the ones in TagSet.
	std::shared_ptr<const UI::RenderedText>
	render(const std::string&, uint16_t width, const TagSet& tagset = TagSet());

private:
	std::shared_ptr<RenderNode>
	layout(const std::string& text, uint16_t width, const TagSet& allowed_tags);

	std::unique_ptr<FontCache> font_cache_;
	std::unique_ptr<Parser> parser_;
	ImageCache* const image_cache_;      // Not owned.
	TextureCache* const texture_cache_;  // Not owned.
	const UI::FontSets& fontsets_;       // All fontsets
	RendererStyle renderer_style_;       // Properties that all render nodes need to know about
};

struct NodeStyle {
	UI::FontSet const* fontset;
	std::string font_face;
	uint16_t font_size;
	RGBColor font_color;
	int font_style;

	uint8_t spacing;
	UI::Align halign;
	UI::Align valign;
	std::string reference;
};


enum class TagType {
	kText,
	kBr,
	kDiv,
	kRt,
	kFont,
	kHspace,
	kImg,
	kP,
	kVspace
};

class TagHandler {
public:
	TagHandler(Tag& tag,
	           FontCache& fc,
	           NodeStyle ns,
	           ImageCache* image_cache,
	           RendererStyle& renderer_style,
	           const UI::FontSets& fontsets)
	   : tag_(tag),
	     font_cache_(fc),
	     nodestyle_(ns),
	     image_cache_(image_cache),
	     renderer_style_(renderer_style),
	     fontsets_(fontsets) {
	}

	virtual TagType type() const = 0;

	virtual ~TagHandler() {
	}

	virtual void enter() {
	}
	virtual void emit_nodes(std::vector<std::shared_ptr<RenderNode>>&);

private:
	void make_text_nodes(const std::string& txt,
	                     std::vector<std::shared_ptr<RenderNode>>& nodes,
	                     NodeStyle& ns);

protected:
	Tag& tag_;
	FontCache& font_cache_;
	NodeStyle nodestyle_;
	ImageCache* image_cache_;        // Not owned
	RendererStyle& renderer_style_;  // Reference to global renderer style in the renderer
	const UI::FontSets& fontsets_;
};

} // namespace RT

#endif  // end of include guard: WL_GRAPHIC_TEXT_RT_RENDER_H
