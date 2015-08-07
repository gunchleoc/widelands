/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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

#include "ui_basic/button.h"

#include <boost/format.hpp>
#include <SDL_keyboard.h>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication.h" // NOCOM Back reference to Widelands. We need to find a better place to put this. Graphic?

namespace UI {

// Margin around image. The image will be scaled down to fit into this rectangle with preserving size.
constexpr int kButtonImageMargin = 2;

Button::Button //  for textual buttons
	(Panel * const parent,
	 const std::string & name,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 const Image* bg_pic,
	 const std::string & title_text,
	 const std::string & tooltip_text,
	 bool const _enabled, bool const flat)
	:
	NamedPanel           (parent, name, x, y, w, h, tooltip_text),
	m_highlighted   (false),
	m_pressed       (false),
	m_permpressed   (false),
	m_enabled       (_enabled),
	m_repeating     (false),
	m_flat          (flat),
	m_draw_flat_background(false),
	m_time_nextact  (0),
	m_title         (title_text),
	hotkey_scope_   (""),
	hotkey_code_    (SDLK_UNKNOWN),
	pressed_hotkey_code_(SDLK_UNKNOWN),
	normal_tooltip_(tooltip_text),
	pressed_tooltip_(tooltip_text),
	m_pic_background(bg_pic),
	m_pic_custom    (nullptr),
	m_textstyle(UI::TextStyle::ui_small()),
	m_clr_down      (229, 161, 2),
	m_draw_caret    (false)
{
	set_thinks(false);
	set_tooltip(tooltip_text);
}

Button::Button //  for pictorial buttons
	(Panel * const parent,
	 const std::string & name,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const Image* bg_pic,
	 const Image* fg_pic,
	 const std::string & tooltip_text,
	 bool const _enabled, bool const flat)
	:
	NamedPanel      (parent, name, x, y, w, h, tooltip_text),
	m_highlighted   (false),
	m_pressed       (false),
	m_permpressed   (false),
	m_enabled       (_enabled),
	m_repeating     (false),
	m_flat          (flat),
	m_draw_flat_background(false),
	m_time_nextact  (0),
	hotkey_scope_   (""),
	hotkey_code_    (SDLK_UNKNOWN),
	pressed_hotkey_code_(SDLK_UNKNOWN),
	normal_tooltip_(tooltip_text),
	pressed_tooltip_(tooltip_text),
	m_pic_background(bg_pic),
	m_pic_custom    (fg_pic),
	m_textstyle(UI::TextStyle::ui_small()),
	m_clr_down      (229, 161, 2),
	m_draw_caret    (false)
{
	set_thinks(false);
	set_tooltip(tooltip_text);
}


Button::~Button()
{
}


/**
 * Sets a new picture for the Button.
*/
void Button::set_pic(const Image* pic)
{
	m_title.clear();

	if (m_pic_custom == pic)
		return;

	m_pic_custom = pic;

	update();
}


/**
 * Set a text title for the Button
*/
void Button::set_title(const std::string & title) {
	if (m_title == title)
		return;

	m_pic_custom = nullptr;
	m_title      = title;

	update();
}

/**
 * NOCOM set hotkey scope in constructor. Doing it here now to keep diff small.
 * The scope might even need to be defined all the way up in Panel - let's see how this goes.
 *
 * If the hotkey already exists in the global table, the value from the
 * global table is used instead.
*/
void Button::set_hotkey(const std::string& scope, const SDL_Keycode& code, const SDL_Keycode& pressed_code) {
	if (pressed_code == SDLK_UNKNOWN) {
		pressed_hotkey_code_ =
				WLApplication::get()->hotkeys()->add_hotkey(scope, get_name(), code, m_title.empty() ? normal_tooltip_ : m_title);
	} else {
		pressed_hotkey_code_ =
				WLApplication::get()->hotkeys()->add_hotkey(scope, get_name(), pressed_code, m_title.empty() ? pressed_tooltip_ : m_title);
	}
	hotkey_code_ = WLApplication::get()->hotkeys()->add_hotkey(scope, get_name(), code, m_title.empty() ? normal_tooltip_ : m_title);
	hotkey_scope_ = scope;
	update_tooltip();
	update();
}

const SDL_Keycode& Button::get_hotkey() {
	if (m_permpressed) {
		return pressed_hotkey_code_;
	} else {
		return hotkey_code_;
	}
}

void Button::set_tooltip(const std::string& text) {
	normal_tooltip_ = text;
	update_tooltip();
}

void Button::set_pressed_tooltip(const std::string& text) {
	pressed_tooltip_ = text;
}

void Button::update_tooltip() {
	std::string new_tooltip;
	SDL_Keycode hotkey_code;
	if (m_permpressed) {
		new_tooltip = pressed_tooltip_;
		hotkey_code = pressed_hotkey_code_;
	} else {
		new_tooltip = normal_tooltip_;
		hotkey_code = hotkey_code_;
	}
	if (hotkey_code != SDLK_UNKNOWN) {
		if (new_tooltip.empty()) {
			/** TRANSLATORS: %1% is a hotkey */
			new_tooltip = (boost::format(_("Hotkey: %1%"))
						  % WLApplication::get()->hotkeys()->get_displayname(hotkey_code)).str();
		} else {
			/** TRANSLATORS: %1% is a tooltip, %2% is the corresponding hotkey */
			new_tooltip = (boost::format(_("%1% (Hotkey: %2%)"))
						  % new_tooltip
						  % WLApplication::get()->hotkeys()->get_displayname(hotkey_code)).str();
		}
	}
	Panel::set_tooltip(new_tooltip);
}


/**
 * Enable/Disable the button (disabled buttons can't be clicked).
 * Buttons are enabled by default
*/
void Button::set_enabled(bool const on)
{
	if (m_enabled == on)
		return;

	// disabled buttons should look different...
	if (on)
		m_enabled = true;
	else {
		if (m_pressed) {
			m_pressed = false;
			set_thinks(false);
			grab_mouse(false);
		}
		m_enabled = false;
		m_highlighted = false;
	}
	update();
}


/**
 * Redraw the button
*/
void Button::draw(RenderTarget & dst)
{
	// Draw the background
	if (!m_flat || m_draw_flat_background) {
		assert(m_pic_background);
		dst.fill_rect(Rect(Point(0, 0), get_w(), get_h()), RGBAColor(0, 0, 0, 255));
		dst.tile(Rect(Point(0, 0), get_w(), get_h()), m_pic_background, Point(get_x(), get_y()));
	}

	if (m_enabled && m_highlighted && !m_flat)
		dst.brighten_rect
			(Rect(Point(0, 0), get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);

	//  if we got a picture, draw it centered
	if (m_pic_custom) {
		const int max_image_w = get_w() - 2 * kButtonImageMargin;
		const int max_image_h = get_h() - 2 * kButtonImageMargin;
		double image_scale =
		   std::min(1.,
		            std::min(static_cast<double>(max_image_w) / m_pic_custom->width(),
		                     static_cast<double>(max_image_h) / m_pic_custom->height()));
		int blit_width = image_scale * m_pic_custom->width();
		int blit_height = image_scale * m_pic_custom->height();

		if (m_enabled) {
		dst.blitrect_scale(
		   Rect((get_w() - blit_width) / 2, (get_h() - blit_height) / 2, blit_width, blit_height),
		   m_pic_custom,
		   Rect(0, 0, m_pic_custom->width(), m_pic_custom->height()),
		   1.,
		   BlendMode::UseAlpha);
		} else {
			dst.blitrect_scale_monochrome(
			   Rect((get_w() - blit_width) / 2, (get_h() - blit_height) / 2, blit_width, blit_height),
			   m_pic_custom,
			   Rect(0, 0, m_pic_custom->width(), m_pic_custom->height()),
			   RGBAColor(255, 255, 255, 127));
		}

	} else if (m_title.length()) {
		//  otherwise draw title string centered

		m_textstyle.fg = m_enabled ? UI_FONT_CLR_FG : UI_FONT_CLR_DISABLED;

		UI::g_fh->draw_text
			(dst, m_textstyle, Point(get_w() / 2, get_h() / 2),
			 m_title, Align_Center,
			 m_draw_caret ? m_title.length() : std::numeric_limits<uint32_t>::max());
	}

	//  draw border
	//  a pressed but not highlighted button occurs when the user has pressed
	//  the left mouse button and then left the area of the button or the button
	//  stays pressed when it is pressed once
	RGBAColor black(0, 0, 0, 255);

	// m_permpressed is true, we invert the behaviour on m_pressed
	bool draw_pressed = m_permpressed ?    !(m_pressed && m_highlighted)
	                                  :     (m_pressed && m_highlighted);

	if (!m_flat) {
		assert(2 <= get_w());
		assert(2 <= get_h());
		//  button is a normal one, not flat
		if (!draw_pressed) {
			//  top edge
			dst.brighten_rect
				(Rect(Point(0, 0), get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  left edge
			dst.brighten_rect
				(Rect(Point(0, 2), 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  bottom edge
			dst.fill_rect(Rect(Point(2, get_h() - 2), get_w() - 2, 1), black);
			dst.fill_rect(Rect(Point(1, get_h() - 1), get_w() - 1, 1), black);
			//  right edge
			dst.fill_rect(Rect(Point(get_w() - 2, 2), 1, get_h() - 2), black);
			dst.fill_rect(Rect(Point(get_w() - 1, 1), 1, get_h() - 1), black);
		} else {
			//  bottom edge
			dst.brighten_rect
				(Rect(Point(0, get_h() - 2), get_w(), 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);
			//  right edge
			dst.brighten_rect
				(Rect(Point(get_w() - 2, 0), 2, get_h() - 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);
			//  top edge
			dst.fill_rect(Rect(Point(0, 0), get_w() - 1, 1), black);
			dst.fill_rect(Rect(Point(0, 1), get_w() - 2, 1), black);
			//  left edge
			dst.fill_rect(Rect(Point(0, 0), 1, get_h() - 1), black);
			dst.fill_rect(Rect(Point(1, 0), 1, get_h() - 2), black);
		}
	} else {
		//  Button is flat, do not draw borders, instead, if it is pressed, draw
		//  a box around it.
		if (m_enabled && m_highlighted)
		{
			RGBAColor shade(100, 100, 100, 80);
			dst.fill_rect(Rect(Point(0, 0), get_w(), 2), shade);
			dst.fill_rect(Rect(Point(0, 2), 2, get_h() - 2), shade);
			dst.fill_rect(Rect(Point(0, get_h() - 2), get_w(), get_h()), shade);
			dst.fill_rect(Rect(Point(get_w() - 2, 0), get_w(), get_h()), shade);
		}
	}
}

void Button::think()
{
	assert(m_repeating);
	assert(m_pressed);
	Panel::think();

	if (m_highlighted) {
		int32_t const time = WLApplication::get()->get_time();
		if (m_time_nextact <= time) {
			m_time_nextact += MOUSE_BUTTON_AUTOREPEAT_TICK; //  schedule next tick
			if (m_time_nextact < time)
				m_time_nextact = time;
			play_click();
			sigclicked();
			clicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
}

/**
 * Update highlighted status
*/
void Button::handle_mousein(bool const inside)
{
	bool oldhl = m_highlighted;

	m_highlighted = inside && m_enabled;

	if (oldhl == m_highlighted)
		return;

	if (m_highlighted)
		sigmousein();
	else
		sigmouseout();

	update();
}


/**
 * Update the pressed status of the button
*/
bool Button::handle_mousepress(uint8_t const btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	if (m_enabled) {
		grab_mouse(true);
		m_pressed = true;
		if (m_repeating) {
			m_time_nextact =
				WLApplication::get()->get_time() + MOUSE_BUTTON_AUTOREPEAT_DELAY;
			set_thinks(true);
		}
	}
	update();

	return true;
}
bool Button::handle_mouserelease(uint8_t const btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	update_tooltip();

	if (m_pressed) {
		m_pressed = false;
		set_thinks(false);
		grab_mouse(false);
		update();
		if (m_highlighted && m_enabled) {
			play_click();
			sigclicked();
			clicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
	return true;
}

bool Button::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t) {
	return true; // We handle this always by lighting up
}

void Button::set_perm_pressed(bool state) {
	if (state != m_permpressed) {
		m_permpressed = state;
		update_tooltip();
		update();
	}
}

void Button::set_flat(bool flat) {
	m_flat = flat;
}

void Button::set_draw_flat_background(bool set) {
	m_draw_flat_background = set;
}

}
