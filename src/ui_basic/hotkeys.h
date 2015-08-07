/*
 * Copyright (C) 2015 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_HOTKEYS_H
#define WL_UI_BASIC_HOTKEYS_H

#include <map>

#include <SDL_keycode.h>

namespace UI {

struct Hotkeys {
	Hotkeys();

	bool has_hotkey(const std::string& scope, const std::string& key);
	bool has_code(const std::string& scope, const SDL_Keycode& code);
	bool add_hotkey(const std::string& scope,
	                const std::string& key,
	                const SDL_Keycode& code,
	                const std::string& title);
	bool replace_hotkey(const std::string& scope, const std::string& key, const SDL_Keycode& code);
	const SDL_Keycode& get_hotkey(const std::string& scope, const std::string& key);
	const std::string& get_hotkey_title(const std::string& scope, const std::string& key);
	const std::string get_displayname(const SDL_Keycode& code);

	// NOCOM comment stuff
	// key
	// scope
	// SDL_key

private:
	struct ScopeAndKey {
		ScopeAndKey();
		ScopeAndKey(const std::string& scope, const std::string& key) : scope_(scope), key_(key) {
		}

		bool operator<(const ScopeAndKey& other) const {
			if (scope_ == other.scope_) {
				return key_ < other.key_;
			}
			return scope_ < other.scope_;
		}

		bool operator==(const ScopeAndKey& other) const {
			return (scope_ == other.scope_ && key_ == other.key_);
		}

		std::string scope_;
		std::string key_;
	};
	using HotkeyEntry = std::pair<SDL_Keycode, std::string>;

	void register_localized_names();

	std::map<ScopeAndKey, HotkeyEntry> hotkeys_;

	std::map<SDL_Keycode, std::string> localized_names_;

	SDL_Keycode no_key_;
	std::string no_title_;
};
}

#endif  // end of include guard: WL_UI_BASIC_HOTKEYS_H
