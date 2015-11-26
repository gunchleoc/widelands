/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLAG_H
#define WL_ECONOMY_FLAG_H

#include <list>
#include <vector>

#include "base/macros.h"
#include "logic/immovable.h"
#include "economy/routing_node.h"

namespace Widelands {
class Building;
class Request;
struct Road;
class WareInstance;

class FlagDescr : public MapObjectDescr {
public:
	FlagDescr(char const* const _name, char const* const _descname)
		: MapObjectDescr(MapObjectType::FLAG, _name, _descname) {
	}
	~FlagDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(FlagDescr);
};

/**
 * Flag represents a flag, obviously.
 * A flag itself doesn't do much. However, it can have up to 6 roads attached
 * to it. Instead of the WALK_NW road, it can also have a building attached to
 * it.
 * Flags also have a store of up to 8 wares.
 *
 * You can also assign an arbitrary number of "jobs" for a flag.
 * A job consists of a request for a worker, and the name of a program that the
 * worker is to execute. Once execution of the program has finished, the worker
 * will return to a warehouse.
 *
 * Important: Do not access m_roads directly. get_road() and others use
 * WALK_xx in all "direction" parameters.
 */
struct Flag : public PlayerImmovable, public RoutingNode {
	using Wares = std::vector<const WareInstance *>;

	friend class Economy;
	friend class FlagQueue;
	friend class MapFlagdataPacket; // has to read/write this to a file
	friend struct MapWarePacket;     // has to look at pending wares
	friend struct MapWaredataPacket; // has to look at pending wares
	friend struct Router;

	const FlagDescr& descr() const;

	Flag(); /// empty flag for savegame loading
	Flag(EditorGameBase &, Player & owner, Coords); /// create a new flag
	~Flag() override;

	void load_finish(EditorGameBase &) override;
	void destroy(EditorGameBase &) override;

	int32_t  get_size    () const override;
	bool get_passable() const override;

	Flag & base_flag() override;

	const Coords & get_position() const override {return m_position;}
	PositionList get_positions (const EditorGameBase &) const override;
	void get_neighbours(WareWorker type, RoutingNodeNeighbours &) override;
	int32_t get_waitcost() const {return m_ware_filled;}

	void set_economy(Economy *) override;

	Building * get_building() const {return m_building;}
	void attach_building(EditorGameBase &, Building &);
	void detach_building(EditorGameBase &);

	bool has_road() const {
		return
			m_roads[0] || m_roads[1] || m_roads[2] ||
			m_roads[3] || m_roads[4] || m_roads[5];
	}
	Road * get_road(uint8_t const dir) const {return m_roads[dir - 1];}
	uint8_t nr_of_roads() const;
	void attach_road(int32_t dir, Road *);
	void detach_road(int32_t dir);

	Road * get_road(Flag &);

	bool is_dead_end() const;

	bool has_capacity() const;
	uint32_t total_capacity() {return m_ware_capacity;}
	uint32_t current_wares() const {return m_ware_filled;}
	void wait_for_capacity(Game &, Worker &);
	void skip_wait_for_capacity(Game &, Worker &);
	void add_ware(EditorGameBase &, WareInstance &);
	bool has_pending_ware(Game &, Flag & destflag);
	bool ack_pickup(Game &, Flag & destflag);
	bool cancel_pickup(Game &, Flag & destflag);
	WareInstance * fetch_pending_ware(Game &, PlayerImmovable & dest);
	Wares get_wares();

	void call_carrier(Game &, WareInstance &, PlayerImmovable * nextstep);
	void update_wares(Game &, Flag * other);

	void remove_ware(EditorGameBase &, WareInstance * const);

	void add_flag_job(Game &, DescriptionIndex workerware, const std::string & programname);

	void log_general_info(const EditorGameBase &) override;

protected:
	void init(EditorGameBase &) override;
	void cleanup(EditorGameBase &) override;

	void draw(const EditorGameBase &, RenderTarget &, const FCoords&, const Point&) override;

	void wake_up_capacity_queue(Game &);

	static void flag_job_request_callback(Game &, Request &, DescriptionIndex, Worker *, PlayerImmovable &);

	void set_flag_position(Coords coords);

private:
	struct PendingWare {
		WareInstance    * ware;     ///< the ware itself
		bool              pending;  ///< if the ware is pending
		int32_t           priority;  ///< carrier prefers the ware with highest priority
		OPtr<PlayerImmovable> nextstep; ///< next step that this ware is sent to
	};

	struct FlagJob {
		Request *   request;
		std::string program;
	};

	Coords       m_position;
	int32_t      m_animstart;

	Building    * m_building; ///< attached building (replaces road WALK_NW)
	Road        * m_roads[6]; ///< WALK_xx - 1 as index

	int32_t      m_ware_capacity; ///< size of m_wares array
	int32_t      m_ware_filled; ///< number of wares currently on the flag
	PendingWare * m_wares;    ///< wares currently on the flag

	/// call_carrier() will always call a carrier when the destination is
	/// the given flag
	Flag        * m_always_call_for_flag;

	using CapacityWaitQueue = std::vector<OPtr<Worker>>;
	CapacityWaitQueue m_capacity_wait; ///< workers waiting for capacity

	using FlagJobs = std::list<FlagJob>;
	FlagJobs m_flag_jobs;
};

extern FlagDescr g_flag_descr;
}

#endif  // end of include guard: WL_ECONOMY_FLAG_H
