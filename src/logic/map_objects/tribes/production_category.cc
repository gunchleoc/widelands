/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "logic/map_objects/tribes/production_category.h"

#include "base/wexception.h"

namespace Widelands {

const std::string to_string(ProductionCategory category) {
	switch (category) {
	case ProductionCategory::kConstruction:
		return "construction";
	case ProductionCategory::kMining:
		return "mining";
	case ProductionCategory::kRoads:
		return "roads";
	case ProductionCategory::kSeafaring:
		return "seafaring";
	case ProductionCategory::kTool:
		return "tool";
	case ProductionCategory::kTraining:
		return "training";
	case ProductionCategory::kWaterways:
		return "waterways";
	case ProductionCategory::kNone:
		return "none";
	}
	NEVER_HERE();
}

// NOCOM we'll want name/descname here
const std::string to_string(ProductionUICategory category) {
	switch (category) {
	case ProductionUICategory::kConstruction:
		return "construction";
	case ProductionUICategory::kMilitary:
		return "military";
	case ProductionUICategory::kMines:
		return "mines";
	case ProductionUICategory::kTransport:
		return "transport";
	case ProductionUICategory::kTools:
		return "tool";
	case ProductionUICategory::kTraining:
		return "training";
	case ProductionUICategory::kNone:
		return "miscellaneous";
	}
	NEVER_HERE();
}
}  // namespace Widelands
