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

#include "ui_basic/hotkeys.h"

#include <cassert>

#include <boost/format.hpp>
#include <SDL_keyboard.h>

#include "base/i18n.h"
#include "base/log.h"  // NOCOM

namespace UI {

Hotkeys::Hotkeys() : no_key_(SDLK_UNKNOWN) {
	// NOCOM move this to a unit test
	/*
	log("\nNOCOM testing hotkeys ***********\n");
	const std::string first_test_scope = "test1";
	const std::string second_test_scope = "test2";
	const std::string first_test_key = "first";
	const std::string second_test_key = "second";
	const SDL_Keycode first_test_code = SDLK_a;
	const SDL_Keycode second_test_code = SDLK_2;

	assert(!has_hotkey(first_test_scope, first_test_key));
	assert(!has_code(first_test_scope, first_test_code));
	assert(get_hotkey_title(first_test_scope, first_test_key).empty());
	add_hotkey(first_test_scope, first_test_key, first_test_code, "test title");
	assert(get_hotkey(first_test_scope, first_test_key) == first_test_code);
	assert(get_hotkey_title(first_test_scope, first_test_key) == "test title");
	log("NOCOM key is: %s\n", SDL_GetKeyName(get_hotkey(first_test_scope, first_test_key)));
	assert(has_hotkey(first_test_scope, first_test_key));
	assert(!has_hotkey(second_test_scope, first_test_key));
	assert(!has_hotkey(first_test_scope, second_test_key));
	assert(has_code(first_test_scope, first_test_code));
	assert(!has_code(first_test_scope, second_test_code));

	log("\nNOCOM adding hotkeys finished ***********\n");

	replace_hotkey(first_test_scope, first_test_key, second_test_code);
	assert(get_hotkey(first_test_scope, first_test_key) == second_test_code);
	log("NOCOM key is: %s\n", SDL_GetKeyName(get_hotkey(first_test_scope, first_test_key)));
	assert(has_hotkey(first_test_scope, first_test_key));
	assert(!has_hotkey(second_test_scope, first_test_key));
	assert(!has_hotkey(first_test_scope, second_test_key));
	assert(has_code(first_test_scope, second_test_code));
	assert(!has_code(first_test_scope, first_test_code));

	log("\nNOCOM replacing hotkeys finished ***********\n");

	hotkeys_.erase(ScopeAndKey(first_test_scope, first_test_key));
	assert(hotkeys_.empty());

	log("\nNOCOM testing hotkeys done ***********\n");
	*/

	// NOCOM Default hotkeys
	// add_hotkey("mainmenu", "tutorial", SDLK_t, "Play Tutorial");
}

bool Hotkeys::has_hotkey(const std::string& scope, const std::string& key) {
	return hotkeys_.count(ScopeAndKey(scope, key)) == 1;
}

bool Hotkeys::has_code(const std::string& scope, const SDL_Keycode& code) {
	for (const std::pair<const UI::Hotkeys::ScopeAndKey, const HotkeyEntry>& hotkey : hotkeys_) {
		if ((hotkey.first.scope_ == scope || hotkey.first.scope_ == "global") &&
		    hotkey.second.first == code) {
			return true;
		}
	}
	return false;
}

bool Hotkeys::add_hotkey(const std::string& scope,
                         const std::string& key,
                         const SDL_Keycode& code,
                         const std::string& title) {
	if (!has_hotkey(scope, key) && !has_code(scope, code)) {
		hotkeys_.emplace(ScopeAndKey(scope, key), HotkeyEntry(code, title));
		return true;
	}
	return false;
}

bool
Hotkeys::replace_hotkey(const std::string& scope, const std::string& key, const SDL_Keycode& code) {
	if (has_hotkey(scope, key) && !has_code(scope, code)) {
		const std::string& title = get_hotkey_title(scope, key);
		hotkeys_.erase(ScopeAndKey(scope, key));
		hotkeys_.emplace(ScopeAndKey(scope, key), HotkeyEntry(code, title));
		return true;
	}
	return false;
}

const SDL_Keycode& Hotkeys::get_hotkey(const std::string& scope, const std::string& key) {
	if (has_hotkey(scope, key)) {
		return hotkeys_.at(ScopeAndKey(scope, key)).first;
	}
	return no_key_;
}

const std::string& Hotkeys::get_hotkey_title(const std::string& scope, const std::string& key) {
	if (has_hotkey(scope, key)) {
		return hotkeys_.at(ScopeAndKey(scope, key)).second;
	}
	return no_title_;
}

}  // namespace UI
