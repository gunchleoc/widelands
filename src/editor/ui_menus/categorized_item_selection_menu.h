/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_CATEGORIZED_ITEM_SELECTION_MENU_H
#define WL_EDITOR_UI_MENUS_CATEGORIZED_ITEM_SELECTION_MENU_H

#include <string>
#include <cmath>

#include "base/i18n.h"
#include "graphic/image.h"
#include "logic/description_maintainer.h"
#include "logic/world/editor_category.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"

template <typename DescriptionType, typename ToolType>
class CategorizedItemSelectionMenu : public UI::Box {
public:
	// Creates a box with a tab panel for each category in 'categories' and
	// populates them with the 'descriptions' ordered by the category by calling
	// 'create_checkbox' for each of the descriptions. Calls
	// 'select_correct_tool' whenever a selection has been made, also keeps a
	// text label updated and updates the 'tool' with current selections. Does
	// not take ownership.
	CategorizedItemSelectionMenu(
	   UI::Panel* parent,
	   const DescriptionMaintainer<Widelands::EditorCategory>& categories,
	   const DescriptionMaintainer<DescriptionType>& descriptions,
	   std::function<UI::Checkbox* (UI::Panel* parent, const DescriptionType& descr)> create_checkbox,
	   const std::function<void()> select_correct_tool,
	   ToolType* const tool);

private:
	// Called when an item was selected.
	void selected(int32_t, bool);

	// Update the label with the currently selected object names.
	void update_label();

	const DescriptionMaintainer<DescriptionType>& descriptions_;
	std::function<void()> select_correct_tool_;
	bool protect_against_recursive_select_;
	UI::Textarea current_selection_names_;
	std::map<int, UI::Checkbox*> checkboxes_;
	ToolType* const tool_;  // not owned
};

template <typename DescriptionType, typename ToolType>
CategorizedItemSelectionMenu<DescriptionType, ToolType>::CategorizedItemSelectionMenu
	(UI::Panel* parent,
	const DescriptionMaintainer<Widelands::EditorCategory>& categories,
	const DescriptionMaintainer<DescriptionType>& descriptions,
	const std::function<UI::Checkbox* (UI::Panel* parent, const DescriptionType& descr)>
		create_checkbox,
	const std::function<void()> select_correct_tool,
	ToolType* const tool) :
	UI::Box(parent, 0, 0, UI::Box::Vertical),
	descriptions_(descriptions),
	select_correct_tool_(select_correct_tool),
	protect_against_recursive_select_(false),
	current_selection_names_(this, 0, 0, 0, 20, UI::Align_Center),
	tool_(tool)
{
	UI::TabPanel* tab_panel = new UI::TabPanel(this, 0, 0, nullptr);
	add(tab_panel, UI::Align_Center);

	for (uint32_t category_index = 0; category_index < categories.size(); ++category_index) {
		const Widelands::EditorCategory& category = categories.get(category_index);

		std::vector<int> item_indices;
		for (size_t j = 0; j < descriptions_.size(); ++j) {
			if (descriptions_.get(j).editor_category().name() != category.name()) {
				continue;
			}
			item_indices.push_back(j);
		}

		UI::Box* vertical = new UI::Box(tab_panel, 0, 0, UI::Box::Vertical);
		const int kSpacing = 5;
		vertical->add_space(kSpacing);

		const uint32_t items_in_row =
		   static_cast<uint32_t>(std::ceil(std::sqrt(static_cast<float>(item_indices.size()))));
		int nitems_handled = 0;
		UI::Box* horizontal = nullptr;
		for (const int i : item_indices) {
			if (nitems_handled % items_in_row == 0) {
				horizontal = new UI::Box(vertical, 0, 0, UI::Box::Horizontal);
				horizontal->add_space(kSpacing);

				vertical->add(horizontal, UI::Align_Left);
				vertical->add_space(kSpacing);
			}
			assert(horizontal != nullptr);

			UI::Checkbox* cb = create_checkbox(horizontal, descriptions_.get(i));
			cb->set_state(tool_->is_enabled(i));
			cb->changedto.connect(boost::bind(&CategorizedItemSelectionMenu::selected, this, i, _1));
			checkboxes_[i] = cb;
			horizontal->add(cb, UI::Align_Left);
			horizontal->add_space(kSpacing);
			++nitems_handled;
		}
		tab_panel->add(category.name(), category.picture(), vertical, category.descname());
	}
	add(&current_selection_names_, UI::Align_Center, true);
}

template <typename DescriptionType, typename ToolType>
void CategorizedItemSelectionMenu<DescriptionType, ToolType>::selected(const int32_t n,
                                                                       const bool t) {
	if (protect_against_recursive_select_)
		return;

	//  TODO(unknown): This code is erroneous. It checks the current key state. What it
	//  needs is the key state at the time the mouse was clicked. See the
	//  usage comment for get_key_state.
	const bool multiselect = get_key_state(SDL_SCANCODE_LCTRL) | get_key_state(SDL_SCANCODE_RCTRL);
	if (!t && (!multiselect || tool_->get_nr_enabled() == 1))
		checkboxes_[n]->set_state(true);
	else {
		if (!multiselect) {
			for (uint32_t i = 0; tool_->get_nr_enabled(); ++i)
				tool_->enable(i, false);
			//  disable all checkboxes
			protect_against_recursive_select_ = true;
			const int32_t size = checkboxes_.size();
			for (int32_t i = 0; i < size; ++i) {
				if (i != n)
					checkboxes_[i]->set_state(false);
			}
			protect_against_recursive_select_ = false;
		}

		tool_->enable(n, t);
		select_correct_tool_();
		update_label();
	}
}

template <typename DescriptionType, typename ToolType>
void CategorizedItemSelectionMenu<DescriptionType, ToolType>::update_label() {
	std::string buf = _("Current:");
	int j = tool_->get_nr_enabled();
	for (int i = 0; j; ++i) {
		if (tool_->is_enabled(i)) {
			buf += " ";
			buf += descriptions_.get(i).descname();
			--j;
		}
	}
	current_selection_names_.set_text(buf);
}

#endif  // end of include guard: WL_EDITOR_UI_MENUS_CATEGORIZED_ITEM_SELECTION_MENU_H
