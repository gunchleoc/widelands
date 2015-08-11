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

#include "base/i18n.h"
#include "base/log.h"  // NOCOM
#include "base/wexception.h"

// NOCOM Test all modifier keys to see what they are - revisit the enum and the localization.

namespace UI {

Hotkeys::Hotkeys() : root_scope_("global"), use_numlock_(true) {
	register_localized_names();

	add_scope(root_scope_, _("Global"), "");

	add_scope("menus", _("Menus"), root_scope_);
	add_scope("mainmenu", _("Main Menu"), "menus");
	add_scope("game", _("Game"), root_scope_);
	add_scope("game_message_menu", _("Message Menu"), "game");

	// NOCOM copied over from the readme, these still need to be implemented
	add_hotkey("game", "speed_increase", _("Increase Game Speed"), SDLK_PAGEUP);
	add_hotkey("game", "speed_decrease", _("Decrease Game Speed"), SDLK_PAGEDOWN);
	add_hotkey("game", "pause", _("Pause the game"), SDLK_PAUSE);
	add_hotkey("game", "buildhelp", _("Toggle Building Spaces"), SDLK_SPACE);
	add_hotkey("game", "minimap", _("Toggle Minimap"), SDLK_m);
	add_hotkey("game", "messages", _("Toggle Messages (‘News’)"), SDLK_n);
	add_hotkey("game", "census", _("Toggle Census"), SDLK_c);
	add_hotkey("game", "statistics", _("Toggle Statistics"), SDLK_s);
	add_hotkey("game", "stock", _("Toggle Stock Inventory"), SDLK_i);
	add_hotkey("game", "objectives", _("Toggle Objectives"), SDLK_o);
	add_hotkey("game", "buildings", _("Toggle Building Statictics"), SDLK_b);
	add_hotkey("game", "fullscreen", _("Toggle Fullscreen"), SDLK_PAGEDOWN);
	add_hotkey("game", "location_home", _("Go to Starting Location"), SDLK_HOME);
	add_hotkey("game", "location_previous", _("Go to Previous Location"), SDLK_COMMA);
	add_hotkey("game", "location_next", _("Go to Next Location"), SDLK_PERIOD);
#ifndef NDEBUG
	add_hotkey("game", "debug", _("Debug Console"), SDLK_F6);
#endif
	add_hotkey(
	   "global", "screenshot", _("Screenshot"), SDLK_F11, std::set<Modifier>({Modifier::kCtrl}));

	/* NOCOM
.. _"(CTRL+) 0-9: Remember and go to previously remembered locations" .. "<br>"
.. _"CTRL+F10: quits the game immediately" .. "<br>"
.. _"CTRL+Leftclick: skips confirmation dialogs" .. "<br>"
.. _"CTRL+F11: takes a screenshot"
	 * */

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

bool Hotkeys::use_numlock() const {
	return use_numlock_;
}
void Hotkeys::set_numlock(bool yes_or_no) {
	use_numlock_ = yes_or_no;
}

bool Hotkeys::has_scope(const std::string& name) const {
	return scopes_.count(name) == 1;
}

void
Hotkeys::add_scope(const std::string& name, const std::string& title, const std::string& parent) {
	if (!has_scope(name)) {
		scopes_.emplace(name, Scope(name, title, parent));
	}

	if (has_scope(parent)) {
		Hotkeys::Scope* parent_scope = get_scope(parent);
		parent_scope->add_child(name);
	} else if (name != root_scope_) {
		Hotkeys::Scope* parent_scope = get_scope(root_scope_);
		parent_scope->add_child(name);
	}
	assert(scope_has_root_ancestor(name));
}

const std::string& Hotkeys::get_scope_title(const std::string& name) const {
	if (has_scope(name)) {
		return scopes_.at(name).get_title();
	}
	return no_title_;
}

Hotkeys::Scope* Hotkeys::get_scope(const std::string& name) {
	if (has_scope(name)) {
		return &scopes_.at(name);
	}
	return nullptr;
}

bool Hotkeys::scope_has_root_ancestor(const std::string& name) const {
	if (!has_scope(name)) {
		return false;
	}
	std::string temp_name = name;
	// Make sure that this terminates
	for (int i = 0; i < 100; ++i) {
		if (temp_name == root_scope_) {
			return true;
		}
		temp_name = scopes_.at(temp_name).get_parent();
		if (!has_scope(temp_name)) {
			return false;
		}
	}
	return false;
}

bool Hotkeys::has_hotkey(const std::string& scope, const std::string& key) const {
	return hotkeys_.count(Id(scope, key)) == 1;
}

bool Hotkeys::has_code(const std::string& scope,
                       const SDL_Keycode& sym,
                       const std::set<Modifier>& mods) const {
	if (!scope_has_root_ancestor(scope)) {
		throw wexception("Hotkey scope '%s'' is not a decendant of root", scope.c_str());
	}

	for (const std::pair<const UI::Hotkeys::Id, const HotkeyEntry>& hotkey : hotkeys_) {
		// Iterate the scopes, so we get no hotkey collisions
		std::string temp_scope = scope;
		// Make sure that this terminates
		for (int i = 0; i < 100; ++i) {
			if (hotkey.first.scope == temp_scope && hotkey.second.code.sym == sym) {
				// Iterate over all mods NOCOM make sure this is correct - have all modifiers been
				// considered? i.e., they are exactly the same and the same number.
				std::set<Modifier> temp_mods = mods;
				for (const Modifier& existing_mod : hotkey.second.code.mods) {
					std::set<Modifier>::iterator iterator = temp_mods.find(existing_mod);
					if (iterator == temp_mods.end()) {
						return false;
					} else {
						temp_mods.erase(iterator);
					}
				}
			}
			if (temp_scope == root_scope_) {
				return false;
			}
			if (!has_scope(temp_scope)) {
				throw wexception("Hotkey scope '%s'' does not exist for hotkey '%s'",
				                 temp_scope.c_str(),
				                 get_displayname(Hotkeys::HotkeyCode(sym, mods)).c_str());
			}
			temp_scope = scopes_.at(temp_scope).get_parent();
		}
	}
	return false;
}

const Hotkeys::HotkeyCode& Hotkeys::add_hotkey(const std::string& scope,
                                               const std::string& key,
                                               const std::string& title,
                                               const SDL_Keycode& sym,
                                               const std::set<Modifier>& mods) {
	if (!has_scope(scope)) {
		add_scope(scope, scope, root_scope_);
	}

	if (!has_hotkey(scope, key)) {
		if (has_code(scope, sym, mods)) {
			// NOCOM hotkey collisions need to be handled - we still want to be able to add a hotkey
			// here.
			// Popup messagebox?
			log("NOCOM Hotkey collision - need error handling!\n");
		}
		hotkeys_.emplace(Id(scope, key), HotkeyEntry(HotkeyCode(sym, mods), title));
	}
	assert(scope_has_root_ancestor(scope));
	return get_hotkey(scope, key);
}

bool Hotkeys::replace_hotkey(const std::string& scope,
                             const std::string& key,
                             const SDL_Keycode& sym,
                             const std::set<SDL_Keymod>& mods) {
	if (!scope_has_root_ancestor(scope)) {
		throw wexception("Hotkey scope '%s'' is not a decendant of root", scope.c_str());
	}
	std::set<Modifier> modifiers;
	for (const SDL_Keymod& mod : mods) {
		modifiers.emplace(get_modifier(mod));
	}
	if (has_hotkey(scope, key) && !has_code(scope, sym, modifiers)) {
		const std::string& title = get_hotkey_title(scope, key);
		hotkeys_.erase(Id(scope, key));
		hotkeys_.emplace(Id(scope, key), HotkeyEntry(HotkeyCode(sym, modifiers), title));
		return true;
	}
	return false;
}

const Hotkeys::HotkeyCode& Hotkeys::get_hotkey(const std::string& scope,
                                               const std::string& key) const {
	if (has_hotkey(scope, key)) {
		return hotkeys_.at(Id(scope, key)).code;
	}
	log("Unknown hotkey '%s' in scope '%s'\n", key.c_str(), scope.c_str());
	return no_key_;
}

const std::string& Hotkeys::get_hotkey_title(const std::string& scope,
                                             const std::string& key) const {
	if (has_hotkey(scope, key)) {
		return hotkeys_.at(Id(scope, key)).title;
	}
	return no_title_;
}

bool Hotkeys::is_hotkey_pressed(const UI::Hotkeys::HotkeyCode& hotkey,
                                const SDL_Keysym& code) const {
	if (is_symbol_pressed(hotkey.sym, code)) {
		if (!hotkey.mods.empty()) {
			return are_modifiers_pressed(hotkey.mods, code.mod);
		}
		return true;
	}
	return false;
}

bool Hotkeys::is_symbol_pressed(const SDL_Keycode& hotkey_sym, const SDL_Keysym& code) const {

	switch (hotkey_sym) {
	case SDLK_KP_DIVIDE:
	case SDLK_SLASH:
		return (code.sym == SDLK_KP_DIVIDE) || (code.sym == SDLK_SLASH);
	case SDLK_KP_MULTIPLY:
	case SDLK_ASTERISK:
		return (code.sym == SDLK_KP_MULTIPLY) || (code.sym == SDLK_ASTERISK);
	case SDLK_KP_MINUS:
	case SDLK_MINUS:
		return (code.sym == SDLK_KP_MINUS) || (code.sym == SDLK_MINUS);
	case SDL_SCANCODE_KP_PLUS:
	case SDLK_PLUS:
		return (code.sym == SDL_SCANCODE_KP_PLUS) || (code.sym == SDLK_PLUS);
	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		return (code.sym == SDLK_KP_ENTER) || (code.sym == SDLK_RETURN);
	case SDLK_KP_0:
	case SDLK_0:
		return (use_numlock() && code.sym == SDLK_KP_0) || (code.sym == SDLK_0);
	case SDLK_KP_1:
	case SDLK_1:
		return (use_numlock() && code.sym == SDLK_KP_1) || (code.sym == SDLK_1);
	case SDLK_KP_2:
	case SDLK_2:
		return (use_numlock() && code.sym == SDLK_KP_2) || (code.sym == SDLK_2);
	case SDLK_KP_3:
	case SDLK_3:
		return (use_numlock() && code.sym == SDLK_KP_3) || (code.sym == SDLK_3);
	case SDLK_KP_4:
	case SDLK_4:
		return (use_numlock() && code.sym == SDLK_KP_4) || (code.sym == SDLK_4);
	case SDLK_KP_5:
	case SDLK_5:
		return (use_numlock() && code.sym == SDLK_KP_5) || (code.sym == SDLK_5);
	case SDLK_KP_6:
	case SDLK_6:
		return (use_numlock() && code.sym == SDLK_KP_6) || (code.sym == SDLK_6);
	case SDLK_KP_7:
	case SDLK_7:
		return (use_numlock() && code.sym == SDLK_KP_7) || (code.sym == SDLK_7);
	case SDLK_KP_8:
	case SDLK_8:
		return (use_numlock() && code.sym == SDLK_KP_8) || (code.sym == SDLK_8);
	case SDLK_KP_9:
	case SDLK_9:
		return (use_numlock() && code.sym == SDLK_KP_9) || (code.sym == SDLK_9);
	default:
		break;
	}
	return hotkey_sym == code.sym;
}

Hotkeys::Modifier Hotkeys::get_modifier(const SDL_Keymod& mod) const {
	switch (mod) {
	case KMOD_LSHIFT:
	case KMOD_RSHIFT:
		return Hotkeys::Modifier::kShift;
	case KMOD_LCTRL:
	case KMOD_RCTRL:
		return Hotkeys::Modifier::kCtrl;
	case KMOD_LALT:
	case KMOD_RALT:
		return Hotkeys::Modifier::kAlt;
	case KMOD_LGUI:
	case KMOD_RGUI:
		return Hotkeys::Modifier::kGui;
	case KMOD_CAPS:
		return Hotkeys::Modifier::kCaps;
	case KMOD_MODE:
		return Hotkeys::Modifier::kMode;
	default:
		return Hotkeys::Modifier::kNone;
	}
}

bool Hotkeys::are_modifiers_pressed(const std::set<Modifier>& hotkey_mods,
                                    const Uint16 pressed_mod) const {
	bool result = true;
	// Make sure that all the modifiers that we want are pressed, and only them.
	for (int i = static_cast<int>(Modifier::kNone); i < static_cast<int>(Modifier::kMode) && result;
	     ++i) {
		bool is_wanted = hotkey_mods.count(static_cast<Modifier>(i)) == 1;
		bool is_pressed = is_modifier_pressed(static_cast<Modifier>(i), pressed_mod);
		result = result && (is_wanted == is_pressed);
	}
	return result;
}

bool Hotkeys::is_modifier_pressed(const Hotkeys::Modifier& modifier,
                                  const Uint16 pressed_mod) const {
	switch (modifier) {
	case Hotkeys::Modifier::kShift:
		return (pressed_mod & (KMOD_LSHIFT | KMOD_RSHIFT));
	case Hotkeys::Modifier::kCtrl:
		return (pressed_mod & (KMOD_LCTRL | KMOD_RCTRL));
	case Hotkeys::Modifier::kAlt:
		return (pressed_mod & (KMOD_LALT | KMOD_RALT));
	case Hotkeys::Modifier::kGui:
		return (pressed_mod & (KMOD_LGUI | KMOD_RGUI));
	case Hotkeys::Modifier::kCaps:
		return (pressed_mod & KMOD_CAPS);
	case Hotkeys::Modifier::kMode:
		return (pressed_mod & KMOD_MODE);
	default:
		return false;
	}
}

std::map<std::string, Hotkeys::HotkeyEntry> Hotkeys::hotkeys(const std::string& scope) const {
	std::map<std::string, Hotkeys::HotkeyEntry> result;
	for (const std::pair<const UI::Hotkeys::Id, HotkeyEntry>& hotkey : hotkeys_) {
		if (hotkey.first.scope == scope) {
			result.emplace(hotkey.first.key, hotkey.second);
		}
	}
	return result;
}

const std::string Hotkeys::get_displayname(const HotkeyCode& code) const {
	std::string result;
	if (localized_codes_.count(code.sym) == 1) {
		result = localized_codes_.at(code.sym);
	} else {
		result = SDL_GetKeyName(code.sym);
	}
	for (const Modifier& mod : code.mods) {
		if (mod != Modifier::kNone && localized_modifiers_.count(mod) == 1) {
			/** TRANSLATORS: A key combination on the keyboard, e.g. 'Ctrl + A' */
			result = (boost::format(_("%1% + %2%")) % localized_modifiers_.at(mod) % result).str();
		}
	}
	return result;
}

void Hotkeys::register_localized_names() {
	/** TRANSLATORS: A key on the keyboard. Unknown hotkey */
	localized_codes_.emplace(SDLK_UNKNOWN, _("Unknown"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_RETURN, _("Return"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_ESCAPE, _("Escape"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_BACKSPACE, _("Backspace"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F1, _("F1"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F2, _("F2"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F3, _("F3"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F4, _("F4"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F5, _("F5"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F6, _("F6"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F7, _("F7"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F8, _("F8"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F9, _("F9"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F10, _("F10"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F11, _("F11"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_F12, _("F12"));
	// localized_names_.emplace(SDLK_PRINTSCREEN, _("Print"));
	// localized_names_.emplace(SDLK_PAUSE, _("Pause"));
	// localized_names_.emplace(SDLK_INSERT, _("Ins"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_HOME, _("Home"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_PAGEUP, _("Page Up"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_DELETE, _("Del"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_END, _("End"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_PAGEDOWN, _("Page Down"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_RIGHT, _("Right Arrow"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_LEFT, _("Left Arrow"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_DOWN, _("Down Arrow"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_UP, _("Up Arrow"));
	/** TRANSLATORS: A key on the keyboard. Hotkey name */
	localized_codes_.emplace(SDLK_KP_ENTER, _("Enter"));

	/** TRANSLATORS: A key on the keyboard. Hotkey modifier */
	localized_modifiers_.emplace(Hotkeys::Modifier::kShift, _("Shift"));
	/** TRANSLATORS: A key on the keyboard. Hotkey modifier */
	localized_modifiers_.emplace(Hotkeys::Modifier::kCtrl, _("Ctrl"));
	/** TRANSLATORS: A key on the keyboard. Hotkey modifier */
	localized_modifiers_.emplace(Hotkeys::Modifier::kAlt, _("Alt"));
	/** TRANSLATORS: A key on the keyboard. Hotkey modifier */
	localized_modifiers_.emplace(Hotkeys::Modifier::kGui, _("Gui"));
	/** TRANSLATORS: A key on the keyboard. Hotkey modifier */
	localized_modifiers_.emplace(Hotkeys::Modifier::kCaps, _("Caps Lock"));
	/** TRANSLATORS: A key on the keyboard. Hotkey modifier */
	localized_modifiers_.emplace(Hotkeys::Modifier::kMode, _("Mode"));
}

}  // namespace UI
