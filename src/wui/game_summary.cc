/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "wui/game_summary.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/time_string.h"
#include "graphic/graphic.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wlapplication.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"

#define PADDING 4

GameSummaryScreen::GameSummaryScreen
	(InteractiveGameBase * parent, UI::UniqueWindow::Registry * r)
: UI::UniqueWindow(parent, "game_summary", r, 550, 260, _("Game over")),
m_game(parent->game())
{
	// Init boxes
	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, PADDING);
	m_title_area = new UI::Textarea(vbox, "", UI::Align_HCenter);
	vbox->add(m_title_area, UI::Box::AlignCenter);
	vbox->add_space(PADDING);

	UI::Box * hbox1 = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	m_players_table = new UI::Table<uintptr_t const>(hbox1, 0, 0, 150 + 80 + 100 + 100, 200);
	hbox1->add_space(PADDING);
	hbox1->add(m_players_table, UI::Box::AlignTop);
	hbox1->add_space(PADDING);

	UI::Box * infoBox = new UI::Box(hbox1, 0, 0, UI::Box::Vertical, 0, 0);
	m_gametime_label = new UI::Textarea(infoBox, _("Elapsed time:"));
	infoBox->add(m_gametime_label, UI::Box::AlignLeft);
	m_gametime_value = new UI::Textarea(infoBox);
	infoBox->add(m_gametime_value, UI::Box::AlignLeft);
	infoBox->add_space(2 * PADDING);
	m_info_area_label = new UI::Textarea(infoBox, _("Player info:"));
	infoBox->add(m_info_area_label, UI::Box::AlignLeft);
	m_info_area = new UI::MultilineTextarea(infoBox, 0, 0, 130, 130, "");
	infoBox->add(m_info_area, UI::Box::AlignLeft, true);
	infoBox->add_space(PADDING);
	hbox1->add(infoBox, UI::Box::AlignTop);
	hbox1->add_space(PADDING);
	vbox->add(hbox1, UI::Box::AlignLeft);

	UI::Box * buttonBox = new UI::Box(this, 0, 0, UI::Box::Horizontal);
	m_continue_button = new UI::Button
		(buttonBox, "continue_button",
		 0, 0, 35, 35,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/continue.png"),
		 _("Continue playing"));
	buttonBox->add(m_continue_button, UI::Box::AlignRight);
	buttonBox->add_space(PADDING);
	m_stop_button = new UI::Button
		(buttonBox, "stop_button",
		 0, 0, 35, 35,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/menu_exit_game.png"),
		_("Exit Game"));
	buttonBox->add(m_stop_button, UI::Box::AlignRight);
	buttonBox->add_space(PADDING);
	vbox->add(buttonBox, UI::Box::AlignBottom);
	vbox->add_space(PADDING);
	set_center_panel(vbox);

	// Prepare table
	m_players_table->add_column(150, _("Player"));
	m_players_table->add_column(80, _("Team"), "", UI::Align_HCenter);
	m_players_table->add_column(100, _("Status"), "", UI::Align_HCenter);
	m_players_table->add_column(100, _("Time"));

	// Prepare Elements
	m_title_area->set_textstyle(UI::TextStyle::ui_big());

	// Connections
	m_continue_button->sigclicked.connect
		(boost::bind(&GameSummaryScreen::continue_clicked, this));
	m_stop_button->sigclicked.connect
		(boost::bind(&GameSummaryScreen::stop_clicked, this));
	m_players_table->selected.connect
		(boost::bind(&GameSummaryScreen::player_selected, this, _1));

	// Window
	center_to_parent();
	set_can_focus(true);
	focus();
	fill_data();

	set_thinks(false);
}

bool GameSummaryScreen::handle_mousepress(uint8_t btn, int32_t mx, int32_t my)
{
	// Prevent closing with right click
	if (btn == SDL_BUTTON_RIGHT)
		return true;

	return UI::Window::handle_mousepress(btn, mx, my);
}

void GameSummaryScreen::fill_data()
{
	std::vector<Widelands::PlayerEndStatus> players_status
		= m_game.player_manager()->get_players_end_status();
	bool local_in_game = false;
	bool local_won = false;
	Widelands::Player* single_won = nullptr;
	uint8_t team_won = 0;
	InteractivePlayer* ipl = m_game.get_ipl();
	//this defines a row to be selected, current player,
	//if not then the first line
	uint32_t current_player_position = 0;

	for (uintptr_t i = 0; i < players_status.size(); i++) {
		Widelands::PlayerEndStatus pes = players_status.at(i);
		if (ipl && pes.player == ipl->player_number()) {
			local_in_game = true;
			local_won = pes.result == Widelands::PlayerEndResult::PLAYER_WON;
			current_player_position = i;
		}
		Widelands::Player* p = m_game.get_player(pes.player);
		UI::Table<uintptr_t const>::EntryRecord & te = m_players_table->add(i);
		// Player name & pic
		std::string pic_path =
			(boost::format("pics/genstats_enable_plr_0%|1$u|.png")
			 % static_cast<unsigned int>(pes.player)).str();
		const Image* pic = g_gr->images().get(pic_path);
		te.set_picture(0, pic, p->get_name());
		// Team
		std::string team_str =
			(boost::format("%|1$u|")
			 % static_cast<unsigned int>(p->team_number())).str();
		te.set_string(1, team_str);
		// Status
		std::string stat_str;
		switch (pes.result) {
			case Widelands::PlayerEndResult::PLAYER_LOST:
				/** TRANSLATORS: This is shown in the game summary for the players who have lost. */
				stat_str = _("Lost");
				break;
			case Widelands::PlayerEndResult::PLAYER_WON:
				/** TRANSLATORS: This is shown in the game summary for the players who have won. */
				stat_str = _("Won");
				if (!single_won) {
					single_won = p;
				} else {
					team_won = p->team_number();
				}
				break;
			case Widelands::PlayerEndResult::PLAYER_RESIGNED:
				/** TRANSLATORS: This is shown in the game summary for the players who have resigned. */
				 stat_str = _("Resigned");
				 break;
			default:
				/** TRANSLATORS: This is shown in the game summary when we don't know */
				/** TRANSLATORS: if the player has lost or won. */
				stat_str = _("Unknown");
		}
		te.set_string(2, stat_str);
		// Time
		te.set_string(3, gametimestring(pes.time));
	}

	if (local_in_game) {
		if (local_won) {
			m_title_area->set_text(_("You won!"));
		} else {
			m_title_area->set_text(_("You lost."));
		}
	} else {
		if (team_won <= 0) {
			assert(single_won);
			m_title_area->set_text
				((boost::format(_("%s won!")) % single_won->get_name()).str());
		} else {
			m_title_area->set_text
				((boost::format(_("Team %|1$u| won!"))
				  % static_cast<unsigned int>(team_won)).str());
		}
	}
	m_players_table->update();
	if (!players_status.empty()) {
		m_players_table->select(current_player_position);
	}
	m_gametime_value->set_text(gametimestring(m_game.get_gametime()));
}

void GameSummaryScreen::continue_clicked()
{
	die();
}

void GameSummaryScreen::stop_clicked()
{
	m_game.get_ibase()->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

void GameSummaryScreen::player_selected(uint32_t entry_index)
{
	const uintptr_t selected_player_index = (*m_players_table)[entry_index];
	const Widelands::PlayerEndStatus& player_status =
	   m_game.player_manager()->get_players_end_status()[selected_player_index];

	std::string info_str = parse_player_info(player_status.info);
	m_info_area->set_text(info_str);
	layout();
}

std::string GameSummaryScreen::parse_player_info(std::string info)
{
	using StringSplitIterator = boost::split_iterator<std::string::iterator>;
	if (info.empty()) {
		return info;
	}
	// Iterate through all key=value pairs
	StringSplitIterator substring_it = boost::make_split_iterator
		(info, boost::first_finder(";", boost::is_equal()));
	std::string info_str;
	while (substring_it != StringSplitIterator()) {
		std::string substring = boost::copy_range<std::string>(*substring_it);
		std::vector<std::string> pair;
		boost::split(pair, substring, boost::is_any_of("="));
		assert(pair.size() == 2);

		std::string key = pair.at(0);
		if (key == "score") {
			info_str +=
				(boost::format("%1% : %2%\n") % _("Score") % pair.at(1)).str();
		} else if (key == "resign_reason") {
			info_str +=
				(boost::format("%1%\n") % pair.at(1)).str();
		}
		++substring_it;
	}
	return info_str;
}
