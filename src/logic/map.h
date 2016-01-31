/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_H
#define WL_LOGIC_MAP_H

#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "economy/itransport_cost_calculator.h"
#include "logic/field.h"
#include "logic/description_maintainer.h"
#include "logic/map_revision.h"
#include "logic/objective.h"
#include "logic/map_objects/walkingdir.h"
#include "logic/widelands_geometry.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"
#include "random/random.h"

class FileSystem;
class Image;
struct S2MapLoader;

namespace Widelands {

class MapLoader;
class Objective;
class World;
struct BaseImmovable;
struct MapGenerator;
struct PathfieldManager;

#define WLMF_SUFFIX ".wmf"
#define S2MF_SUFFIX ".swd"
#define S2MF_SUFFIX2 ".wld"

#define S2MF_MAGIC  "WORLD_V1.0"

// Global list of available map dimensions.
const std::vector<int32_t> kMapDimensions = {
	64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512
};


struct Path;
class Immovable;

struct NoteFieldTerrainChanged {
	CAN_BE_SENT_AS_NOTE(NoteId::FieldTerrainChanged)

	FCoords fc;
	MapIndex map_index;
};

/// Send when the resource of a field is changed.
struct NoteFieldResourceChanged {
	CAN_BE_SENT_AS_NOTE(NoteId::FieldResourceTypeChanged)

	FCoords fc;
	DescriptionIndex old_resource;
	uint8_t old_initial_amount;
	uint8_t old_amount;
};

struct ImmovableFound {
	BaseImmovable * object;
	Coords          coords;
};

/*
FindImmovable
FindBob
FindNode
FindResource
CheckStep

Predicates used in path finding and find functions.
*/
struct FindImmovable;
const FindImmovable & find_immovable_always_true();

struct FindBob {
	//  Return true if this bob should be returned by find_bobs.
	virtual bool accept(Bob *) const = 0;
	virtual ~FindBob() {}  // make gcc shut up
};
struct FindNode;
struct CheckStep;

/*
Some very simple default predicates (more predicates below Map).
*/
struct FindBobAlwaysTrue : public FindBob {
	bool accept(Bob *) const override {return true;}
	virtual ~FindBobAlwaysTrue() {}  // make gcc shut up
};

/** class Map
 *
 * This really identifies a map like it is in the game
 *
 * Odd rows are shifted FIELD_WIDTH/2 to the right. This means that moving
 * up and down depends on the row numbers:
 *               even   odd
 * top-left      -1/-1   0/-1
 * top-right      0/-1  +1/-1
 * bottom-left   -1/+1   0/+1
 * bottom-right   0/+1  +1/+1
 *
 * Warning: width and height must be even
 */
class Map : public ITransportCostCalculator {
public:
	friend class Editor;
	friend class EditorGameBase;
	friend class MapLoader;
	friend class MapVersionPacket;
	friend struct ::S2MapLoader;
	friend struct MainMenuNewMap;
	friend struct MapAStarBase;
	friend struct MapGenerator;
	friend struct MapElementalPacket;
	friend struct WidelandsMapLoader;

	using PortSpacesSet = std::set<Coords, Coords::OrderingFunctor>;
	using Objectives = std::map<std::string, std::unique_ptr<Objective>>;
	using SuggestedTeam = std::vector<uint16_t>;             // Players in a team
	using SuggestedTeamLineup = std::vector<SuggestedTeam>; // Recommended teams to play against each other


	enum { // flags for findpath()

		//  use bidirection cost instead of normal cost calculations
		//  should be used for road building
		fpBidiCost = 1,
	};

	// ORed bits for scenario types
	using ScenarioTypes = size_t;
	enum {
		NO_SCENARIO = 0,
		SP_SCENARIO = 1,
		MP_SCENARIO = 2 };

	Map ();
	virtual ~Map();

	/// Returns the correct initialized loader for the given mapfile
	std::unique_ptr<MapLoader> get_correct_loader(const std::string& filename);

	void cleanup();

	void create_empty_map  // for editor
	   (const World& world,
	    uint32_t w = 64,
	    uint32_t h = 64,
		 const Widelands::DescriptionIndex default_terrain = 0,
		 const std::string& name = _("No Name"),
		 const std::string& author = pgettext("author_name", "Unknown"),
		 const std::string& description = _("No description defined"));

	void recalc_whole_map(const World& world);
	virtual void recalc_for_field_area(const World& world, Area<FCoords>);

	/***
	 * Ensures that resources match their adjacent terrains.
	 */
	void ensure_resource_consistency(const World& world);

	/***
	 * Recalculates all default resources.
	 *
	 * This is just needed for the game, not for
	 * the editor. Since there, default resources
	 * are not shown.
	 */
	void recalc_default_resources(const World& world);

	void set_nrplayers(PlayerNumber);

	void set_starting_pos(PlayerNumber, Coords);
	Coords get_starting_pos(PlayerNumber const p) const {
		assert(1 <= p && p <= get_nrplayers());
		return m_starting_pos[p - 1];
	}

	void set_filename   (const std::string& filename);
	void set_author     (const std::string& author);
	void set_name       (const std::string& name);
	void set_description(const std::string& description);
	void set_hint       (const std::string& hint);
	void set_background (const std::string& image_path);
	void add_tag        (const std::string& tag);
	void delete_tag     (const std::string& tag);
	void set_scenario_types(ScenarioTypes t) {m_scenario_types = t;}

	// Allows access to the filesystem of the map to access auxiliary files.
	// This can be nullptr if this file is new.
	FileSystem* filesystem() const;
	// swap the filesystem after load / save
	void swap_filesystem(std::unique_ptr<FileSystem>& fs);

	// informational functions
	const std::string& get_filename()    const {return m_filename;}
	const std::string& get_author()      const {return m_author;}
	const std::string& get_name()        const {return m_name;}
	const std::string& get_description() const {return m_description;}
	const std::string& get_hint()        const {return m_hint;}
	const std::string& get_background()  const {return m_background;}

	using Tags = std::set<std::string>;
	const Tags & get_tags() const {return m_tags;}
	void clear_tags() {m_tags.clear();}
	bool has_tag(const std::string& s) const {return m_tags.count(s);}

	const std::vector<SuggestedTeamLineup>& get_suggested_teams() const {return m_suggested_teams;}

	PlayerNumber get_nrplayers() const {return m_nrplayers;}
	ScenarioTypes scenario_types() const {return m_scenario_types;}
	Extent extent() const {return Extent(m_width, m_height);}
	int16_t get_width   () const {return m_width;}
	int16_t get_height  () const {return m_height;}

	//  The next few functions are only valid when the map is loaded as a
	//  scenario.
	const std::string & get_scenario_player_tribe    (PlayerNumber) const;
	const std::string & get_scenario_player_name     (PlayerNumber) const;
	const std::string & get_scenario_player_ai       (PlayerNumber) const;
	bool                get_scenario_player_closeable(PlayerNumber) const;
	void set_scenario_player_tribe    (PlayerNumber, const std::string &);
	void set_scenario_player_name     (PlayerNumber, const std::string &);
	void set_scenario_player_ai       (PlayerNumber, const std::string &);
	void set_scenario_player_closeable(PlayerNumber, bool);

	/// \returns the maximum theoretical possible nodecaps (no blocking bobs, etc.)
	NodeCaps get_max_nodecaps(const World& world, FCoords &);

	BaseImmovable * get_immovable(Coords) const;
	uint32_t find_bobs
		(const Area<FCoords>,
		 std::vector<Bob *> * list,
		 const FindBob & functor = FindBobAlwaysTrue());
	uint32_t find_reachable_bobs
		(const Area<FCoords>,
		 std::vector<Bob *> * list,
		 const CheckStep &,
		 const FindBob & functor = FindBobAlwaysTrue());
	uint32_t find_immovables
		(const Area<FCoords>,
		 std::vector<ImmovableFound> * list,
		 const FindImmovable & = find_immovable_always_true());
	uint32_t find_reachable_immovables
		(const Area<FCoords>,
		 std::vector<ImmovableFound> * list,
		 const CheckStep &,
		 const FindImmovable & = find_immovable_always_true());
	uint32_t find_reachable_immovables_unique
		(const Area<FCoords>,
		 std::vector<BaseImmovable *> & list,
		 const CheckStep &,
		 const FindImmovable & = find_immovable_always_true());
	uint32_t find_fields
		(const Area<FCoords>,
		 std::vector<Coords> * list,
		 const FindNode & functor);
	uint32_t find_reachable_fields
		(const Area<FCoords>,
		 std::vector<Coords>* list,
		 const CheckStep &,
		 const FindNode &);

	// Field logic
	static MapIndex get_index(const Coords &, int16_t width);
	MapIndex max_index() const {return m_width * m_height;}
	Field & operator[](MapIndex) const;
	Field & operator[](const Coords &) const;
	FCoords get_fcoords(const Coords &) const;
	void normalize_coords(Coords &) const;
	FCoords get_fcoords(Field &) const;
	void get_coords(Field & f, Coords & c) const;

	uint32_t calc_distance(Coords, Coords) const;

	int32_t calc_cost_estimate(Coords, Coords) const override;
	int32_t calc_cost_lowerbound(Coords, Coords) const;
	int32_t calc_cost(int32_t slope) const;
	int32_t calc_cost(Coords, int32_t dir) const;
	int32_t calc_bidi_cost(Coords, int32_t dir) const;
	void calc_cost(const Path &, int32_t * forward, int32_t * backward) const;

	void get_ln  (const Coords &,  Coords *) const;
	void get_ln (const FCoords &, FCoords *) const;
	Coords  l_n  (const Coords &) const;
	FCoords l_n (const FCoords &) const;
	void get_rn  (const Coords &,  Coords *) const;
	void get_rn (const FCoords &, FCoords *) const;
	Coords  r_n  (const Coords &) const;
	FCoords r_n (const FCoords &) const;
	void get_tln (const Coords &,  Coords *) const;
	void get_tln(const FCoords &, FCoords *) const;
	Coords  tl_n (const Coords &) const;
	FCoords tl_n(const FCoords &) const;
	void get_trn (const Coords &,  Coords *) const;
	void get_trn(const FCoords &, FCoords *) const;
	Coords  tr_n (const Coords &) const;
	FCoords tr_n(const FCoords &) const;
	void get_bln (const Coords &,  Coords *) const;
	void get_bln(const FCoords &, FCoords *) const;
	Coords  bl_n (const Coords &) const;
	FCoords bl_n(const FCoords &) const;
	void get_brn (const Coords &,  Coords *) const;
	void get_brn(const FCoords &, FCoords *) const;
	Coords  br_n (const Coords &) const;
	FCoords br_n(const FCoords &) const;

	void get_neighbour (const Coords &, Direction dir,  Coords *) const;
	void get_neighbour(const FCoords &, Direction dir, FCoords *) const;
	FCoords get_neighbour(const FCoords &, Direction dir) const;

	// Pathfinding
	int32_t findpath
		(Coords instart,
		 Coords inend,
		 const int32_t persist,
		 Path &,
		 const CheckStep &,
		 const uint32_t flags = 0);

	/**
	 * We can reach a field by water either if it has MOVECAPS_SWIM or if it has
	 * MOVECAPS_WALK and at least one of the neighbours has MOVECAPS_SWIM
	 */
	bool can_reach_by_water(Coords) const;

	/// Sets the height to a value. Recalculates brightness. Changes the
	/// surrounding nodes if necessary. Returns the radius that covers all
	/// changes that were made.
	///
	/// Do not call this to set the height of each node in an area to the same
	/// value, because it adjusts the heights of surrounding nodes in each call,
	/// so it will be terribly slow. Use set_height for Area for that purpose
	/// instead.
	uint32_t set_height(const World& world, FCoords, Field::Height);

	/// Changes the height of the nodes in an Area by a difference.
	uint32_t change_height(const World& world, Area<FCoords>, int16_t difference);

	/// Initializes the 'initial_resources' on 'coords' to the 'resource_type'
	/// with the given 'amount'.
	void initialize_resources(const FCoords& coords, DescriptionIndex resource_type, uint8_t amount);

	/// Sets the number of resources of the field to 'amount'. The type of the
	/// resource on this field is not changed.
	void set_resources(const FCoords& coords, uint8_t amount);

	/// Clears the resources, i.e. the amount will be set to 0 and the type of
	/// resources will be kNoResource.
	void clear_resources(const FCoords& coords);

	/**
	 * Ensures that the height of each node within radius from fc is in
	 * height_interval. If the height is < height_interval.min, it is changed to
	 * height_interval.min. If the height is > height_interval.max, it is changed
	 * to height_interval.max. Otherwise it is left unchanged.
	 *
	 * Recalculates brightness. Changes the surrounding nodes if necessary.
	 * Returns the radius of the area that covers all changes that were made.
	 *
	 * Calling this is much faster than calling change_height for each node in
	 * the area, because this adjusts the surrounding nodes only once, after all
	 * nodes in the area had their new height set.
	 */
	uint32_t set_height(const World& world, Area<FCoords>, HeightInterval height_interval);

	/***
	 * Changes the given triangle's terrain. This happens in the editor and might
	 * happen in the game too if some kind of land increasement is implemented (like
	 * drying swamps). The nodecaps need to be recalculated
	 *
	 * @return the radius of changes (which are always 2)
	 */
	int32_t change_terrain(const World& world, TCoords<FCoords>, DescriptionIndex);

	/***
	 * Verify if a resource attached to a vertex has enough adjacent matching terrains to be valid.
	 *
	 * To qualify as valid, resources need to be surrounded by at least two matching terrains.
	 */
	bool is_resource_valid
		(const Widelands::World& world, const Widelands::TCoords<Widelands::FCoords>& c,
		int32_t const curres);

	// The objectives that are defined in this map if it is a scenario.
	const Objectives& objectives() const {
		return objectives_;
	}
	Objectives* mutable_objectives() {
		return &objectives_;
	}

	/// Returns the military influence on a location from an area.
	MilitaryInfluence calc_influence(Coords, Area<>) const;

	/// Translate the whole map so that the given point becomes the new origin.
	void set_origin(Coords);

	/// Port space specific functions
	bool is_port_space(const Coords& c) const;
	void set_port_space(Coords c, bool allowed);
	const PortSpacesSet& get_port_spaces() const {return m_port_spaces;}
	std::vector<Coords> find_portdock(const Widelands::Coords& c) const;
	bool allows_seafaring();
	bool has_artifacts(const World& world);

protected: /// These functions are needed in Testclasses
	void set_size(uint32_t w, uint32_t h);

private:
	void recalc_border(FCoords);

	/// # of players this map supports (!= Game's number of players!)
	PlayerNumber m_nrplayers;
	ScenarioTypes m_scenario_types; // whether the map is playable as scenario

	int16_t m_width;
	int16_t m_height;
	std::string m_filename;
	std::string m_author;
	std::string m_name;
	std::string m_description;
	std::string m_hint;
	std::string m_background;
	Tags        m_tags;
	std::vector<SuggestedTeamLineup> m_suggested_teams;

	std::vector<Coords> m_starting_pos;    //  players' starting positions

	std::unique_ptr<Field[]> m_fields;

	std::unique_ptr<PathfieldManager> m_pathfieldmgr;
	std::vector<std::string> m_scenario_tribes;
	std::vector<std::string> m_scenario_names;
	std::vector<std::string> m_scenario_ais;
	std::vector<bool>        m_scenario_closeables;

	// The map file as a filesystem.
	std::unique_ptr<FileSystem> filesystem_;

	PortSpacesSet m_port_spaces;
	Objectives objectives_;

	void recalc_brightness(FCoords);
	void recalc_nodecaps_pass1(const World& world, FCoords);
	void recalc_nodecaps_pass2(const World& world, const FCoords & f);
	NodeCaps _calc_nodecaps_pass1(const World& world, FCoords, bool consider_mobs = true);
	NodeCaps _calc_nodecaps_pass2(const World& world,
	                              FCoords,
	                              bool consider_mobs = true,
	                              NodeCaps initcaps = CAPS_NONE);
	void check_neighbour_heights(FCoords, uint32_t & radius);
	int calc_buildsize
		(const World& world, const FCoords& f, bool avoidnature, bool * ismine = nullptr,
		 bool consider_mobs = true, NodeCaps initcaps = CAPS_NONE);
	bool is_cycle_connected
		(const FCoords & start, uint32_t length, const WalkingDir * dirs);
	template<typename functorT>
		void find_reachable(Area<FCoords>, const CheckStep &, functorT &);
	template<typename functorT> void find(const Area<FCoords>, functorT &) const;

	MapVersion m_map_version;
};


/*
==============================================================================

Field arithmetics

==============================================================================
*/

inline MapIndex Map::get_index(const Coords & c, int16_t const width) {
	assert(0 < width);
	assert(0 <= c.x);
	assert     (c.x < width);
	assert(0 <= c.y);
	return c.y * width + c.x;
}

inline Field & Map::operator[](MapIndex const i) const {return m_fields[i];}
inline Field & Map::operator[](const Coords & c) const {
	return operator[](get_index(c, m_width));
}

inline FCoords Map::get_fcoords(const Coords & c) const
{
	return FCoords(c, &operator[](c));
}

inline void Map::normalize_coords(Coords & c) const
{
	while (c.x < 0)         c.x += m_width;
	while (c.x >= m_width)  c.x -= m_width;
	while (c.y < 0)         c.y += m_height;
	while (c.y >= m_height) c.y -= m_height;
}


/**
 * Calculate the field coordates from the pointer
 */
inline FCoords Map::get_fcoords(Field & f) const {
	const int32_t i = &f - m_fields.get();
	return FCoords(Coords(i % m_width, i / m_width), &f);
}
inline void Map::get_coords(Field & f, Coords & c) const {c = get_fcoords(f);}


/** get_ln, get_rn, get_tln, get_trn, get_bln, get_brn
 *
 * Calculate the coordinates and Field pointer of a neighboring field.
 * Assume input coordinates are valid.
 *
 * Note: Input coordinates are passed as value because we have to allow
 *       usage get_XXn(foo, &foo).
 */
inline void Map::get_ln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y;
	o->x = (f.x ? f.x : m_width) - 1;
	assert(0 <= o->x);
	assert(0 <= o->y);
	assert(o->x < m_width);
	assert(o->y < m_height);
}

inline void Map::get_ln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert            (f.field < m_fields.get() + max_index());
	o->y = f.y;
	o->x = f.x - 1;
	o->field = f.field - 1;
	if (o->x == -1) {
		o->x = m_width - 1;
		o->field += m_width;
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields.get() <= o->field);
	assert(o->field < m_fields.get() + max_index());
}
inline Coords Map::l_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x - 1, f.y);
	if (result.x == -1)
		result.x = m_width - 1;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::l_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	FCoords result(Coords(f.x - 1, f.y), f.field - 1);
	if (result.x == -1) {
		result.x = m_width - 1;
		result.field += m_width;
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields.get() <= result.field);
	assert(result.field < m_fields.get() + max_index());
	return result;
}

inline void Map::get_rn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y;
	o->x = f.x + 1;
	if (o->x == m_width)
		o->x = 0;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_rn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert            (f.field < m_fields.get() + max_index());
	o->y = f.y;
	o->x = f.x + 1;
	o->field = f.field + 1;
	if (o->x == m_width) {o->x = 0; o->field -= m_width;}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields.get() <= o->field);
	assert(o->field < m_fields.get() + max_index());
}
inline Coords Map::r_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x + 1, f.y);
	if (result.x == m_width)
		result.x = 0;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::r_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	FCoords result(Coords(f.x + 1, f.y), f.field + 1);
	if (result.x == m_width) {result.x = 0; result.field -= m_width;}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields.get() <= result.field);
	assert(result.field < m_fields.get() + max_index());
	return result;
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y - 1;
	o->x = f.x;
	if (o->y & 1) {
		if (o->y == -1)
			o->y = m_height - 1;
		o->x = (o->x ? o->x : m_width) - 1;
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_tln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	o->y = f.y - 1;
	o->x = f.x;
	o->field = f.field - m_width;
	if (o->y & 1) {
		if (o->y == -1) {
			o->y = m_height - 1;
			o->field += max_index();
		}
		--o->x;
		--o->field;
		if (o->x == -1) {
			o->x = m_width - 1;
			o->field += m_width;
		}
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields.get() <= o->field);
	assert(o->field < m_fields.get() + max_index());
}
inline Coords Map::tl_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y - 1);
	if (result.y & 1) {
		if (result.y == -1)
			result.y = m_height - 1;
		--result.x;
		if (result.x == -1)
			result.x = m_width  - 1;
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::tl_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	FCoords result(Coords(f.x, f.y - 1), f.field - m_width);
	if (result.y & 1) {
		if (result.y == -1) {
			result.y = m_height - 1;
			result.field += max_index();
		}
		--result.x;
		--result.field;
		if (result.x == -1) {
			result.x = m_width - 1;
			result.field += m_width;
		}
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields.get() <= result.field);
	assert(result.field < m_fields.get() + max_index());
	return result;
}

// top-right: even: 0/-1  odd: +1/-1
inline void Map::get_trn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->x = f.x;
	if (f.y & 1) {
		++o->x;
		if (o->x == m_width)
			o->x = 0;
	}
	o->y = (f.y ? f.y : m_height) - 1;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_trn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	o->x = f.x;
	o->field = f.field - m_width;
	if (f.y & 1) {
		++o->x;
		++o->field;
		if (o->x == m_width) {
			o->x = 0;
			o->field -= m_width;
		}
	}
	o->y = f.y - 1;
	if (o->y == -1) {
		o->y = m_height - 1;
		o->field += max_index();
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields.get() <= o->field);
	assert(o->field < m_fields.get() + max_index());
}
inline Coords Map::tr_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y - 1);
	if (f.y & 1) {
		++result.x;
		if (result.x == m_width)
			result.x = 0;
	}
	if (result.y == -1)
		result.y = m_height - 1;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::tr_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	FCoords result(Coords(f.x, f.y - 1), f.field - m_width);
	if (f.y & 1) {
		++result.x;
		++result.field;
		if (result.x == m_width) {
			result.x = 0;
			result.field -= m_width;
		}
	}
	if (result.y == -1) {
		result.y = m_height - 1;
		result.field += max_index();
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields.get() <= result.field);
	assert(result.field < m_fields.get() + max_index());
	return result;
}

// bottom-left: even: -1/+1  odd: 0/+1
inline void Map::get_bln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y + 1;
	o->x = f.x;
	if (o->y == m_height)
		o->y = 0;
	if (o->y & 1)
		o->x = (o->x ? o->x : m_width) - 1;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_bln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	o->y = f.y + 1;
	o->x = f.x;
	o->field = f.field + m_width;
	if (o->y == m_height) {
		o->y = 0;
		o->field -= max_index();
	}
	if (o->y & 1) {
		--o->x;
		--o->field;
		if (o->x == -1) {
			o->x = m_width - 1;
			o->field += m_width;
		}
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields.get() <= o->field);
	assert(o->field < m_fields.get() + max_index());
}
inline Coords Map::bl_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y + 1);
	if (result.y == m_height)
		result.y = 0;
	if (result.y & 1) {
		--result.x;
		if (result.x == -1)
			result.x = m_width - 1;
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::bl_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	FCoords result(Coords(f.x, f.y + 1), f.field + m_width);
	if (result.y == m_height) {
		result.y = 0;
		result.field -= max_index();
	}
	if (result.y & 1) {
		--result.x;
		--result.field;
		if (result.x == -1) {
			result.x = m_width - 1;
			result.field += m_width;
		}
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields.get() <= result.field);
	assert(result.field < m_fields.get() + max_index());
	return result;
}

// bottom-right: even: 0/+1  odd: +1/+1
inline void Map::get_brn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->x = f.x;
	if (f.y & 1) {
		++o->x;
		if (o->x == m_width)
			o->x = 0;
	}
	o->y = f.y + 1;
	if (o->y == m_height)
		o->y = 0;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_brn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	o->x = f.x;
	o->field = f.field + m_width;
	if (f.y & 1) {
		++o->x;
		++o->field;
		if (o->x == m_width) {
			o->x = 0;
			o->field -= m_width;
		}
	}
	o->y = f.y + 1;
	if (o->y == m_height) {
		o->y = 0;
		o->field -= max_index();
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields.get() <= o->field);
	assert(o->field < m_fields.get() + max_index());
}
inline Coords Map::br_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y + 1);
	if (f.y & 1) {
		++result.x;
		if (result.x == m_width)
			result.x = 0;
	}
	if (result.y == m_height)
		result.y = 0;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::br_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields.get() <= f.field);
	assert(f.field < m_fields.get() + max_index());
	FCoords result(Coords(f.x, f.y + 1), f.field + m_width);
	if (f.y & 1) {
		++result.x;
		++result.field;
		if (result.x == m_width) {
			result.x = 0;
			result.field -= m_width;
		}
	}
	if (result.y == m_height) {
		result.y = 0;
		result.field -= max_index();
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields.get() <= result.field);
	assert(result.field < m_fields.get() + max_index());
	return result;
}

inline FCoords Map::get_neighbour(const FCoords & f, const Direction dir) const
{
	switch (dir) {
	case WALK_NW: return tl_n(f);
	case WALK_NE: return tr_n(f);
	case WALK_E:  return  r_n(f);
	case WALK_SE: return br_n(f);
	case WALK_SW: return bl_n(f);
	//case WALK_W:  return  l_n(f);
	default:
	assert(WALK_W == dir);
	return l_n(f);
	}
}

inline void move_r(const int16_t mapwidth, FCoords & f) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth;}
	assert(f.x < mapwidth);
}

inline void move_r(int16_t const mapwidth, FCoords & f, MapIndex & i) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	++i;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth; i -= mapwidth;}
	assert(f.x < mapwidth);
}


#define iterate_Map_FCoords(map, extent, fc)                                  \
   for                                                                        \
      (Widelands::FCoords fc = (map).get_fcoords(Widelands::Coords(0, 0));    \
		 fc.y < static_cast<int16_t>(extent.h);                 \
       ++fc.y)                                                                \
      for                                                                     \
         (fc.x = 0;                                                           \
			 fc.x < static_cast<int16_t>(extent.w);              \
          ++fc.x, ++fc.field)                                                 \

}


#endif  // end of include guard: WL_LOGIC_MAP_H
