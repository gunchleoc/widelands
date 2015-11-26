/*
 * Copyright (C) 2006-2011 by the Widelands Development Team
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

#include "logic/partially_finished_building.h"

#include "base/macros.h"
#include "economy/request.h"
#include "economy/wares_queue.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/worker.h"
#include "sound/sound_handler.h"

namespace Widelands {

PartiallyFinishedBuilding::PartiallyFinishedBuilding
	(const BuildingDescr & gdescr) :
Building         (gdescr),
m_building       (nullptr),
m_builder_request(nullptr),
m_working        (false),
m_work_steptime  (0),
m_work_completed (0),
m_work_steps     (0)
{}

/*
===============
Set the type of building we're going to build
===============
*/
void PartiallyFinishedBuilding::set_building(const BuildingDescr & building_descr) {
	assert(!m_building);

	m_building = &building_descr;
}

void PartiallyFinishedBuilding::cleanup(EditorGameBase & egbase) {
	if (m_builder_request) {
		delete m_builder_request;
		m_builder_request = nullptr;
	}

	for (WaresQueue * temp_ware : m_wares) {
		temp_ware->cleanup();
		delete temp_ware;
	}
	m_wares.clear();

	Building::cleanup(egbase);
}

void PartiallyFinishedBuilding::init(EditorGameBase & egbase) {
	Building::init(egbase);

	if (upcast(Game, game, &egbase))
		request_builder(*game);

	g_sound_handler.play_fx("sound/create_construction_site", m_position, 255);
}

/*
===============
Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void PartiallyFinishedBuilding::set_economy(Economy * const e)
{
	if (Economy * const old = get_economy()) {
		for (WaresQueue * temp_ware : m_wares) {
			temp_ware->remove_from_economy(*old);
		}
	}
	Building::set_economy(e);
	if (m_builder_request)
		m_builder_request->set_economy(e);

	if (e)
		for (WaresQueue * temp_ware : m_wares) {
			temp_ware->add_to_economy(*e);
		}
}



/*
===============
Issue a request for the builder.
===============
*/
void PartiallyFinishedBuilding::request_builder(Game &) {
	assert(!m_builder.is_set() && !m_builder_request);

	m_builder_request =
		new Request
			(*this,
			 owner().tribe().builder(),
			 PartiallyFinishedBuilding::request_builder_callback,
			 wwWORKER);
}

/*
===============
Override: construction size is always the same size as the building
===============
*/
int32_t PartiallyFinishedBuilding::get_size() const {
	return m_building->get_size();
}

/*
===============
Override: Even though construction sites cannot be built themselves, you can
bulldoze them.
===============
*/
uint32_t PartiallyFinishedBuilding::get_playercaps() const {
	uint32_t caps = Building::get_playercaps();

	caps |= PCap_Bulldoze;
	caps &= ~PCap_Dismantle;

	return caps;
}

/*
===============
Return the animation for the building that is in construction, as this
should be more useful to the player.
===============
*/
const Image* PartiallyFinishedBuilding::representative_image() const
{
	return m_building->representative_image(&owner().get_playercolor());
}


/*
===============
Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
// TODO(unknown): should take gametime or so
uint32_t PartiallyFinishedBuilding::get_built_per64k() const
{
	const uint32_t time = owner().egbase().get_gametime();
	uint32_t thisstep = 0;

	uint32_t ts = build_step_time();
	if (m_working) {
		thisstep = ts - (m_work_steptime - time);
		// The check below is necessary because we drive construction via
		// the construction worker in get_building_work(), and there can be
		// a small delay between the worker completing his job and requesting
		// new work.
		if (thisstep > ts)
			thisstep = ts;
	}
	thisstep = (thisstep << 16) / ts;
	uint32_t total = (thisstep + (m_work_completed << 16));
	if (m_work_steps)
		total /= m_work_steps;

	assert(total <= (1 << 16));

	return total;
}



/*
===============
Called by transfer code when the builder has arrived on site.
===============
*/
void PartiallyFinishedBuilding::request_builder_callback
	(Game            &       game,
	 Request         &       rq,
	 DescriptionIndex,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	assert(w);

	PartiallyFinishedBuilding & b = dynamic_cast<PartiallyFinishedBuilding&>(target);

	b.m_builder = w;

	delete &rq;
	b.m_builder_request = nullptr;

	w->start_task_buildingwork(game);
	b.set_seeing(true);
}


}
