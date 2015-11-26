/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "map_io/map_elemental_packet.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 1;

void MapElementalPacket::pre_read(FileSystem & fs, Map * map)
{
	Profile prof;
	prof.read("elemental", nullptr, fs);
	Section & s = prof.get_safe_section("global");

	try {
		int32_t const packet_version = s.get_int("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			map->m_width       = s.get_int   ("map_w");
			map->m_height      = s.get_int   ("map_h");
			map->set_nrplayers  (s.get_int   ("nr_players"));
			map->set_name       (s.get_string("name"));
			map->set_author     (s.get_string("author"));
			map->set_description(s.get_string("descr"));
			map->set_hint       (s.get_string("hint", ""));
			map->set_background (s.get_string("background", ""));
			old_world_name_ = s.get_string("world", "");

			std::string t = s.get_string("tags", "");
			if (t != "") {
				std::vector<std::string> tags;
				boost::split(tags, t, boost::is_any_of(","));

				for (std::vector<std::string>::const_iterator ci = tags.begin(); ci != tags.end(); ++ci) {
					std::string tn = *ci;
					boost::trim(tn);
					map->add_tag(tn);
				}
			}

			// Get suggested teams
			map->m_suggested_teams.clear();

			uint16_t team_section_id = 0;
			std::string teamsection_key = (boost::format("teams%02i") % team_section_id).str();
			while (Section* teamsection = prof.get_section(teamsection_key.c_str())) {

				// A lineup is made up of teams
				Map::SuggestedTeamLineup lineup;

				uint16_t team_number = 1;
				std::string team_key = (boost::format("team%i") % team_number).str().c_str();
				std::string team_string = teamsection->get_string(team_key.c_str(), "");
				while (!team_string.empty()) {
					// A team is made up of players
					Map::SuggestedTeam team;

					std::vector<std::string> players_string;
					boost::split(players_string, team_string, boost::is_any_of(","));

					for (const std::string& player: players_string) {
						uint16_t player_number = static_cast<uint16_t>(atoi(player.c_str()));
						assert(player_number < MAX_PLAYERS);
						team.push_back(player_number);
					}

					lineup.push_back(team);

					// Increase team number
					++team_number;
					team_key = (boost::format("team%i") % team_number).str();
					team_string = teamsection->get_string(team_key.c_str(), "");
				}

				map->m_suggested_teams.push_back(lineup);

				// Increase teamsection
				++team_section_id;
				teamsection_key = (boost::format("teams%02i") % team_section_id).str().c_str();
			}
		} else
			throw UnhandledVersionError("MapElementalPacket", packet_version, kCurrentPacketVersion);
	} catch (const WException & e) {
		throw GameDataError("elemental data: %s", e.what());
	}
}


void MapElementalPacket::read
	(FileSystem & fs, EditorGameBase & egbase, bool, MapObjectLoader &)
{
	pre_read(fs, &egbase.map());
}


void MapElementalPacket::write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver &)
{

	Profile prof;
	Section & s = prof.create_section("global");

	s.set_int   ("packet_version", kCurrentPacketVersion);
	const Map & map = egbase.map();
	s.set_int   ("map_w",          map.get_width      ());
	s.set_int   ("map_h",          map.get_height     ());
	s.set_int   ("nr_players",     map.get_nrplayers  ());
	s.set_string("name",           map.get_name       ());
	s.set_string("author",         map.get_author     ());
	s.set_string("descr",          map.get_description());
	s.set_string("hint",           map.get_hint       ());
	if (!map.get_background().empty())
		s.set_string("background",  map.get_background ());
	s.set_string("tags", boost::algorithm::join(map.get_tags(), ","));

	prof.write("elemental", false, fs);
}

}
