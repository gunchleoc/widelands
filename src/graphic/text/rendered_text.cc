/*
 * Copyright (C) 2017-2018 by the Widelands Development Team
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

#include "graphic/text/rendered_text.h"

#include <memory>

#include "base/macros.h"
#include "graphic/graphic.h"
#include "graphic/text/bidi.h"
#include "graphic/text_layout.h"

namespace UI {
// RenderedRect
RenderedRect::RenderedRect(const Recti& init_rect,
                           std::shared_ptr<const Image> init_image,
                           bool visited,
                           const RGBColor& color,
                           bool is_background_color_set,
                           DrawMode init_mode, bool advances_caret,
						   const std::string& text, const RT::IFont* font)
   : rect_(init_rect),
     transient_image_(init_image),
     permanent_image_(nullptr),
     visited_(visited),
     background_color_(color),
     is_background_color_set_(is_background_color_set),
     mode_(init_mode),
	 advances_caret_(advances_caret),
	 text_(text),
	 font_(font) {
}
RenderedRect::RenderedRect(const Recti& init_rect,
                           const Image* init_image,
                           bool visited,
                           const RGBColor& color,
                           bool is_background_color_set,
                           DrawMode init_mode)
   : rect_(init_rect),
     transient_image_(nullptr),
     permanent_image_(init_image),
     visited_(visited),
     background_color_(color),
     is_background_color_set_(is_background_color_set),
     mode_(init_mode),
	 advances_caret_(false),
	 text_(""),
	 font_(nullptr) {
}

RenderedRect::RenderedRect(const Recti& init_rect, const Image* init_image)
   : RenderedRect(init_rect, init_image, false, RGBColor(0, 0, 0), false, DrawMode::kTile) {
}
RenderedRect::RenderedRect(const Recti& init_rect, const RGBColor& color)
   : RenderedRect(init_rect, nullptr, false, color, true, DrawMode::kTile) {
}
RenderedRect::RenderedRect(std::shared_ptr<const Image> init_image, bool advances_caret, const std::string& text, const RT::IFont* font)
   : RenderedRect(Recti(0, 0, init_image->width(), init_image->height()),
                  init_image,
                  false,
                  RGBColor(0, 0, 0),
                  false,
                  DrawMode::kBlit,
				  advances_caret,
				  text,
				  font) {
}
RenderedRect::RenderedRect(const Image* init_image)
   : RenderedRect(Recti(0, 0, init_image->width(), init_image->height()),
                  init_image,
                  false,
                  RGBColor(0, 0, 0),
                  false,
                  DrawMode::kBlit) {
}

const Image* RenderedRect::image() const {
	assert(permanent_image_ == nullptr || transient_image_ == nullptr);
	return permanent_image_ == nullptr ? transient_image_.get() : permanent_image_;
}

int RenderedRect::x() const {
	return rect_.x;
}

int RenderedRect::y() const {
	return rect_.y;
}

int RenderedRect::width() const {
	return rect_.w;
}
int RenderedRect::height() const {
	return rect_.h;
}

void RenderedRect::set_origin(const Vector2i& new_origin) {
	rect_.x = new_origin.x;
	rect_.y = new_origin.y;
}
void RenderedRect::set_visited() {
	visited_ = true;
}
bool RenderedRect::was_visited() const {
	return visited_;
}

bool RenderedRect::has_background_color() const {
	return is_background_color_set_;
}
const RGBColor& RenderedRect::background_color() const {
	return background_color_;
}

RenderedRect::DrawMode RenderedRect::mode() const {
	return mode_;
}

const std::string RenderedRect::text() const {
	return text_;
}

const RT::IFont* RenderedRect::font() const {
	return font_;
}

bool RenderedRect::advances_caret() const {
	return advances_caret_;
}

// RenderedText
int RenderedText::width() const {
	int result = 0;
	for (const auto& rect : rects) {
		result = std::max(result, rect->x() + rect->width());
	}
	return result;
}
int RenderedText::height() const {
	int result = 0;
	for (const auto& rect : rects) {
		result = std::max(result, rect->y() + rect->height());
	}
	return result;
}

void RenderedText::draw(RenderTarget& dst,
                        const Vector2i& position,
                        const Recti& region,
                        Align align,
                        CropMode cropmode) const {

	// Un-const the position and adjust for alignment according to region width
	Vector2i aligned_pos(position.x, position.y);

	// For cropping images that don't fit
	int offset_x = 0;
	if (cropmode == CropMode::kSelf) {
		// NOCOM dependency on text_layout - move the function to "align"
		UI::correct_for_align(align, width(), &aligned_pos);
		if (align != UI::Align::kLeft) {
			for (const auto& rect : rects) {
				offset_x = std::min(region.w - rect->width(), offset_x);
			}
			if (align == UI::Align::kCenter) {
				offset_x /= 2;
			}
		}
	} else {
		aligned_pos.x -= region.x;
		aligned_pos.y -= region.y;
		UI::correct_for_align(align, region.w, &aligned_pos);
	}

	// Blit the rects
	for (const auto& rect : rects) {
		blit_rect(dst, offset_x, aligned_pos, *rect, region, align, cropmode);
	}
}

/// Calculate the caret position for letter number caretpos
Vector2i RenderedText::handle_caret(int caret_index, RenderTarget* dst) const {
	log("NOCOM caret wanted at: %d\n", caret_index);
	// TODO(GunChleoc): Arabic: Fix caret position for BIDI text.
	Vector2i result = Vector2i::zero();
	int counter = 0; // NOCOM for testing only
	for (const auto& rect : rects) {
		if (!rect->advances_caret()) {
			// This is not a text or newline node NOCOM document
			continue;
		}
		if (rect->text().empty()) {
			log("NOCOM newline\n");
			--caret_index;
		}
		log("NOCOM text: '%s'\n", rect->text().c_str());

		const int text_size = rect->text().size();
		log("NOCOM rect %d: %d %d %d %d, text_size: %d\n", ++counter, rect->x(), rect->y(), rect->width(), rect->height(), text_size);
		if (caret_index > text_size) {
			caret_index -= text_size;
			// Make sure we don't get smaller than the start of the text
			caret_index = std::max(caret_index, 0);
			log("NOCOM new caret pos: %d\n", caret_index);
		} else {
			// Make sure we don't get bigger than the end of the text
			caret_index = std::max(caret_index, 0);
			caret_index = std::min(caret_index, text_size);
			// Blit caret here
			log("NOCOM blitting caret at pos %d. ", caret_index);
			const std::string line_to_caret = rect->text().substr(0, caret_index);
			const int caret_offset_x = (!rect->text().empty()) ? rect->font()->text_width(line_to_caret) : 0;
			log("NOCOM line to caret: %s, width = %d\n", line_to_caret.c_str(), caret_offset_x);
			result = Vector2i(rect->x() + caret_offset_x, rect->y());
			log("NOCOM caretpt: %d, %d\n", result.x, result.y);
			if (dst) {
				// NOCOM caret pos is broken for editbox, fine for multilineeditbox
				if (rect->text().empty()) {
					result.y += rect->height();
				}
				// NOCOM hard-coded color
				dst->fill_rect(Recti(result.x, result.y, 1, rect->font()->lineskip()), RGBAColor(255, 255, 255, 0));
			}
			// Don't calculate it twice
			return result;
		}
	}
	return result;
}
/// returns -1 if skipping forward fails - this needs to be handled by caller
int RenderedText::shift_caret(int caret_index, LineSkip lineskip) const {
	// NOCOM fix this function
	int result = 0;
	log("NOCOM %lu rects, caret is %d\n", rects.size(), caret_index);
	// NOCOM empty lines don't work
	int last_y = -1;
	int current_wanted_ypos = -1;
	int previous_ypos = 0;
	int current_wanted_xpos = -1;

	// Find out where we want to go
	for (const auto& rect : rects) {
		if (rect->font() == nullptr && rect->text().empty()) {
			// This is not a text or newline node NOCOM document
			continue;
		}
		const int y = rect->y();
		if (y != last_y) {
			previous_ypos = last_y;
		}

		const int text_size = rect->text().size();
		if (caret_index > text_size) {
			caret_index -= text_size;
			// Make sure we don't get smaller than the start of the text
			caret_index = std::max(caret_index, 0);
		} else {
			// Make sure we don't get bigger than the end of the text
			current_wanted_xpos = std::min(caret_index, text_size);
			current_wanted_ypos = rect->y();
			break;
		}
	}

	if (current_wanted_xpos == -1) {
		// We did not find anything
		return 0;
	}

	assert(current_wanted_ypos != -1);

	// Skip to start or end of line
	if (lineskip == LineSkip::kStartOfLine || lineskip == LineSkip::kEndOfLine) {
		for (const auto& rect : rects) {
			if (rect->y() < current_wanted_ypos) {
				result += rect->text().size();
			} else if (rect->y() == current_wanted_ypos) {
				if (lineskip == LineSkip::kStartOfLine) {
					return result;
				} else {
					result += rect->text().size();
				}
			} else {
				return result;
			}
		}
		// In case we didn't find anything
		if (lineskip == LineSkip::kStartOfLine) {
			return 0;
		} else {
			return -1;
		}
	}

	// Skip line
	if (lineskip == LineSkip::kLineForward) {
		for (const auto& rect : rects) {
			if (rect->y() <= current_wanted_ypos) {
				result += rect->text().size();
			} else {
				return result + current_wanted_xpos;
			}
		}
		// We didn't find anything
		return -1;
	} else if (lineskip == LineSkip::kLineBack) {
		for (const auto& rect : rects) {
			if (rect->y() <= previous_ypos) {
				result += rect->text().size();
			} else {
				return result + current_wanted_xpos;
			}
		}
		// We didn't find anything
		return 0;
	}
	NEVER_HERE();
}

void RenderedText::blit_rect(RenderTarget& dst,
                             int offset_x,
                             const Vector2i& aligned_position,
                             const RenderedRect& rect,
                             const Recti& region,
                             Align align,
                             CropMode cropmode) const {
	const Vector2i blit_point(aligned_position.x + rect.x(), aligned_position.y + rect.y());

	// Draw Solid background Color
	if (rect.has_background_color()) {
		const int maximum_size = g_gr->max_texture_size_for_font_rendering();
		const int tile_width = std::min(maximum_size, rect.width());
		const int tile_height = std::min(maximum_size, rect.height());
		for (int tile_x = blit_point.x; tile_x + tile_width <= blit_point.x + rect.width();
		     tile_x += tile_width) {
			for (int tile_y = blit_point.y; tile_y + tile_height <= blit_point.y + rect.height();
			     tile_y += tile_height) {
				dst.fill_rect(Recti(tile_x, tile_y, tile_width, tile_height), rect.background_color());
			}
		}
	}

	if (rect.image() != nullptr) {
		switch (rect.mode()) {
		// Draw a foreground texture
		case RenderedRect::DrawMode::kBlit: {
			switch (cropmode) {
			case CropMode::kRenderTarget:
				// dst will handle any cropping
				dst.blit(blit_point, rect.image());
				break;
			case CropMode::kSelf:
				blit_cropped(dst, offset_x, aligned_position, blit_point, rect, region, align);
			}
		} break;
		// Draw a background image (tiling)
		case RenderedRect::DrawMode::kTile:
			dst.tile(Recti(blit_point, rect.width(), rect.height()), rect.image(), Vector2i::zero());
			break;
		}
	}
}

void RenderedText::draw(RenderTarget& dst, const Vector2i& position, UI::Align align) const {
	draw(dst, position, Recti(0, 0, width(), height()), align);
}

// Crop horizontally if it doesn't fit
void RenderedText::blit_cropped(RenderTarget& dst,
                                int offset_x,
                                const Vector2i& position,
                                const Vector2i& blit_point,
                                const RenderedRect& rect,
                                const Recti& region,
                                Align align) const {

	int blit_width = rect.width();
	int cropped_left = 0;
	if (align != UI::Align::kLeft) {
		if (rect.x() + rect.width() + offset_x <= region.x) {
			// Falls off the left-hand side
			return;
		}
		if (rect.x() + offset_x < 0) {
			// Needs cropping
			blit_width = rect.width() + offset_x + rect.x() - region.x;
			cropped_left = rect.width() - blit_width;
		}
	}

	if (align != UI::Align::kRight) {
		if (rect.x() + rect.width() - offset_x > region.w - region.x) {
			blit_width = region.w - rect.x() - offset_x;
		}
	}

	// Don't blit tiny or negative width
	if (blit_width < 3) {
		return;
	}

	dst.blitrect(
	   Vector2i(cropped_left > 0 ?
	               position.x + region.x - (align == UI::Align::kRight ? region.w : region.w / 2) :
	               blit_point.x,
	            blit_point.y),
	   rect.image(), Recti(cropped_left > 0 ? cropped_left : 0, region.y, blit_width, region.h));
}

}  // namespace UI
