/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef WL_LOGIC_PRODUCTIONSITE_H
#define WL_LOGIC_PRODUCTIONSITE_H

#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/macros.h"
#include "logic/bill_of_materials.h"
#include "logic/building.h"
#include "logic/production_program.h"
#include "logic/program_result.h"
#include "scripting/lua_table.h"

namespace Widelands {

struct ProductionProgram;
class Request;
class Soldier;
class WareDescr;
class WaresQueue;
class WorkerDescr;


/**
 * Every building that is part of the economics system is a production site.
 *
 * A production site has a worker.
 * A production site can have one (or more) output wares types (in theory it
 * should be possible to burn wares for some virtual result such as "mana", or
 *  maybe even just for the fun of it, although that's not planned).
 * A production site can have one (or more) input wares types. Every input
 * wares type has an associated store.
 */
class ProductionSiteDescr : public BuildingDescr {
public:
	friend struct ProductionProgram; // To add animations

	ProductionSiteDescr(const std::string& init_descname, const char* msgctxt, MapObjectType type,
							  const LuaTable& t, const EditorGameBase& egbase);
	ProductionSiteDescr(const std::string& init_descname, const char* msgctxt,
							  const LuaTable& t, const EditorGameBase& egbase);

	Building & create_object() const override;

	uint32_t nr_working_positions() const {
		uint32_t result = 0;
		for (const WareAmount& working_pos : working_positions()) {
			result += working_pos.second;
		}
		return result;
	}
	const BillOfMaterials & working_positions() const {
		return m_working_positions;
	}
	bool is_output_ware_type  (const DescriptionIndex& i) const {
		return m_output_ware_types  .count(i);
	}
	bool is_output_worker_type(const DescriptionIndex& i) const {
		return m_output_worker_types.count(i);
	}
	const BillOfMaterials & inputs() const {return m_inputs;}
	using Output = std::set<DescriptionIndex>;
	const Output   & output_ware_types  () const {return m_output_ware_types;}
	const Output   & output_worker_types() const {return m_output_worker_types;}
	const ProductionProgram * get_program(const std::string &) const;
	using Programs = std::map<std::string, std::unique_ptr<ProductionProgram>>;
	const Programs & programs() const {return m_programs;}

	const std::string& out_of_resource_title() const {
		return m_out_of_resource_title;
	}

	const std::string& out_of_resource_message() const {
		return m_out_of_resource_message;
	}
	uint32_t out_of_resource_productivity_threshold() const {
		return out_of_resource_productivity_threshold_;
	}

private:
	BillOfMaterials m_working_positions;
	BillOfMaterials m_inputs;
	Output   m_output_ware_types;
	Output   m_output_worker_types;
	Programs m_programs;
	std::string m_out_of_resource_title;
	std::string m_out_of_resource_message;
	int         out_of_resource_productivity_threshold_;

	DISALLOW_COPY_AND_ASSIGN(ProductionSiteDescr);
};

class ProductionSite : public Building {
	friend class MapBuildingdataPacket;
	friend struct ProductionProgram::ActReturn;
	friend struct ProductionProgram::ActReturn::WorkersNeedExperience;
	friend struct ProductionProgram::ActCall;
	friend struct ProductionProgram::ActWorker;
	friend struct ProductionProgram::ActSleep;
	friend struct ProductionProgram::ActCheckMap;
	friend struct ProductionProgram::ActAnimate;
	friend struct ProductionProgram::ActConsume;
	friend struct ProductionProgram::ActProduce;
	friend struct ProductionProgram::ActRecruit;
	friend struct ProductionProgram::ActMine;
	friend struct ProductionProgram::ActCheckSoldier;
	friend struct ProductionProgram::ActTrain;
	friend struct ProductionProgram::ActPlayFX;
	friend struct ProductionProgram::ActConstruct;
	MO_DESCR(ProductionSiteDescr)

public:
	ProductionSite(const ProductionSiteDescr & descr);
	virtual ~ProductionSite();

	void log_general_info(const EditorGameBase &) override;

	bool is_stopped() const {return m_is_stopped;}
	void set_stopped(bool);

	struct WorkingPosition {
		WorkingPosition(Request * const wr = nullptr, Worker * const w = nullptr)
			: worker_request(wr), worker(w)
		{}
		Request * worker_request;
		Worker  * worker;
	};

	WorkingPosition const * working_positions() const {
		return m_working_positions;
	}

	virtual bool has_workers(DescriptionIndex targetSite, Game & game);
	uint8_t get_statistics_percent() {return m_last_stat_percent;}
	uint8_t get_crude_statistics() {return (m_crude_percent + 5000) / 10000;}


	const std::string& production_result() const {return m_production_result;}

	 // Production and worker programs set this to explain the current
	 // state of the production. This string is shown as a tooltip
	 // when the mouse hovers over the building.
	 void set_production_result(const std::string& text) {
		m_production_result = text;
	}

	WaresQueue & waresqueue(DescriptionIndex) override;

	void init(EditorGameBase &) override;
	void cleanup(EditorGameBase &) override;
	void act(Game &, uint32_t data) override;

	void remove_worker(Worker &) override;
	int warp_worker(EditorGameBase &, const WorkerDescr & wd);

	bool fetch_from_flag(Game &) override;
	bool get_building_work(Game &, Worker &, bool success) override;

	void set_economy(Economy *) override;

	using InputQueues = std::vector<WaresQueue *>;
	const InputQueues & warequeues() const {return m_input_queues;}
	const std::vector<Worker *>& workers() const;

	bool can_start_working() const;

	/// sends a message to the player e.g. if the building's resource can't be found
	void notify_player(Game& game, uint8_t minutes);
	void unnotify_player();

	void set_default_anim(std::string);

protected:
	void update_statistics_string(std::string* statistics) override;

	void create_options_window
		(InteractiveGameBase &, UI::Window * & registry) override;


	void load_finish(EditorGameBase & egbase) override;

protected:
	struct State {
		const ProductionProgram * program; ///< currently running program
		int32_t  ip; ///< instruction pointer
		uint32_t phase; ///< micro-step index (instruction dependent)
		uint32_t flags; ///< pfXXX flags

		/**
		 * Instruction-dependent additional data.
		 */
		/*@{*/
		ObjectPointer objvar;
		Coords coord;
		/*@}*/

		State() :
			program(nullptr),
			ip(0),
			phase(0),
			flags(0),
			coord(Coords::null()) {}
	};

	Request & request_worker(DescriptionIndex);
	static void request_worker_callback
		(Game &, Request &, DescriptionIndex, Worker *, PlayerImmovable &);

	/**
	 * Determine the next program to be run when the last program has finished.
	 * The default implementation starts program "work".
	 */
	virtual void find_and_start_next_program(Game &);

	State & top_state() {assert(m_stack.size()); return *m_stack.rbegin();}
	State * get_state() {return m_stack.size() ? &*m_stack.rbegin() : nullptr;}
	void program_act(Game &);

	/// \param phase can be used to pass a value on to the next step in the
	/// program. For example if one step is a mine command, it can calculate
	/// how long it should take to mine, given the particular circumstances,
	/// and pass the result to the following animation command, to set the
	/// duration.
	void program_step(Game &, uint32_t delay = 10, uint32_t phase = 0);

	void program_start(Game &, const std::string & program_name);
	virtual void program_end(Game &, ProgramResult);
	virtual void train_workers(Game &);

	void calc_statistics();
	void try_start_working(Game &);
	void set_post_timer (int32_t const t) {m_post_timer = t;}

protected:  // TrainingSite must have access to this stuff
	WorkingPosition                   * m_working_positions;

	int32_t m_fetchfromflag; ///< Number of wares to fetch from flag

	/// If a program has ended with the result Skipped, that program may not
	/// start again until a certain time has passed. This is a map from program
	/// name to game time. When a program ends with the result Skipped, its name
	/// is added to this map, with the current game time. (When the program ends
	/// with any other result, its name is removed from the map.)
	using SkippedPrograms = std::map<std::string, Time>;
	SkippedPrograms m_skipped_programs;

	using Stack = std::vector<State>;
	Stack        m_stack; ///<  program stack
	bool         m_program_timer; ///< execute next instruction based on pointer
	int32_t      m_program_time; ///< timer time
	int32_t      m_post_timer;    ///< Time to schedule after ends

	ProductionProgram::ActProduce::Items m_produced_wares;
	ProductionProgram::ActProduce::Items m_recruited_workers;
	InputQueues m_input_queues; ///< input queues for all inputs
	std::vector<bool>        m_statistics;
	uint8_t                  m_last_stat_percent;
	uint32_t                 m_crude_percent; //integer0-10000000, to be shirink to range 0-10
	bool                     m_is_stopped;
	std::string              m_default_anim; // normally "idle", "empty", if empty mine.

private:
	enum class Trend {kUnchanged, kRising, kFalling};
	Trend                    trend_;
	std::string              m_statistics_string_on_changed_statistics;
	std::string              m_production_result; // hover tooltip text

	DISALLOW_COPY_AND_ASSIGN(ProductionSite);
};

/**
 * Describes, how many wares of a certain ware can be stored in a house.
 *
 * This class will be extended to support ordering of certain wares directly or
 * releasing some wares out of a building
*/
struct Input {
	Input(const DescriptionIndex& Ware, uint8_t const Max) : m_ware(Ware), m_max(Max)
	{}
	~Input() {}

	DescriptionIndex ware() const {return m_ware;}
	uint8_t     max() const {return m_max;}

private:
	DescriptionIndex m_ware;
	uint8_t    m_max;
};

/**
 * Note to be published when a production site is out of resources
 */
// A note we're using to notify the AI
struct NoteProductionSiteOutOfResources {
	CAN_BE_SENT_AS_NOTE(NoteId::ProductionSiteOutOfResources)

	// The production site that is out of resources.
	ProductionSite* ps;

	// The player that owns the production site.
	Player * player;

	NoteProductionSiteOutOfResources(ProductionSite* const init_ps, Player* init_player)
		: ps(init_ps), player(init_player) {
	}
};

}

#endif  // end of include guard: WL_LOGIC_PRODUCTIONSITE_H
