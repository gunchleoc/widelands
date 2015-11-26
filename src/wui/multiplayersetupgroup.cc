/*
 * Copyright (C) 2010-2011 by the Widelands Development Team
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

#include "wui/multiplayersetupgroup.h"

#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "logic/constants.h"
#include "logic/game.h"
#include "logic/game_settings.h"
#include "logic/player.h"
#include "logic/tribes/tribe_descr.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/icon.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

struct MultiPlayerClientGroup : public UI::Box {
	MultiPlayerClientGroup
		(UI::Panel            * const parent, uint8_t id,
		 int32_t const /* x */, int32_t const /* y */, int32_t const w, int32_t const h,
		 GameSettingsProvider * const settings,
		 UI::Font * font)
		 :
		 UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
		 type_icon(nullptr),
		 type(nullptr),
		 s(settings),
		 m_id(id),
		 m_save(-2)
	{
		set_size(w, h);
		name = new UI::Textarea
			(this, 0, 0, w - h - UI::Scrollbar::Size * 11 / 5, h);
		name->set_textstyle(UI::TextStyle::makebold(font, UI_FONT_CLR_FG));
		add(name, UI::Box::AlignCenter);
		// Either Button if changeable OR text if not
		if (id == settings->settings().usernum) { // Our Client
			type = new UI::Button
				(this, "client_type",
				 0, 0, h, h,
				 g_gr->images().get("pics/but1.png"),
				 std::string(), std::string(), true, false);
			type->sigclicked.connect
				(boost::bind
					 (&MultiPlayerClientGroup::toggle_type, boost::ref(*this)));
			add(type, UI::Box::AlignCenter);
		} else { // just a shown client
			type_icon = new UI::Icon
				(this, 0, 0, h, h,
				 g_gr->images().get("pics/menu_tab_watch.png"));
			add(type_icon, UI::Box::AlignCenter);
		}

	}

	/// Switch human players and spectator
	void toggle_type() {
		UserSettings us = s->settings().users.at(m_id);
		int16_t p = us.position;
		if (p == UserSettings::none())
			p = -1;

		for (++p; p < static_cast<int16_t>(s->settings().players.size()); ++p) {
			if
				(s->settings().players.at(p).state == PlayerSettings::stateHuman
				 ||
				 s->settings().players.at(p).state == PlayerSettings::stateOpen)
			{
				s->set_player_number(p);
				return;
			}
		}
		s->set_player_number(UserSettings::none());
	}

	/// Care about visibility and current values
	void refresh() {
		UserSettings us = s->settings().users.at(m_id);
		if (us.position == UserSettings::not_connected()) {
			std::string free_i18n = _("free");
			std::string free_text =
				(boost::format("\\<%s\\>") % free_i18n).str();
			name->set_text(free_text);
			if (type)
				type->set_visible(false);
			else
				type_icon->set_visible(false);
		} else {
			name->set_text(us.name);
			if (m_save != us.position) {
				std::string pic;
				std::string temp_tooltip;
				if (us.position < UserSettings::highest_playernum()) {
					pic = (boost::format("pics/genstats_enable_plr_0%u.png")
							  % static_cast<unsigned int>(us.position + 1)).str();
					temp_tooltip = (boost::format(_("Player %u"))
										 % static_cast<unsigned int>(us.position + 1)).str();
				} else {
					pic = "pics/menu_tab_watch.png";
					temp_tooltip = _("Spectator");
				}

				// Either Button if changeable OR text if not
				if (m_id == s->settings().usernum) {
					type->set_pic(g_gr->images().get(pic));
					type->set_tooltip(temp_tooltip);
					type->set_visible(true);
				} else {
					type_icon->set_icon(g_gr->images().get(pic));
					type_icon->set_tooltip(temp_tooltip);
					type_icon->set_visible(true);
				}
				m_save = us.position;
			}
		}
	}

	UI::Textarea           * name;
	UI::Icon               * type_icon;
	UI::Button             * type;
	GameSettingsProvider   * const s;
	uint8_t                  const m_id;
	int16_t                  m_save; // saved position to check rewrite need.
};

struct MultiPlayerPlayerGroup : public UI::Box {
	MultiPlayerPlayerGroup
		(UI::Panel            * const parent, uint8_t id,
		 int32_t const /* x */, int32_t const /* y */, int32_t const w, int32_t const h,
		 GameSettingsProvider * const settings,
		 NetworkPlayerSettingsBackend * const npsb,
		 std::map<std::string, const Image* > & tp,
		 std::map<std::string, std::string> & tn)
		 :
		 UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
		 player(nullptr),
		 type(nullptr),
		 tribe(nullptr),
		 init(nullptr),
		 s(settings),
		 n(npsb),
		 m_id(id),
		 m_tribepics(tp),
		 m_tribenames(tn)
	{
		set_size(w, h);

		const std::string pic = (boost::format("pics/fsel_editor_set_player_0%i_pos.png")
										 % static_cast<unsigned int>(id + 1)).str();
		player =
			new UI::Icon(this, 0, 0, h, h, g_gr->images().get(pic));
		add(player, UI::Box::AlignCenter);
		type = new UI::Button
			(this, "player_type",
			 0, 0, h, h,
			 g_gr->images().get("pics/but1.png"),
			 std::string(), std::string(), true, false);
		type->sigclicked.connect
			(boost::bind
				 (&MultiPlayerPlayerGroup::toggle_type, boost::ref(*this)));
		add(type, UI::Box::AlignCenter);
		tribe = new UI::Button
			(this, "player_tribe",
			 0, 0, h, h,
			 g_gr->images().get("pics/but1.png"),
			 std::string(), std::string(), true, false);
		tribe->sigclicked.connect
			(boost::bind
				 (&MultiPlayerPlayerGroup::toggle_tribe, boost::ref(*this)));
		add(tribe, UI::Box::AlignCenter);
		tribe->set_draw_flat_background(true);
		init = new UI::Button
			(this, "player_init",
			 0, 0, w - 4 * h, h,
			 g_gr->images().get("pics/but1.png"),
			 std::string(), std::string(), true, false);
		init->sigclicked.connect
			(boost::bind
				 (&MultiPlayerPlayerGroup::toggle_init, boost::ref(*this)));
		add(init, UI::Box::AlignCenter);
		team = new UI::Button
			(this, "player_team",
			 0, 0, h, h,
			 g_gr->images().get("pics/but1.png"),
			 std::string(), std::string(), true, false);
		team->sigclicked.connect
			(boost::bind
				 (&MultiPlayerPlayerGroup::toggle_team, boost::ref(*this)));
		add(team, UI::Box::AlignCenter);
	}

	/// Toggle through the types
	void toggle_type() {
		n->toggle_type(m_id);
	}

	/// Toggle through the tribes + handle shared in players
	void toggle_tribe() {
		n->toggle_tribe(m_id);
	}

	/// Toggle through the initializations
	void toggle_init() {
		n->toggle_init(m_id);
	}

	/// Toggle through the teams
	void toggle_team() {
		n->toggle_team(m_id);
	}

	/// Refresh all user interfaces
	void refresh() {
		const GameSettings & settings = s->settings();

		if (m_id >= settings.players.size()) {
			set_visible(false);
			return;
		}

		n->refresh(m_id);

		set_visible(true);

		const PlayerSettings & player_setting = settings.players[m_id];
		bool typeaccess       = s->can_change_player_state(m_id);
		bool tribeaccess      = s->can_change_player_tribe(m_id);
		bool const initaccess = s->can_change_player_init(m_id);
		bool teamaccess       = s->can_change_player_team(m_id);

		type->set_enabled(typeaccess);
		if (player_setting.state == PlayerSettings::stateClosed) {
			type ->set_tooltip(_("Closed"));
			type ->set_pic(g_gr->images().get("pics/stop.png"));
			team ->set_visible(false);
			team ->set_enabled(false);
			tribe->set_visible(false);
			tribe->set_enabled(false);
			tribe->set_flat(false);
			init ->set_visible(false);
			init ->set_enabled(false);
			return;
		} else if (player_setting.state == PlayerSettings::stateOpen) {
			type ->set_tooltip(_("Open"));
			type ->set_pic(g_gr->images().get("pics/continue.png"));
			team ->set_visible(false);
			team ->set_enabled(false);
			tribe->set_visible(false);
			tribe->set_enabled(false);
			tribe->set_flat(false);
			init ->set_visible(false);
			init ->set_enabled(false);
			return;
		} else if (player_setting.state == PlayerSettings::stateShared) {
			type ->set_tooltip(_("Shared in"));
			type ->set_pic(g_gr->images().get("pics/shared_in.png"));

			const std::string pic = (boost::format("pics/fsel_editor_set_player_0%u_pos.png")
											 % static_cast<unsigned int>(player_setting.shared_in)).str();

			tribe->set_pic(g_gr->images().get(pic));
			tribe->set_tooltip((boost::format(_("Player %u"))
									  % static_cast<unsigned int>(player_setting.shared_in)).str());

			team ->set_visible(false);
			team ->set_enabled(false);
			// Flat ~= icon
			tribe->set_flat(!initaccess);
			tribe->set_enabled(true);
		} else {
			std::string title;
			std::string pic = "pics/";
			if (player_setting.state == PlayerSettings::stateComputer) {
				if (player_setting.ai.empty()) {
					title = _("Computer");
					pic += "novalue.png";
				} else {
					if (player_setting.random_ai) {
						title = (boost::format(_("AI: %s")) % pgettext("ai_name", "Random")).str();
						pic += "ai_Random.png";
					} else {
						title = (boost::format(_("AI: %s")) % _(player_setting.ai)).str();
						pic += "ai_" + player_setting.ai + ".png";
					}
				}
			} else { // PlayerSettings::stateHuman
				title = _("Human");
				pic += "genstats_nrworkers.png";
			}
			type->set_tooltip(title.c_str());
			type->set_pic(g_gr->images().get(pic));
			if (player_setting.random_tribe) {
				std::string random = pgettext("tribe", "Random");
				if (!m_tribenames["random"].size())
					m_tribepics[random] = g_gr->images().get("pics/random.png");
				tribe->set_tooltip(random.c_str());
				tribe->set_pic(m_tribepics[random]);
			} else {
				if (!m_tribenames[player_setting.tribe].size()) {
					// get tribes name and picture
					i18n::Textdomain td("tribes");
					for (const TribeBasicInfo& tribeinfo : settings.tribes) {
						m_tribenames[tribeinfo.name] = _(tribeinfo.descname);
						m_tribepics[tribeinfo.name] = g_gr->images().get(tribeinfo.icon);
					}
				}
				tribe->set_tooltip(m_tribenames[player_setting.tribe].c_str());
				tribe->set_pic(m_tribepics[player_setting.tribe]);
			}
			tribe->set_flat(false);

			if (player_setting.team) {
				team->set_title(std::to_string(static_cast<unsigned int>(player_setting.team)));
			} else {
				team->set_title("--");
			}
			team ->set_visible(true);
			team ->set_enabled(teamaccess);
			tribe->set_enabled(tribeaccess);
		}
		init ->set_enabled(initaccess);
		tribe->set_visible(true);
		init ->set_visible(true);

		if (settings.scenario)
			init->set_title(_("Scenario"));
		else if (settings.savegame)
			/** Translators: This is a game type */
			init->set_title(_("Saved Game"));
		else {
			i18n::Textdomain td("tribes"); // for translated initialisation
			for (const TribeBasicInfo& tribeinfo : settings.tribes) {
				if (tribeinfo.name == player_setting.tribe) {
					init->set_title(_(tribeinfo.initializations.at(player_setting.initialization_index).descname));
					break;
				}
			}
		}
	}


	UI::Icon                     *       player;
	UI::Button                   *       type;
	UI::Button                   *       tribe;
	UI::Button                   *       init;
	UI::Button                   *       team;
	GameSettingsProvider         * const s;
	NetworkPlayerSettingsBackend * const n;
	uint8_t                        const m_id;
	std::map<std::string, const Image* >   & m_tribepics;
	std::map<std::string, std::string> & m_tribenames;
};

MultiPlayerSetupGroup::MultiPlayerSetupGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t /* butw */, uint32_t buth,
	 const std::string & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
s(settings),
npsb(new NetworkPlayerSettingsBackend(s)),
clientbox(this, 0, buth, UI::Box::Vertical, w / 3, h - buth),
playerbox(this, w * 6 / 15, buth, UI::Box::Vertical, w * 9 / 15, h - buth),
m_buth(buth),
m_fsize(fsize),
m_fname(fname)
{
	UI::TextStyle tsmaller(UI::TextStyle::makebold(UI::Font::get(fname, fsize * 3 / 4), UI_FONT_CLR_FG));

	// Clientbox and labels
	labels.push_back
		(new UI::Textarea
			(this,
			 UI::Scrollbar::Size * 6 / 5, buth / 3,
			 w / 3 - buth - UI::Scrollbar::Size * 2, buth));
	labels.back()->set_text(_("Client name"));
	labels.back()->set_textstyle(tsmaller);

	labels.push_back
		(new UI::Textarea
			(this,
			 w / 3 - buth - UI::Scrollbar::Size * 6 / 5, buth / 3,
			 buth * 2, buth));
	labels.back()->set_text(_("Role"));
	labels.back()->set_textstyle(tsmaller);

	clientbox.set_size(w / 3, h - buth);
	clientbox.set_scrolling(true);

	// Playerbox and labels
	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15, buth / 3,
			 buth, buth));
	labels.back()->set_text(_("Start"));
	labels.back()->set_textstyle(tsmaller);

	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15 + buth, buth / 3 - 10,
			 buth, buth));
	labels.back()->set_text(_("Type"));
	labels.back()->set_textstyle(tsmaller);

	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15 + buth * 2, buth / 3,
			 buth, buth));
	labels.back()->set_text(_("Tribe"));
	labels.back()->set_textstyle(tsmaller);

	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15 + buth * 3, buth / 3,
			 w * 9 / 15 - 4 * buth, buth, UI::Align_HCenter));
	labels.back()->set_text(_("Initialization"));
	labels.back()->set_textstyle(tsmaller);

	labels.push_back(new UI::Textarea(this, w - buth, buth / 3, buth, buth, UI::Align_Right));
	labels.back()->set_text(_("Team"));
	labels.back()->set_textstyle(tsmaller);

	playerbox.set_size(w * 9 / 15, h - buth);
	multi_player_player_groups.resize(MAX_PLAYERS);
	for (uint8_t i = 0; i < multi_player_player_groups.size(); ++i) {
		multi_player_player_groups.at(i) = new MultiPlayerPlayerGroup
			(&playerbox, i,
			 0, 0, playerbox.get_w(), buth,
			 s, npsb.get(),
			 m_tribepics, m_tribenames);
		playerbox.add(multi_player_player_groups.at(i), 1);
	}
	refresh();
}


MultiPlayerSetupGroup::~MultiPlayerSetupGroup()
{
}


/**
 * Update display and enabled buttons based on current settings.
 */
void MultiPlayerSetupGroup::refresh()
{
	const GameSettings & settings = s->settings();

	// Update / initialize client groups
	if (multi_player_client_groups.size() < settings.users.size()) {
		multi_player_client_groups.resize(settings.users.size());
	}
	for (uint32_t i = 0; i < settings.users.size(); ++i) {
		if (!multi_player_client_groups.at(i)) {
			multi_player_client_groups.at(i) = new MultiPlayerClientGroup(
			   &clientbox, i, 0, 0, clientbox.get_w(), m_buth, s, UI::Font::get(m_fname, m_fsize));
			clientbox.add(&*multi_player_client_groups.at(i), 1);
		}
		multi_player_client_groups.at(i)->refresh();
	}

	// Update player groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i) {
		multi_player_player_groups.at(i)->refresh();
	}
}
