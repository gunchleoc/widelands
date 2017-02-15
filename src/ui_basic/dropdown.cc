/*
 * Copyright (C) 2016-2017 by the Widelands Development Team
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

#include "ui_basic/dropdown.h"

#include <algorithm>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/align.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "ui_basic/mouse_constants.h"

namespace {

int base_height() {
	return std::max(
	   24,
	   UI::g_fh1->render(as_uifont(UI::g_fh1->fontset()->representative_character()))->height() + 2);
}

}  // namespace

namespace UI {

BaseDropdown::BaseDropdown(UI::Panel* parent,
                           int32_t x,
                           int32_t y,
                           uint32_t w,
                           uint32_t h,
                           const std::string& label,
                           const Image* background,
                           const Image* button_background)
   : UI::Panel(parent,
               x,
               y,
               w,
               base_height()),  // Height only to fit the button, so we can use this in Box layout.
     max_list_height_(h - 2 * get_h()),
     mouse_tolerance_(50),
     button_box_(this, 0, 0, UI::Box::Horizontal, w, h),
     push_button_(&button_box_,
                  "dropdown_select",
                  0,
                  0,
                  24,
                  get_h(),
                  button_background,
                  g_gr->images().get("images/ui_basic/scrollbar_down.png"),
                  pgettext("dropdown", "Select Item")),
     display_button_(&button_box_, "dropdown_label", 0, 0, w - 24, get_h(), background, label),
     // Hook into parent so we can drop down outside the panel
     list_(parent, x, y + get_h(), w, 0, button_background, ListselectLayout::kDropdown),
     label_(label) {
	list_.set_visible(false);
	list_.set_background(background);
	display_button_.set_perm_pressed(true);
	button_box_.add(&display_button_, UI::Align::kLeft);
	button_box_.add(&push_button_, UI::Align::kLeft);
	button_box_.set_size(w, get_h());

	display_button_.sigclicked.connect(boost::bind(&BaseDropdown::toggle_list, this));
	push_button_.sigclicked.connect(boost::bind(&BaseDropdown::toggle_list, this));
	list_.clicked.connect(boost::bind(&BaseDropdown::set_value, this));
	list_.clicked.connect(boost::bind(&BaseDropdown::toggle_list, this));
	set_can_focus(true);
	layout();
}

BaseDropdown::~BaseDropdown() {
	clear();
}

void BaseDropdown::set_height(int height) {
	max_list_height_ = height - base_height();
	layout();
}

void BaseDropdown::layout() {
	const int base_h = base_height();
	const int w = get_w();
	button_box_.set_size(w, base_h);
	display_button_.set_desired_size(w - 24, base_h);
	int new_list_height =
	   std::min(static_cast<int>(list_.size()) * list_.get_lineheight(), max_list_height_);
	list_.set_size(w, new_list_height);
	set_desired_size(w, base_h);
}

void BaseDropdown::add(const std::string& name,
                       const uint32_t value,
                       const Image* pic,
                       const bool select_this,
                       const std::string& tooltip_text) {
	list_.add(name, value, pic, select_this, tooltip_text);
	if (select_this) {
		set_value();
	}
	layout();
}

bool BaseDropdown::has_selection() const {
	return list_.has_selection();
}

uint32_t BaseDropdown::get_selected() const {
	return list_.get_selected();
}

void BaseDropdown::set_label(const std::string& text) {
	label_ = text;
	display_button_.set_title(label_);
}

void BaseDropdown::set_tooltip(const std::string& text) {
	tooltip_ = text;
	display_button_.set_tooltip(tooltip_);
}

void BaseDropdown::set_enabled(bool on) {
	set_can_focus(on);
	push_button_.set_enabled(on);
	push_button_.set_tooltip(on ? pgettext("dropdown", "Select Item") : "");
	display_button_.set_enabled(on);
	list_.set_visible(false);
}

void BaseDropdown::set_pos(Vector2i point) {
	UI::Panel::set_pos(point);
	list_.set_pos(Vector2i(point.x, point.y + get_h()));
}

void BaseDropdown::clear() {
	list_.clear();
	list_.set_size(list_.get_w(), 0);
	set_layout_toplevel(false);
}

void BaseDropdown::think() {
	if (list_.is_visible()) {
		// Autocollapse with a bit of tolerance for the mouse movement to make it less fiddly.
		if (!(has_focus() || list_.has_focus()) || is_mouse_away()) {
			toggle_list();
		}
	}
}

uint32_t BaseDropdown::size() const {
	return list_.size();
}

void BaseDropdown::set_value() {
	const std::string name = list_.has_selection() ? list_.get_selected_name() :
	                                                 /** TRANSLATORS: Selection in Dropdown menus. */
	                            pgettext("dropdown", "Not Selected");

	if (label_.empty()) {
		display_button_.set_title(name);
	} else {
		/** TRANSLATORS: Label: Value. */
		display_button_.set_title((boost::format(_("%1%: %2%")) % label_ % (name)).str());
	}
	display_button_.set_tooltip(list_.has_selection() ? list_.get_selected_tooltip() : tooltip_);
	selected();
	current_selection_ = list_.selection_index();
}

void BaseDropdown::toggle_list() {
	list_.set_visible(!list_.is_visible());
	if (list_.is_visible()) {
		list_.move_to_top();
		focus();
	}
	// Make sure that the list covers and deactivates the elements below it
	set_layout_toplevel(list_.is_visible());
}

bool BaseDropdown::is_mouse_away() const {
	return (get_mouse_position().x + mouse_tolerance_) < 0 ||
	       get_mouse_position().x > (get_w() + mouse_tolerance_) ||
	       (get_mouse_position().y + mouse_tolerance_ / 2) < 0 ||
	       get_mouse_position().y > (get_h() + list_.get_h() + mouse_tolerance_);
}

bool BaseDropdown::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			if (list_.is_visible()) {
				set_value();
			}
		case SDLK_ESCAPE:
			if (list_.is_visible()) {
				list_.select(current_selection_);
				toggle_list();
				return true;
			}
			break;
		case SDLK_DOWN:
			if (!list_.is_visible() && !is_mouse_away()) {
				toggle_list();
				return true;
			}
			break;
		default:
			break;  // not handled
		}
	}
	if (list_.is_visible()) {
		return list_.handle_key(down, code);
	}
	return false;
}

}  // namespace UI
