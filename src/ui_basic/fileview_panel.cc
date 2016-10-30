/*
 * Copyright (C) 2016 by Widelands Development Team
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

#include "ui_basic/fileview_panel.h"

#include <memory>

#include <boost/format.hpp>

#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace UI {

FileViewPanel::FileViewPanel(Panel* parent,
                             int32_t x,
                             int32_t y,
                             int32_t w,
                             int32_t h,
                             const Image* background,
                             TabPanel::Type border_type)
   : TabPanel(parent, x, y, w, h, background, border_type), padding_(5) {
}

void FileViewPanel::add_tab(const std::string& lua_script) {
	std::string content, title;
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> t(lua.run_script(lua_script));
		content = t->get_string("text");
		title = t->get_string("title");
	} catch (LuaError& err) {
		content = err.what();
		title = "Lua error";
	}
	boxes_.push_back(
	   std::unique_ptr<UI::Box>(new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, padding_)));
	size_t index = boxes_.size() - 1;

	// If there is a border, we have less space
	const int width =
	   border_type_ == TabPanel::Type::kNoBorder ? get_w() - padding_ : get_w() - 2 * padding_;

	const int height = border_type_ == TabPanel::Type::kNoBorder ?
	                      get_inner_h() - 2 * padding_ - UI::kTabPanelButtonHeight :
	                      get_inner_h() - 3 * padding_ - UI::kTabPanelButtonHeight;

	textviews_.push_back(std::unique_ptr<UI::MultilineTextarea>(
	   new UI::MultilineTextarea(boxes_.at(index).get(), 0, 0, width, height, content)));
	add((boost::format("about_%lu") % index).str(), title, boxes_.at(index).get(), "");
	boxes_.at(index)->set_size(get_inner_w(), get_inner_h());

	assert(boxes_.size() == textviews_.size());
	assert(tabs().size() == textviews_.size());
}

}  // namespace UI
