/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LUA_NOTES_H
#define WL_SCRIPTING_LUA_NOTES_H

#include <string>

#include "base/rect.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"

namespace LuaGame {

struct NotePlayerSettings {
	CAN_BE_SENT_AS_NOTE(NoteId::LuaGamePlayerSettings)

	enum class Action { kSwitchPlayer, kRevealCampaign, kRevealScenario };
	const Action action;

	const Widelands::PlayerNumber player;
	const Widelands::PlayerNumber new_player;  // For switch player
	const std::string visibility_entry;        // For campaign/scenario visibility

	NotePlayerSettings(const Action& init_action,
	                   Widelands::PlayerNumber init_player,
	                   Widelands::PlayerNumber init_new_player,
	                   const std::string& init_visibility_entry = "")
	   : action(init_action),
	     player(init_player),
	     new_player(init_new_player),
	     visibility_entry(init_visibility_entry) {
	}
};

struct NoteStoryMessage {
	CAN_BE_SENT_AS_NOTE(NoteId::LuaGameStoryMessage)

	const Widelands::PlayerNumber player;
	const std::string title;
	const std::string body;
	const std::string button_text;
	const Recti dimensions;
	const Widelands::Coords scrollto;

	NoteStoryMessage(Widelands::PlayerNumber init_player,
	                 const std::string& init_title,
	                 const std::string& init_body,
	                 const std::string& init_button_text,
	                 const Recti& init_dimensions,
	                 const Widelands::Coords& init_scrollto)
	   : player(init_player),
	     title(init_title),
	     body(init_body),
	     button_text(init_button_text),
	     dimensions(init_dimensions),
	     scrollto(init_scrollto) {
	}
};

}  // namespace LuaGame
#endif  // end of include guard: WL_SCRIPTING_LUA_NOTES_H
