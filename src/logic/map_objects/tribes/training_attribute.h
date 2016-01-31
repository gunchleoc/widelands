/*
 * Copyright (C) 2004, 2008 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRAINING_ATTRIBUTE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRAINING_ATTRIBUTE_H

namespace Widelands {

/**
 * Indices for specific, individual attributes that \ref MapObject instances
 * may have. Used in conjunction with \ref Requirements.
 */
enum TrainingAttribute {
	atrHP = 0,
	atrAttack,
	atrDefense,
	atrEvade,
	atrTotal = 100
};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRAINING_ATTRIBUTE_H
