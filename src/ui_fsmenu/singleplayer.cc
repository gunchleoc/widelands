/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "ui_fsmenu/singleplayer.h"

#include "base/i18n.h"
#include "base/log.h" // NOCOM
#include "graphic/graphic.h"
#include "graphic/text_constants.h"

#include "wlapplication.h" // NOCOM

FullscreenMenuSinglePlayer::FullscreenMenuSinglePlayer() :
	FullscreenMenuMainMenu(),

// Title
	title
		(this,
		 get_w() / 2, m_title_y,
		 _("Single Player"), UI::Align_HCenter),

// Buttons
	vbox(this, m_box_x, m_box_y, UI::Box::Vertical,
		  m_butw, get_h() - m_box_y, m_padding),
	new_game
		(&vbox, "new_game", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("New Game"), "", true, false),
	campaign
		(&vbox, "campaigns", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("Campaigns"), "", true, false),
	load_game
		(&vbox, "load_game", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("Load Game"), "", true, false),
	back
		(&vbox, "back", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("Back"), "", true, false)
{
	new_game.sigclicked.connect
		(boost::bind
			(&FullscreenMenuSinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(MenuTarget::kNewGame)));
	campaign.sigclicked.connect
		(boost::bind
			(&FullscreenMenuSinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(MenuTarget::kCampaign)));
	load_game.sigclicked.connect
		(boost::bind
			(&FullscreenMenuSinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(MenuTarget::kLoadGame)));
	back.sigclicked.connect
		(boost::bind
			(&FullscreenMenuSinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(MenuTarget::kBack)));
	log("NOCOM KMOD_LCTRL = %d\n", KMOD_LCTRL);
	new_game.set_hotkey("singleplayer", UI::Hotkeys::HotkeyCode(SDLK_n, KMOD_LCTRL)); // NOCOM
	campaign.set_hotkey("singleplayer", UI::Hotkeys::HotkeyCode(SDLK_c));
	load_game.set_hotkey("singleplayer", UI::Hotkeys::HotkeyCode(SDLK_l));
	back.set_hotkey("singleplayer", UI::Hotkeys::HotkeyCode(SDLK_ESCAPE));

	/*
	NOCOM testing hotkey stuff
	This needs to be fully recursive in the end.
	 */
/*
	UI::Hotkeys* hotkeys = WLApplication::get()->hotkeys();

	UI::Hotkeys::Scope* global = hotkeys ->get_scope("global");

	for (const std::string& child : global->get_children()) {

		UI::Hotkeys::Scope* scope = hotkeys->get_scope(child);
		if (scope != nullptr) {
			log("NOCOM Scope: %s - Parent: %s\n", scope->get_title().c_str(), scope->get_parent().c_str());

			for (std::pair<std::string, UI::Hotkeys::HotkeyEntry> entry : hotkeys->hotkeys(scope->get_name())) {
				log("NOCOM -- %s\n", entry.second.second.c_str());
			}
		} else {
			log("NOCOM unknown scope: %s\n", child.c_str());
		}
	}
	*/

	title.set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);

	vbox.add(&new_game, UI::Box::AlignCenter);
	vbox.add(&campaign, UI::Box::AlignCenter);

	vbox.add_space(m_buth);

	vbox.add(&load_game, UI::Box::AlignCenter);

	vbox.add_space(6 * m_buth);

	vbox.add(&back, UI::Box::AlignCenter);

	vbox.set_size(m_butw, get_h() - vbox.get_y());
}


bool FullscreenMenuSinglePlayer::handle_key(bool down, SDL_Keysym code)
{
	if (!down)
		return false;
	if (WLApplication::get()->hotkeys()->is_hotkey_pressed(new_game.get_hotkey(), code)) {
		play_click();
		end_modal(static_cast<int32_t>(MenuTarget::kNewGame));
	} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(campaign.get_hotkey(), code)) {
		play_click();
		end_modal(static_cast<int32_t>(MenuTarget::kCampaign));
	} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(load_game.get_hotkey(), code)) {
		play_click();
		end_modal(static_cast<int32_t>(MenuTarget::kLoadGame));
	} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(back.get_hotkey(), code)) {
		play_click();
		end_modal(static_cast<int32_t>(MenuTarget::kBack));
	}

	log("NOCOM mod: %d \n", code.mod);

	return FullscreenMenuBase::handle_key(down, code);
}
