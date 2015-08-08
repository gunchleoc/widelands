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

namespace UI {

class Hotkeys {
public:
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

	struct ScopeAndKey {
		ScopeAndKey();
		ScopeAndKey(const std::string& init_scope, const std::string& init_key)
		   : scope(init_scope), key(init_key) {
		}

		bool operator<(const ScopeAndKey& other) const {
			if (scope == other.scope) {
				return key < other.key;
			}
			return scope < other.scope;
		}

		bool operator==(const ScopeAndKey& other) const {
			return (scope == other.scope && key == other.key);
		}

		std::string scope;
		std::string key;
	};

	struct HotkeyCode {
		HotkeyCode() : sym(SDLK_UNKNOWN), mod(KMOD_NONE) {
		}
		HotkeyCode(const SDL_Keycode& init_sym) : sym(init_sym), mod(KMOD_NONE) {
		}
		HotkeyCode(const SDL_Keycode& init_sym, const SDL_Keymod& init_mod)
		   : sym(init_sym), mod(init_mod) {
		}
		SDL_Keycode sym;
		SDL_Keymod mod;
	};
	using HotkeyEntry = std::pair<HotkeyCode, std::string>;

	Hotkeys();

	bool has_scope(const std::string& name) const;
	void add_scope(const std::string& name, const std::string& title, const std::string& parent);
	const std::string& get_scope_title(const std::string& name) const;
	Scope* get_scope(const std::string& name);

	bool has_hotkey(const std::string& scope, const std::string& key) const;
	bool has_code(const std::string& scope, const SDL_Keycode& sym, const SDL_Keymod& mod) const;
	const HotkeyCode& add_hotkey(const std::string& scope,
	                             const std::string& key,
	                             const std::string& title,
	                             const SDL_Keycode& sym,
	                             const SDL_Keymod& mod = KMOD_NONE);
	bool replace_hotkey(const std::string& scope,
	                    const std::string& key,
	                    const SDL_Keycode& sym,
	                    const SDL_Keymod& mod);
	const Hotkeys::HotkeyCode& get_hotkey(const std::string& scope, const std::string& key) const;
	const std::string& get_hotkey_title(const std::string& scope, const std::string& key) const;
	const std::string get_displayname(const HotkeyCode& code) const;

	bool is_hotkey_pressed(const UI::Hotkeys::HotkeyCode& hotkey, const SDL_Keysym& code) const;

	std::map<std::string, HotkeyEntry> hotkeys(const std::string& scope) const;

	// NOCOM can this go?
	const std::map<ScopeAndKey, HotkeyEntry>& all_hotkeys() const {
		return hotkeys_;
	}

	// NOCOM comment stuff
	// key
	// scope
	// SDL_key

private:
	bool scope_has_root_ancestor(const std::string& name) const;
	void register_localized_names();

	std::map<std::string, Scope> scopes_;
	const std::string root_scope_;

	std::map<ScopeAndKey, HotkeyEntry> hotkeys_;

	std::map<SDL_Keycode, std::string> localized_codes_;
	std::map<SDL_Keycode, std::string> localized_mods_;

	HotkeyCode no_key_;
	std::string no_title_;
};
}

#endif  // end of include guard: WL_UI_BASIC_HOTKEYS_H
