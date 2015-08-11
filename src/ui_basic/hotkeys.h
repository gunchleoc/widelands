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
#include <set>

#include <SDL_keycode.h>
#include <SDL_keyboard.h>

// NOCOM comment stuff
// NOCOM go through the structs to see what should be private.

namespace UI {

class Hotkeys {
public:
	// Hotkey scopes are a tree - the same hotkey code/sym combination is not allowed in the same
	// scope, including its ancestors.
	struct Scope {
		Scope(const std::string& init_name,
		      const std::string& init_title,
		      const std::string& init_parent)
		   : name_(init_name), title_(init_title), parent_(init_parent) {
		}

		void add_child(const std::string& name) {
			if (children_.count(name) == 0) {
				children_.emplace(name);
			}
		}

		const std::string& get_name() const {
			return name_;
		}

		const std::string& get_title() const {
			return title_;
		}

		const std::string& get_parent() const {
			return parent_;
		}

		const std::set<std::string>& get_children() const {
			return children_;
		}

	private:
		const std::string name_;
		const std::string title_;
		const std::string parent_;
		std::set<std::string> children_;
	};

	// Identifier for a hotkey
	struct Id {
		Id();
		Id(const std::string& init_scope, const std::string& init_key)
		   : scope(init_scope), key(init_key) {
		}

		bool operator<(const Id& other) const {
			if (scope == other.scope) {
				return key < other.key;
			}
			return scope < other.scope;
		}

		bool operator==(const Id& other) const {
			return (scope == other.scope && key == other.key);
		}

		std::string scope;
		std::string key;
	};

	// Modifiers combine SDL_Keymod types (e.g. kCtrl represents KMOD_LCTRL and/or KMOD_RCTRL)
	enum class Modifier { kNone = 0, kShift, kCtrl, kAlt, kGui, kCaps, kMode };

	// Wrapper class for keys that can be pressed together
	struct HotkeyCode {
		HotkeyCode() : sym(SDLK_UNKNOWN) {
		}
		HotkeyCode(const SDL_Keycode& init_sym) : sym(init_sym) {
		}
		HotkeyCode(const SDL_Keycode& init_sym, const std::set<Modifier>& init_mods)
		   : sym(init_sym), mods(init_mods) {
		}
		SDL_Keycode sym;
		std::set<Modifier> mods;
	};

	// Keys pressed, title.
	struct HotkeyEntry {
		HotkeyEntry(const HotkeyCode& init_code, const std::string& init_title)
		   : code(init_code), title(init_title) {
		}
		HotkeyCode code;
		std::string title;
	};

	Hotkeys();

	// Workaround for SDL bug where initial Num Lock state isn't read from the operating system.
	bool use_numlock() const;
	void set_numlock(bool yes_or_no);

	bool has_scope(const std::string& name) const;
	void add_scope(const std::string& name, const std::string& title, const std::string& parent);
	const std::string& get_scope_title(const std::string& name) const;
	Scope* get_scope(const std::string& name);

	Modifier get_modifier(const SDL_Keymod& mod) const;

	bool has_hotkey(const std::string& scope, const std::string& key) const;
	bool
	has_code(const std::string& scope, const SDL_Keycode& sym, const std::set<Modifier>& mods) const;
	const HotkeyCode& add_hotkey(const std::string& scope,
	                             const std::string& key,
	                             const std::string& title,
	                             const SDL_Keycode& sym,
	                             const std::set<Modifier>& mods = std::set<Modifier>());
	bool replace_hotkey(const std::string& scope,
	                    const std::string& key,
	                    const SDL_Keycode& sym,
	                    const std::set<SDL_Keymod>& mods);
	const Hotkeys::HotkeyCode& get_hotkey(const std::string& scope, const std::string& key) const;
	const std::string& get_hotkey_title(const std::string& scope, const std::string& key) const;
	const std::string get_displayname(const HotkeyCode& code) const;

	bool is_hotkey_pressed(const UI::Hotkeys::HotkeyCode& hotkey, const SDL_Keysym& code) const;

	std::map<std::string, HotkeyEntry> hotkeys(const std::string& scope) const;

	// NOCOM can this go?
	const std::map<Id, HotkeyEntry>& all_hotkeys() const {
		return hotkeys_;
	}

private:
	bool is_symbol_pressed(const SDL_Keycode& hotkey_sym, const SDL_Keysym& code) const;
	bool is_modifier_pressed(const Hotkeys::Modifier& modifier, const Uint16 pressed_mod) const;
	bool are_modifiers_pressed(const std::set<Modifier>& hotkey_mods,
	                           const Uint16 pressed_mod) const;

	bool scope_has_root_ancestor(const std::string& name) const;
	void register_localized_names();

	std::map<std::string, Scope> scopes_;
	const std::string root_scope_;

	std::map<Id, HotkeyEntry> hotkeys_;

	std::map<SDL_Keycode, std::string> localized_codes_;
	std::map<Modifier, std::string> localized_modifiers_;

	HotkeyCode no_key_;
	std::string no_title_;

	// KMOD_NUM does not initialize the Num Lock state from the OS
	// (Bug in SDL2)
	// https://bugs.launchpad.net/widelands/+bug/1177064
	bool use_numlock_;
};
}

#endif  // end of include guard: WL_UI_BASIC_HOTKEYS_H
