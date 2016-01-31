/*
 * Copyright (C) 2003, 2006-2011 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_BOX_H
#define WL_UI_BASIC_BOX_H

#include <memory>
#include <vector>

#include "graphic/align.h"
#include "ui_basic/panel.h"
#include "ui_basic/scrollbar.h"

namespace UI {

/**
 * A layouting panel that holds a number of child panels.
 * The Panels you add to the Box must be children of the Box.
 * The Box automatically resizes itself and positions the added children.
*/
struct Box : public Panel {
	enum {
		Horizontal = 0,
		Vertical = 1,
	};

	Box
		(Panel * parent,
		 int32_t x, int32_t y,
		 uint32_t orientation,
		 int32_t max_x = 0, int32_t max_y = 0,
		 uint32_t inner_spacing = 0);

	void set_scrolling(bool scroll);

	int32_t get_nritems() const {return m_items.size();}

	void add
		(Panel * panel,
		UI::Align align,
		bool fullsize = false,
		bool fillspace = false);
	void add_space(uint32_t space);
	void add_inf_space();
	bool is_snap_target() const override {return true;}

	void set_min_desired_breadth(uint32_t min);

protected:
	void layout() override;
	void update_desired_size() override;

private:
	void get_item_desired_size(uint32_t idx, int* depth, int* breadth);
	void get_item_size(uint32_t idx, int* depth, int* breadth);
	void set_item_size(uint32_t idx, int depth, int breadth);
	void set_item_pos(uint32_t idx, int32_t pos);
	void scrollbar_moved(int32_t);
	void update_positions();

	//don't resize beyond this size
	int m_max_x, m_max_y;

	struct Item {
		enum Type {
			ItemPanel,
			ItemSpace,
		};

		Type type;

		union {
			struct {
				Panel * panel;
				UI::Align align;
				bool fullsize;
			} panel;
			int space;
		} u;

		bool fillspace;
		int assigned_var_depth;
	};

	bool m_scrolling;
	std::unique_ptr<Scrollbar> m_scrollbar;
	uint32_t m_orientation;
	uint32_t m_mindesiredbreadth;
	uint32_t m_inner_spacing;

	std::vector<Item> m_items;
};

}

#endif  // end of include guard: WL_UI_BASIC_BOX_H
