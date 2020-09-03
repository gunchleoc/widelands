/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "logic/field.h"

#include <algorithm>

#include "base/log.h" // NOCOM
#include "logic/map_objects/bob.h" // NOCOM
#include "wui/mapviewpixelconstants.h"

namespace Widelands {

void Field::clear_bobs() {
	// NOCOM why is it not there in spite of being initialized in the constructor?!
	if (bobs_queue) {
		bobs_queue->clear();
	} else {
		bobs_queue = new std::list<Bob*>();
	}
}
const std::list<Bob*>& Field::bobs() const {
	return *bobs_queue;
}
Bob* Field::remove_first_bob() {
	// NOCOM why is it not there in spite of being initialized in the constructor?!
	if (!bobs_queue) {
		bobs_queue = new std::list<Bob*>();
	}
	assert(!bobs_queue->empty());
	Bob* result = bobs_queue->front();
	bobs_queue->pop_front();
	return result;
}
void Field::add_bob(Bob* bob, bool front) {
	// NOCOM why is it not there in spite of being initialized in the constructor?!
	if (!bobs_queue) {
		bobs_queue = new std::list<Bob*>();
	}
	/*
NOCOM add_bob badger
NOCOM elements: 18446744073709551552
NOCOM queue is empty
	 * */
	if (bob == nullptr) {
		log("NOCOM bob is nullptr!\n");
	}
	log("NOCOM add_bob %s\n", bob->descr().name().c_str());
	log("NOCOM elements: %lu\n", bobs_queue->size());
	if (!bobs_queue->empty()) {
		int counter = 0;
		for (Bob* b : *bobs_queue) {
			log("NOCOM have %s\n", b->descr().name().c_str());
			++counter;
			if (counter > 10) break;
		}
	} else {
		log("NOCOM queue is empty\n");
	}
	if (front) {
		bobs_queue->push_front(bob);
	} else {
		// NOCOM points to zero page.
		bobs_queue->push_back(bob);
	}
	log("NOCOM elements after: %lu\n", bobs_queue->size());
}
void Field::remove_bob(Bob* bob) {
	// NOCOM why is it not there in spite of being initialized in the constructor?!
	if (!bobs_queue) {
		bobs_queue = new std::list<Bob*>();
	}
	// NOCOM use list::remove
	auto it = std::find(bobs_queue->begin(), bobs_queue->end(), bob);
	if (it != bobs_queue->end()) {
		bobs_queue->erase(it);
	} else {
		NEVER_HERE();
	}
}

/**
 * Set the field's brightness based upon the slopes.
 * Slopes are calulated as this field's height - neighbour's height.
 */
void Field::set_brightness(int32_t const l,
                           int32_t const r,
                           int32_t const tl,
                           int32_t const tr,
                           int32_t const bl,
                           int32_t const br) {

	auto calc_brightness = [](int32_t const left, int32_t const right, int32_t const topleft,
	                          int32_t const topright, int32_t const bottomleft,
	                          int32_t const bottomright) {
		constexpr float kVectorThird = 0.57735f;  // sqrt(1/3)
		constexpr float kCos60 = 0.5f;
		constexpr float kSin60 = 0.86603f;
		constexpr float kLightFactor = -75.0f;

		static Vector3f sun_vect =
		   Vector3f(kVectorThird, -kVectorThird, -kVectorThird);  //  |sun_vect| = 1

		// find normal
		// more guessed than thought about
		// but hey, results say I am good at guessing :)
		// perhaps I will paint an explanation for this someday
		// florian
		Vector3f normal(0, 0, kTriangleWidth);
		normal.x -= left * kHeightFactor;
		normal.x += right * kHeightFactor;
		normal.x -= topleft * kHeightFactorFloat * kCos60;
		normal.y -= topleft * kHeightFactorFloat * kSin60;
		normal.x += topright * kHeightFactorFloat * kCos60;
		normal.y -= topright * kHeightFactorFloat * kSin60;
		normal.x -= bottomleft * kHeightFactorFloat * kCos60;
		normal.y += bottomleft * kHeightFactorFloat * kSin60;
		normal.x += bottomright * kHeightFactorFloat * kCos60;
		normal.y += bottomright * kHeightFactorFloat * kSin60;
		normal.normalize();

		return normal.dot(sun_vect) * kLightFactor;
	};

	// HACK to normalize flat terrain to zero brightness
	static float flatbrightness = calc_brightness(0, 0, 0, 0, 0, 0);

	float b = calc_brightness(l, r, tl, tr, bl, br) - flatbrightness;

	if (b > 0) {
		b *= 1.5;
	}

	if (b < -128) {
		b = -128;
	} else if (b > 127) {
		b = 127;
	}
	brightness = static_cast<int8_t>(b);
}
}  // namespace Widelands
