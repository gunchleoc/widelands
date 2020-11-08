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
	case ProductionCategory::kRoad:
		return "road";
	case ProductionCategory::kWaterway:
		return "waterway";
	case ProductionCategory::kTool:
		return "tool";
	case ProductionCategory::kTraining:
		return "training";
	case ProductionCategory::kNone:
		return "none";
	}
	NEVER_HERE();
}
}  // namespace Widelands
