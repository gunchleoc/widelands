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

#include "graphic/text_layout.h"

#include <map>

#include <SDL_ttf.h>
#include <boost/format.hpp>

#include "base/utf8.h"
#include "graphic/font_handler1.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"

std::string as_game_tip(const std::string& txt) {
	static boost::format f
		("<rt padding_l=48 padding_t=28 padding_r=48 padding_b=28>"
		 "<p align=center><font color=21211b face=serif size=16>%s</font></p></rt>");

	f % txt;
	return f.str();
}

std::string as_window_title(const std::string& txt) {
	static boost::format f("<rt><p><font face=serif size=13 bold=1 color=%s>%s</font></p></rt>");

	f % UI_FONT_CLR_FG.hex_value();
	f % txt;
	return f.str();
}
std::string as_uifont(const std::string & txt, int size, const RGBColor& clr) {
	// UI Text is always bold due to historic reasons
	static boost::format
			f("<rt><p><font face=serif size=%i bold=1 shadow=1 color=%s>%s</font></p></rt>");

	f % size;
	f % clr.hex_value();
	f % txt;
	return f.str();
}

std::string as_tooltip(const std::string & txt) {
	static boost::format f("<rt><p><font face=serif size=%i bold=1 color=%s>%s</font></p></rt>");

	f % UI_FONT_SIZE_SMALL;
	f % UI_FONT_TOOLTIP_CLR.hex_value();
	f % txt;
	return f.str();
}

std::string as_waresinfo(const std::string & txt) {
	static boost::format f
		("<rt><p><font face=condensed size=10 bold=0 color=%s>%s</font></p></rt>");
	f % UI_FONT_TOOLTIP_CLR.hex_value();
	f % txt;
	return f.str();
}


namespace UI {


/**
 * Prepare the TTF style settings for rendering in this style.
 */
void TextStyle::setup() const
{
	int32_t font_style = TTF_STYLE_NORMAL;
	if (bold)
		font_style |= TTF_STYLE_BOLD;
	if (italics)
		font_style |= TTF_STYLE_ITALIC;
	if (underline)
		font_style |= TTF_STYLE_UNDERLINE;
	TTF_SetFontStyle(font->get_ttf_font(), font_style);
}

/**
 * Compute the bare width (without caret padding) of the given string.
 */
uint32_t TextStyle::calc_bare_width(const std::string & text) const
{
	int w, h;
	setup();

	TTF_SizeUTF8(font->get_ttf_font(), text.c_str(), &w, &h);
	return w;
}

/**
 * \note Please only use this function once you understand the definitions
 * of ascent/descent etc.
 *
 * Computes the actual line height we should use for rendering the given text.
 * This is heuristic, because it pre-initializes the miny and maxy values to
 * the ones that are typical for Latin scripts, so that lineskips should always
 * be the same for such scripts.
 */
void TextStyle::calc_bare_height_heuristic(const std::string & text, int32_t & miny, int32_t & maxy) const
{
	miny = font->m_computed_typical_miny;
	maxy = font->m_computed_typical_maxy;

	setup();
	std::string::size_type pos = 0;
	while (pos < text.size()) {
		uint16_t ch = Utf8::utf8_to_unicode(text, pos);
		int32_t glyphminy, glyphmaxy;
		TTF_GlyphMetrics(font->get_ttf_font(), ch, nullptr, nullptr, &glyphminy, &glyphmaxy, nullptr);
		miny = std::min(miny, glyphminy);
		maxy = std::max(maxy, glyphmaxy);
	}
}


/*
=============================

Default styles

=============================
*/


const TextStyle & TextStyle::ui_big()
{
	static TextStyle style;

	style.font = Font::get(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_BIG);
	style.fg = UI_FONT_CLR_FG;
	style.bold = true;

	return style;
}

const TextStyle & TextStyle::ui_small()
{
	static TextStyle style;

	style.font = Font::get(UI::g_fh1->fontset().serif(), UI_FONT_SIZE_SMALL);
	style.fg = UI_FONT_CLR_FG;
	style.bold = true;

	return style;
}

} // namespace UI
