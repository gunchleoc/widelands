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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_CATEGORY_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_CATEGORY_H

#include <string>

namespace Widelands {
// TODO(GunChleoc): Add more categories as needed, eg. seafaring, ...
// NOCOM update comment
// Direct construction materials are registered at the TribeDescr.
// The kConstruction class here includes all wares involved in creating a construction material too.
enum class ProductionCategory {
	kMisc,
	kConstruction,
	kRoads,
	kSeafaring,
	kTool,
	kTraining,
	kWaterways
};
const std::string to_string(ProductionCategory category);

// The order here determines the UI order
enum class ProductionUICategory {
	kConstruction,
	kTools,
	kTransport,
	kMisc,
	kTraining,
	kMilitary,
};
const std::string to_string(ProductionUICategory category);

struct WeightedProductionCategory {
	const ProductionCategory category;
	const unsigned distance;

	inline bool operator<(const WeightedProductionCategory& other) const {
		return (distance < other.distance) ||
		       (distance == other.distance &&
		        static_cast<unsigned>(category) < static_cast<unsigned>(other.category));
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_CATEGORY_H
