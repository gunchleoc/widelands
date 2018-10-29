/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_TYPE_H
#define WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_TYPE_H

#include <string>

namespace Widelands {

// This enum lists the available classes of Map Objects.
enum class MapObjectType : uint8_t {
	MAPOBJECT = 0,  // Root superclass

	WARE,  //  class WareInstance
	BATTLE,
	FLEET,

	BOB = 10,  // Bob
	CRITTER,   // Bob -- Critter
	SHIP,      // Bob -- Ship
	WORKER,    // Bob -- Worker
	CARRIER,   // Bob -- Worker -- Carrier
	SOLDIER,   // Bob -- Worker -- Soldier

	// everything below is at least a BaseImmovable
	IMMOVABLE = 30,

	// everything below is at least a PlayerImmovable
	FLAG = 40,
	ROAD,
	PORTDOCK,

	// everything below is at least a Building
	BUILDING = 100,    // Building
	CONSTRUCTIONSITE,  // Building -- Constructionsite
	DISMANTLESITE,     // Building -- Dismantlesite
	WAREHOUSE,         // Building -- Warehouse
	MARKET,            // Building -- Market
	PRODUCTIONSITE,    // Building -- Productionsite
	MILITARYSITE,      // Building -- Productionsite -- Militarysite
	TRAININGSITE       // Building -- Productionsite -- Trainingsite
};

// Returns a string representation for 'type'.
std::string to_string(MapObjectType type);
} // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_TYPE_H
