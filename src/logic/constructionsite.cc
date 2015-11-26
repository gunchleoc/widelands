/*
 * Copyright (C) 2002-2004, 2006-2009, 2011 by the Widelands Development Team
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

#include "logic/constructionsite.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/wares_queue.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/worker.h"
#include "sound/sound_handler.h"
#include "ui_basic/window.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

ConstructionSiteDescr::ConstructionSiteDescr(const std::string& init_descname,
															const LuaTable& table,
															const EditorGameBase& egbase)
	: BuildingDescr(init_descname, MapObjectType::CONSTRUCTIONSITE, table, egbase)
{
	add_attribute(MapObject::CONSTRUCTIONSITE);
}

Building & ConstructionSiteDescr::create_object() const {
	return *new ConstructionSite(*this);
}


/*
==============================

IMPLEMENTATION

==============================
*/


ConstructionSite::ConstructionSite(const ConstructionSiteDescr & cs_descr) :
PartiallyFinishedBuilding (cs_descr),
m_fetchfromflag     (0),
m_builder_idle      (false)
{}


void ConstructionSite::update_statistics_string(std::string* s)
{
	unsigned int percent = (get_built_per64k() * 100) >> 16;
	*s = (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_DARK.hex_value() %
	      (boost::format(_("%i%% built")) % percent).str()).str();
}

/*
=======
Access to the wares queues by id
=======
*/
WaresQueue & ConstructionSite::waresqueue(DescriptionIndex const wi) {
	for (WaresQueue * ware : m_wares) {
		if (ware->get_ware() == wi) {
			return *ware;
		}
	}
	throw wexception
		("%s (%u) (building %s) has no WaresQueue for %u",
		 descr().name().c_str(), serial(), m_building->name().c_str(), wi);
}


/*
===============
Set the type of building we're going to build
===============
*/
void ConstructionSite::set_building(const BuildingDescr & building_descr) {
	PartiallyFinishedBuilding::set_building(building_descr);

	m_info.becomes = &building_descr;
}

/*
===============
Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(EditorGameBase & egbase)
{
	PartiallyFinishedBuilding::init(egbase);

	const std::map<DescriptionIndex, uint8_t> * buildcost;
	if (!m_old_buildings.empty()) {
		// Enhancement
		DescriptionIndex was_index = m_old_buildings.back();
		const BuildingDescr* was_descr = owner().tribe().get_building_descr(was_index);
		m_info.was = was_descr;
		buildcost = &m_building->enhancement_cost();
	} else {
		buildcost = &m_building->buildcost();
	}

	//  TODO(unknown): figure out whether planing is necessary

	//  initialize the wares queues
	size_t const buildcost_size = buildcost->size();
	m_wares.resize(buildcost_size);
	std::map<DescriptionIndex, uint8_t>::const_iterator it = buildcost->begin();

	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		WaresQueue & wq =
			*(m_wares[i] = new WaresQueue(*this, it->first, it->second));

		wq.set_callback(ConstructionSite::wares_queue_callback, this);
		wq.set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);

		m_work_steps += it->second;
	}
}


/*
===============
Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void ConstructionSite::cleanup(EditorGameBase & egbase)
{
	PartiallyFinishedBuilding::cleanup(egbase);

	if (m_work_steps <= m_work_completed) {
		// Put the real building in place
		DescriptionIndex becomes_idx = owner().tribe().building_index(m_building->name());
		m_old_buildings.push_back(becomes_idx);
		Building & b =
			m_building->create(egbase, owner(), m_position, false, false, m_old_buildings);
		if (Worker * const builder = m_builder.get(egbase)) {
			builder->reset_tasks(dynamic_cast<Game&>(egbase));
			builder->set_location(&b);
		}
		// Open the new building window if needed
		if (m_optionswindow) {
			Point window_position = m_optionswindow->get_pos();
			hide_options();
			InteractiveGameBase & igbase =
				dynamic_cast<InteractiveGameBase&>(*egbase.get_ibase());
			b.show_options(igbase, false, window_position);
		}
	}
}


/*
===============
Construction sites only burn if some of the work has been completed.
===============
*/
bool ConstructionSite::burn_on_destroy()
{
	if (m_work_completed >= m_work_steps)
		return false; // completed, so don't burn

	return m_work_completed || !m_old_buildings.empty();
}

/*
===============
Remember the ware on the flag. The worker will be sent from get_building_work().
===============
*/
bool ConstructionSite::fetch_from_flag(Game & game)
{
	++m_fetchfromflag;

	if (Worker * const builder = m_builder.get(game))
		builder->update_task_buildingwork(game);

	return true;
}


/*
===============
Called by our builder to get instructions.
===============
*/
bool ConstructionSite::get_building_work(Game & game, Worker & worker, bool) {
	if (&worker != m_builder.get(game)) {
		// Not our construction worker; e.g. a miner leaving a mine
		// that is supposed to be enhanced. Make him return to a warehouse
		worker.pop_task(game);
		worker.start_task_leavebuilding(game, true);
		return true;
	}

	if (!m_work_steps) //  Happens for building without buildcost.
		schedule_destroy(game); //  Complete the building immediately.

	// Check if one step has completed
	if (m_working) {
		if (static_cast<int32_t>(game.get_gametime() - m_work_steptime) < 0) {
			worker.start_task_idle
				(game,
				 worker.descr().get_animation("work"),
				 m_work_steptime - game.get_gametime());
			m_builder_idle = false;
			return true;
		} else {
			//TODO(fweber): cause "construction sounds" to be played -
			//perhaps dependent on kind of construction?

			++m_work_completed;
			if (m_work_completed >= m_work_steps)
				schedule_destroy(game);

			m_working = false;
		}
	}

	// Fetch wares from flag
	if (m_fetchfromflag) {
		--m_fetchfromflag;
		m_builder_idle = false;
		worker.start_task_fetchfromflag(game);
		return true;
	}

	// Drop all the wares that are too much out to the flag.
	for (WaresQueue * iqueue: m_wares) {
		WaresQueue * queue = iqueue;
		if (queue->get_filled() > queue->get_max_fill()) {
			queue->set_filled(queue->get_filled() - 1);
			const WareDescr & wd = *owner().tribe().get_ware_descr(queue->get_ware());
			WareInstance & ware = *new WareInstance(queue->get_ware(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);
			return true;
		}
	}

	// Check if we've got wares to consume
	if (m_work_completed < m_work_steps)
	{
		for (uint32_t i = 0; i < m_wares.size(); ++i) {
			WaresQueue & wq = *m_wares[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_max_size(wq.get_max_size() - 1);

			//update consumption statistic
			owner().ware_consumed(wq.get_ware(), 1);

			m_working = true;
			m_work_steptime = game.get_gametime() + CONSTRUCTIONSITE_STEP_TIME;

			worker.start_task_idle
				(game, worker.descr().get_animation("work"), CONSTRUCTIONSITE_STEP_TIME);
			m_builder_idle = false;
			return true;
		}
	}
	// The only work we have got for you, is to run around to look cute ;)
	if (!m_builder_idle) {
		worker.set_animation(game, worker.descr().get_animation("idle"));
		m_builder_idle = true;
	}
	worker.schedule_act(game, 2000);
	return true;
}


/*
===============
Called by WaresQueue code when an ware has arrived
===============
*/
void ConstructionSite::wares_queue_callback
	(Game & game, WaresQueue *, DescriptionIndex, void * const data)
{
	ConstructionSite & cs = *static_cast<ConstructionSite *>(data);

	if (!cs.m_working)
		if (Worker * const builder = cs.m_builder.get(game))
			builder->update_task_buildingwork(game);
}


/*
===============
Draw the construction site.
===============
*/
void ConstructionSite::draw
	(const EditorGameBase & game, RenderTarget & dst, const FCoords& coords, const Point& pos)
{
	const uint32_t gametime = game.get_gametime();
	uint32_t tanim = gametime - m_animstart;

	if (coords != m_position)
		return; // draw big buildings only once

	// Draw the construction site marker
	dst.drawanim(pos, m_anim, tanim, get_owner());

	// Draw the partially finished building

	static_assert(0 <= CONSTRUCTIONSITE_STEP_TIME, "assert(0 <= CONSTRUCTIONSITE_STEP_TIME) failed.");
	m_info.totaltime = CONSTRUCTIONSITE_STEP_TIME * m_work_steps;
	m_info.completedtime = CONSTRUCTIONSITE_STEP_TIME * m_work_completed;

	if (m_working) {
		assert
			(m_work_steptime
			 <=
			 m_info.completedtime + CONSTRUCTIONSITE_STEP_TIME + gametime);
		m_info.completedtime += CONSTRUCTIONSITE_STEP_TIME + gametime - m_work_steptime;
	}

	uint32_t anim_idx;
	uint32_t cur_frame;
	try {
		anim_idx = building().get_animation("build");
	} catch (MapObjectDescr::AnimationNonexistent&) {
		try {
			anim_idx = building().get_animation("unoccupied");
		} catch (MapObjectDescr::AnimationNonexistent) {
			anim_idx = building().get_animation("idle");
		}
	}
	const Animation& anim = g_gr->animations().get_animation(anim_idx);
	const size_t nr_frames = anim.nr_frames();
	cur_frame = m_info.totaltime ? m_info.completedtime * nr_frames / m_info.totaltime : 0;
	// Redefine tanim
	tanim = cur_frame * FRAME_LENGTH;

	const uint16_t w = anim.width();
	const uint16_t h = anim.height();

	uint32_t lines = h * m_info.completedtime * nr_frames;
	if (m_info.totaltime)
		lines /= m_info.totaltime;
	assert(h * cur_frame <= lines);
	lines -= h * cur_frame; //  This won't work if pictures have various sizes.

	if (cur_frame) //  not the first pic
		//  draw the prev pic from top to where next image will be drawing
		dst.drawanimrect(pos, anim_idx, tanim - FRAME_LENGTH, get_owner(), Rect(Point(0, 0), w, h - lines));
	else if (!m_old_buildings.empty()) {
		DescriptionIndex prev_idx = m_old_buildings.back();
		const BuildingDescr* prev_building = owner().tribe().get_building_descr(prev_idx);
		//  Is the first picture but there was another building here before,
		//  get its most fitting picture and draw it instead.
		uint32_t prev_building_anim_idx;
		try {
			prev_building_anim_idx = prev_building->get_animation("unoccupied");
		} catch (MapObjectDescr::AnimationNonexistent &) {
			prev_building_anim_idx = prev_building->get_animation("idle");
		}
		const Animation& prev_building_anim = g_gr->animations().get_animation(prev_building_anim_idx);
		dst.drawanimrect
			(pos, prev_building_anim_idx, tanim - FRAME_LENGTH, get_owner(),
			 Rect
			 (Point(0, 0), prev_building_anim.width(), std::min<int>(prev_building_anim.height(), h - lines)));
	}

	assert(lines <= h);
	dst.drawanimrect(pos, anim_idx, tanim, get_owner(), Rect(Point(0, h - lines), w, lines));

	// Draw help strings
	draw_help(game, dst, coords, pos);
}

}
