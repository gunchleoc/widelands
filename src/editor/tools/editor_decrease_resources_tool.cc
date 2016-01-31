/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2015 by the Widelands Development Team
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

#include "editor/tools/editor_decrease_resources_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "editor/tools/editor_set_resources_tool.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"
#include "logic/widelands_geometry.h"


/**
 * Decrease the resources of the current field by one if
 * there is not already another resource there.
*/
int32_t EditorDecreaseResourcesTool::handle_click_impl(const Widelands::World& world,
                                                       Widelands::NodeAndTriangle<> const center,
                                                       EditorInteractive& /* parent */,
                                                       EditorActionArgs* args,
                                                       Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
	(*map,
	Widelands::Area<Widelands::FCoords>
	(map->get_fcoords(center.node), args->sel_radius));
	do {
		int32_t amount = mr.location().field->get_resources_amount();

		amount -= args->change_by;
		if (amount < 0)
			amount = 0;

		if (mr.location().field->get_resources() == args->cur_res &&
			map->is_resource_valid(world, mr.location(), args->cur_res)) {
			args->orgResT.push_back(mr.location().field->get_resources());
			args->orgRes.push_back(mr.location().field->get_resources_amount());
			map->initialize_resources(mr.location(), args->cur_res, amount);
		}

	} while (mr.advance(*map));
	return mr.radius();
}

int32_t EditorDecreaseResourcesTool::handle_undo_impl(const Widelands::World& world,
													  Widelands::NodeAndTriangle<> center,
													  EditorInteractive& parent,
													  EditorActionArgs* args,
													  Widelands::Map* map) {
	return parent.tools()->set_resources.handle_undo_impl(world, center, parent, args, map);
}

EditorActionArgs EditorDecreaseResourcesTool::format_args_impl(EditorInteractive & parent)
{
	EditorActionArgs a(parent);
	a.change_by = m_change_by;
	a.cur_res = m_cur_res;
	return a;
}
