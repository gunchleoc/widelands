/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_LAYOUT_H
#define WL_GRAPHIC_TEXT_LAYOUT_H

#include <string>
#include <unicode/uchar.h>

#include "graphic/align.h"
#include "graphic/font.h"
#include "graphic/color.h"
#include "graphic/text_constants.h"

/**
 * This function replaces some HTML entities in strings, e.g. %nbsp;.
 * It is used by the renderers after the tags have been parsed.
 */
void replace_entities(std::string* text);

/**
  * Returns the exact width of the text rendered as editorfont for the given font size.
  * This function is inefficient; only call when we need the exact width.
  */

uint32_t text_width(const std::string& text, int ptsize);

/**
  * Returns the exact height of the text rendered as editorfont for the given font size.
  * This function is inefficient; only call when we need the exact height.
  */
uint32_t text_height(const std::string& text, int ptsize);

/**
 * Checks it the given string is RichText or not. Does not do validity checking.
 */
inline bool is_richtext(const std::string& text) {
	return text.compare(0, 3, "<rt") == 0;
}

/**
 * Escapes reserved characters for Richtext.
 */
std::string richtext_escape(const std::string& given_text);

/**
 * Convenience functions to convert simple text into a valid block
 * of rich text which can be rendered.
 */
std::string as_uifont
	(const std::string&, int ptsize = UI_FONT_SIZE_SMALL, const RGBColor& clr = UI_FONT_CLR_FG);

std::string as_editorfont(const std::string& text, int ptsize = UI_FONT_SIZE_SMALL,
								  const RGBColor& clr = UI_FONT_CLR_FG);

std::string as_aligned(const std::string & txt, UI::Align align, int ptsize = UI_FONT_SIZE_SMALL,
							  const RGBColor& clr = UI_FONT_CLR_FG);

std::string as_tooltip(const std::string&);
std::string as_waresinfo(const std::string&);
std::string as_window_title(const std::string&);
std::string as_game_tip(const std::string&);

namespace UI {

/**
 * Text style combines font with other characteristics like color
 * and style (italics, bold).
 */
// TODO(GunChleoc): This struct will disappear with the old font handler
struct TextStyle {
	TextStyle() :
		font(nullptr),
		fg(255, 255, 255),
		bold(false),
		italics(false),
		underline(false)
	{}

	static TextStyle makebold(Font * font, RGBColor fg) {
		TextStyle ts;
		ts.font = font;
		ts.bold = true;
		ts.fg = fg;
		return ts;
	}

	static const TextStyle & ui_big();
	static const TextStyle & ui_small();
	uint32_t calc_bare_width(const std::string & text) const;
	uint32_t calc_width_for_wrapping(const UChar& c) const;
	uint32_t calc_width_for_wrapping(const std::string & text) const;
	void calc_bare_height_heuristic(const std::string & text, int32_t & miny, int32_t & maxy) const;
	void setup() const;

	Font * font;
	RGBColor fg;
	bool bold : 1;
	bool italics : 1;
	bool underline : 1;

	bool operator== (const TextStyle & o) const {
		return
			font == o.font && fg == o.fg &&
			bold == o.bold && italics == o.italics && underline == o.underline;
	}
	bool operator!= (const TextStyle & o) const {
		return !(*this == o);
	}
};

} // namespace UI

#endif  // end of include guard: WL_GRAPHIC_TEXT_LAYOUT_H
