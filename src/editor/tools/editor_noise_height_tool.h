/*
 * Copyright (C) 2002-2004, 2006-2008, 2012 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_EDITOR_NOISE_HEIGHT_TOOL_H
#define WL_EDITOR_TOOLS_EDITOR_NOISE_HEIGHT_TOOL_H

#include "editor/tools/editor_set_height_tool.h"

/// Set the height of a node to a random value within a defined interval.
struct EditorNoiseHeightTool : public EditorTool {
	EditorNoiseHeightTool
	(EditorSetHeightTool & the_set_tool,
	 const Widelands::HeightInterval the_interval =
	     Widelands::HeightInterval(10, 14))
		:
		EditorTool(the_set_tool, the_set_tool),
		m_set_tool(the_set_tool),
		m_interval(the_interval)
	{}

	int32_t handle_click_impl(const Widelands::World& world,
	                          Widelands::NodeAndTriangle<> center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
							  Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::World& world,
	                         Widelands::NodeAndTriangle<> center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
							 Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive & parent) override;

	char const * get_sel_impl() const override {
		return "pics/fsel_editor_noise_height.png";
	}

	Widelands::HeightInterval get_interval() const {
		return m_interval;
	}
	void set_interval(Widelands::HeightInterval const i) {
		m_interval = i;
	}

	EditorSetHeightTool & set_tool() const {return m_set_tool;}

private:
	EditorSetHeightTool & m_set_tool;
	Widelands::HeightInterval m_interval;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_EDITOR_NOISE_HEIGHT_TOOL_H
