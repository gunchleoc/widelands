/*
 * Copyright (C) 2002, 2006-2012 by the Widelands Development Team
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

#include "ui_fsmenu/launch_mpg.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/warning.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/instances.h"
#include "logic/map.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"
#include "wui/gamechatpanel.h"
#include "wui/multiplayersetupgroup.h"

/// Simple user interaction window for selecting either map, save or cancel
struct MapOrSaveSelectionWindow : public UI::Window {
	MapOrSaveSelectionWindow
		(UI::Panel * parent, GameController * gc, uint32_t w, uint32_t h)
	:
	/** TRANSLATORS: Dialog box title for selecting between map or saved game for new multiplayer game */
	Window(parent, "selection_window", 0, 0, w, h, _("Please select")),
	m_ctrl(gc)
	{
		center_to_parent();

		uint32_t y     = get_inner_h() / 10;
		uint32_t space = y;
		uint32_t butw  = get_inner_w() - 2 * space;
		uint32_t buth  = (get_inner_h() - 2 * space) / 5;
		UI::Button * btn = new UI::Button
			(this, "map",
			 space, y, butw, buth,
			 g_gr->images().get("pics/but0.png"),
			 _("Map"), _("Select a map"), true, false);
		btn->sigclicked.connect
			(boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this),
				  FullscreenMenuBase::MenuTarget::kNormalGame));

		btn = new UI::Button
			(this, "saved_game",
			 space, y + buth + space, butw, buth,
			 g_gr->images().get("pics/but0.png"),
			 /** Translators: This is a button to select a savegame */
			 _("Saved Game"), _("Select a saved game"), true, false);
		btn->sigclicked.connect
			(boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this),
				  FullscreenMenuBase::MenuTarget::kScenarioGame));

		btn = new UI::Button
			(this, "cancel",
			 space + butw / 4, y + 3 * buth + 2 * space, butw / 2, buth,
			 g_gr->images().get("pics/but1.png"),
			 _("Cancel"), _("Cancel selection"), true, false);
		btn->sigclicked.connect
			(boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this),
				  FullscreenMenuBase::MenuTarget::kBack));
	}


	void think() override {
		if (m_ctrl)
			m_ctrl->think();
	}

	void pressedButton(FullscreenMenuBase::MenuTarget i) {
		end_modal<FullscreenMenuBase::MenuTarget>(i);
	}
	private:
		GameController * m_ctrl;
};

FullscreenMenuLaunchMPG::FullscreenMenuLaunchMPG
	(GameSettingsProvider * const settings, GameController * const ctrl)
	:
	FullscreenMenuBase("launchMPGmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),
	// TODO(GunChleoc): We still need to use these consistently. Just getting them in for now
	// so we can have the SuggestedTeamsBox
	m_padding(4),
	m_indent(10),
	m_label_height(20),
	m_right_column_x(get_w() * 37 / 50),

// Buttons
	m_change_map_or_save
		(this, "change_map_or_save",
		 m_right_column_x + m_butw - m_buth, get_h() * 3 / 20, m_buth, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/menu_toggle_minimap.png"),
		 _("Change map or saved game"), false, false),
	m_ok
		(this, "ok",
		 m_right_column_x, get_h() * 12 / 20 - 2 * m_label_height, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("Start game"), std::string(), false, false),
	m_back
		(this, "back",
		 m_right_column_x, get_h() * 218 / 240, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	m_wincondition
		(this, "win_condition",
		 m_right_column_x, get_h() * 11 / 20 - 2 * m_label_height, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 "", std::string(), false, false),
	m_help_button
		(this, "help",
		 m_right_column_x + m_butw - m_buth, get_h() / 100, m_buth, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/menu_help.png"),
		 _("Show the help window"), true, false),

// Text labels
	m_title
		(this,
		 get_w() / 2, get_h() / 25,
		 _("Multiplayer Game Setup"), UI::Align_HCenter),
	m_mapname
		(this,
		 m_right_column_x, get_h() * 3 / 20,
		 std::string()),
	m_clients
		(this,
		 // (get_w() * 57 / 80) is the width of the MultiPlayerSetupGroup
		 get_w() / 50, get_h() / 10, (get_w() * 57 / 80) / 3, get_h() / 10,
		 _("Clients"), UI::Align_HCenter),
	m_players
		(this,
		 get_w() / 50 + (get_w() * 57 / 80) * 6 / 15, get_h() / 10, (get_w() * 57 / 80) * 9 / 15, get_h() / 10,
		 _("Players"), UI::Align_HCenter),
	m_map
		(this,
		 m_right_column_x, get_h() / 10, m_butw, get_h() / 10,
		 _("Map"), UI::Align_HCenter),
	m_wincondition_type
		(this,
		 m_right_column_x + (m_butw / 2), get_h() * 10 / 20 - 1.5 * m_label_height,
		 _("Type of game"), UI::Align_HCenter),

	m_map_info(this, m_right_column_x, get_h() * 2 / 10, m_butw, get_h() * 23 / 80 - 2 * m_label_height),
	m_client_info(this, m_right_column_x, get_h() * 13 / 20 - 2 * m_label_height, m_butw, 2 * m_label_height),
	m_help(nullptr),

// Variables and objects used in the menu
	m_settings     (settings),
	m_ctrl         (ctrl),
	m_chat         (nullptr)
{
	m_change_map_or_save.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLaunchMPG::change_map_or_save, boost::ref(*this)));
	m_ok.sigclicked.connect(boost::bind(&FullscreenMenuLaunchMPG::clicked_ok, boost::ref(*this)));
	m_back.sigclicked.connect(boost::bind(&FullscreenMenuLaunchMPG::clicked_back, boost::ref(*this)));
	m_wincondition.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLaunchMPG::win_condition_clicked,
			  boost::ref(*this)));
	m_help_button.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLaunchMPG::help_clicked,
			  boost::ref(*this)));

	m_wincondition_type.set_textstyle(UI::TextStyle::ui_small());

	m_lua = new LuaInterface();
	win_condition_clicked();

	m_title      .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_mapname    .set_font(m_fn, m_fs, RGBColor(255, 255, 127));
	m_clients    .set_font(m_fn, m_fs, RGBColor(0, 255, 0));
	m_players    .set_font(m_fn, m_fs, RGBColor(0, 255, 0));
	m_map        .set_font(m_fn, m_fs, RGBColor(0, 255, 0));
	m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_map_info   .set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	m_mapname .set_text(_("(no map)"));
	m_map_info.set_text(_("The host has not yet selected a map or saved game."));

	m_mpsg =
		new MultiPlayerSetupGroup
			(this,
			 get_w() / 50, get_h() / 8, get_w() * 57 / 80, get_h() / 2,
			 settings, m_butw, m_buth, m_fn, m_fs);

	// If we are the host, open the map or save selection menu at startup
	if (m_settings->settings().usernum == 0 && m_settings->settings().mapname.empty()) {
		change_map_or_save();
		// Try to associate the host with the first player
		if (!m_settings->settings().players.empty()) {
			m_settings->set_player_number(0);
		}
	}

	// Y coordinate will be set later, when we know how high this box will get.
	m_suggested_teams_box = new UI::SuggestedTeamsBox
									(this, m_right_column_x, 0, UI::Box::Vertical,
									 m_padding, m_indent, m_label_height,
									 get_w() - m_right_column_x, 4 * m_label_height);
}

FullscreenMenuLaunchMPG::~FullscreenMenuLaunchMPG() {
	delete m_lua;
	delete m_mpsg;
	if (m_help)
		delete m_help;
	delete m_chat;
}


void FullscreenMenuLaunchMPG::think()
{
	if (m_ctrl)
		m_ctrl->think();

	refresh();
}


/**
 * Set a new chat provider.
 *
 * This automatically creates and displays a chat panel when appropriate.
 */
void FullscreenMenuLaunchMPG::set_chat_provider(ChatProvider & chat)
{
	delete m_chat;
	m_chat = new GameChatPanel
		(this, get_w() / 50, get_h() * 13 / 20, get_w() * 57 / 80, get_h() * 3 / 10, chat);
}


/**
 * back-button has been pressed
 */
void FullscreenMenuLaunchMPG::clicked_back()
{
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
}

/**
 * WinCondition button has been pressed
 */
void FullscreenMenuLaunchMPG::win_condition_clicked()
{
	m_settings->next_win_condition();
	win_condition_update();
}

/**
 * update win conditions information
 */
void FullscreenMenuLaunchMPG::win_condition_update() {
	if (m_settings->settings().scenario) {
		m_wincondition.set_title(_("Scenario"));
		m_wincondition.set_tooltip
			(_("Win condition is set through the scenario"));
	} else if (m_settings->settings().savegame) {
		/** Translators: This is a game type */
		m_wincondition.set_title(_("Saved Game"));
		m_wincondition.set_tooltip
			(_("The game is a saved game – the win condition was set before."));
	} else {
		std::unique_ptr<LuaTable> t = m_lua->run_script(m_settings->get_win_condition_script());
		t->do_not_warn_about_unaccessed_keys();

		try {
			std::string name = t->get_string("name");
			std::string descr = t->get_string("description");

			{
				i18n::Textdomain td("win_conditions");
				m_wincondition.set_title(_(name));
			}
			m_wincondition.set_tooltip(descr.c_str());
		} catch (LuaTableKeyError &) {
			// might be that this is not a win condition after all.
			win_condition_clicked();
		}
	}
}

/// Opens a popup window to select a map or saved game
void FullscreenMenuLaunchMPG::change_map_or_save() {
	MapOrSaveSelectionWindow selection_window
		(this, m_ctrl, get_w() / 3, get_h() / 4);
	switch (selection_window.run<FullscreenMenuBase::MenuTarget>()) {
		case FullscreenMenuBase::MenuTarget::kNormalGame:
			select_map();
			break;
		case FullscreenMenuBase::MenuTarget::kScenarioGame:
			select_saved_game();
			break;
		default:
			return;
	}
}

/**
 * Select a map and send all information to the user interface.
 */
void FullscreenMenuLaunchMPG::select_map() {
	if (!m_settings->can_change_map())
		return;

	FullscreenMenuMapSelect msm(m_settings, m_ctrl);
	FullscreenMenuBase::MenuTarget code = msm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		m_settings->set_scenario(false);
		return;
	}

	m_settings->set_scenario(code == FullscreenMenuBase::MenuTarget::kScenarioGame);

	const MapData & mapdata = *msm.get_map();
	m_nr_players = mapdata.nrplayers;

	// If the same map was selected again, maybe the state of the "scenario" check box was changed
	// So we should recheck all map predefined values,
	// which is done in refresh(), if m_filename_proof is different to settings.mapfilename -> dummy rename
	if (mapdata.filename == m_filename_proof)
		m_filename_proof = m_filename_proof + "new";

	m_settings->set_map(mapdata.name, mapdata.filename, m_nr_players);
}

/**
 * Select a multi player saved game and send all information to the user
 * interface.
 */
void FullscreenMenuLaunchMPG::select_saved_game() {
	if (!m_settings->can_change_map())
		return;

	Widelands::Game game; // The place all data is saved to.
	FullscreenMenuLoadGame lsgm(game, m_settings, m_ctrl);
	FullscreenMenuBase::MenuTarget code = lsgm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		return; // back was pressed
	}

	// Saved game was selected - therefore not a scenario
	m_settings->set_scenario(false);

	std::string filename = lsgm.filename();

	if (g_fs->file_exists(filename.c_str())) {
		// Read the needed data from file "elemental" of the used map.
		std::unique_ptr<FileSystem> l_fs(g_fs->make_sub_file_system(filename.c_str()));
		Profile prof;
		prof.read("map/elemental", nullptr, *l_fs);
		Section & s = prof.get_safe_section("global");

		std::string mapname = s.get_safe_string("name");
		m_nr_players = s.get_safe_int("nr_players");

		m_settings->set_map(mapname, filename, m_nr_players, true);

		// Check for sendability
		if (g_fs->is_directory(filename)) {
			// Send a warning
			UI::WLMessageBox warning
				(this, _("Saved game is directory"),
				_
				("WARNING:\n"
					"The saved game you selected is a directory."
					" This happens if you set the option ‘nozip’ to "
					"true or manually unzipped the saved game.\n"
					"Widelands is not able to transfer directory structures to the clients,"
					" please select another saved game or zip the directories’ content."),
				UI::WLMessageBox::MBoxType::kOk);
			warning.run<UI::Panel::Returncodes>();
		}
	} else {
		if (!m_settings || m_settings->settings().saved_games.empty())
			throw wexception("A file was selected, that is not available to the client");
		// this file is obviously a file from the dedicated server's saved games pool not available locally.
		for (uint32_t i = 0; i < m_settings->settings().saved_games.size(); ++i)
			if (m_settings->settings().saved_games.at(i).path == filename) {
				m_settings->set_map(filename, filename, m_settings->settings().saved_games.at(i).players, true);
				return;
			}
		throw wexception("The selected file could not be found in the pool of dedicated saved games.");
	}
}

/**
 * start-button has been pressed
 */
void FullscreenMenuLaunchMPG::clicked_ok()
{
	if (!g_fs->file_exists(m_settings->settings().mapfilename))
		throw WLWarning
			(_("File not found"),
			 _
			 ("Widelands tried to start a game with a file that could not be "
			  "found at the given path.\n"
			  "The file was: %s\n"
			  "If this happens in a network game, the host might have selected "
			  "a file that you do not own. Normally, such a file should be sent "
			  "from the host to you, but perhaps the transfer was not yet "
			  "finished!?!"),
			 m_settings->settings().mapfilename.c_str());
	if (m_settings->can_launch())
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNormalGame);
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void FullscreenMenuLaunchMPG::refresh()
{
	const GameSettings & settings = m_settings->settings();

	if (settings.mapfilename != m_filename_proof) {
		if (!g_fs->file_exists(settings.mapfilename)) {
			m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_WARNING);
			m_client_info.set_text
				(_("The selected file can not be found. If it is not automatically "
				   "transferred to you, please write to the host about this problem."));
		} else {
			// Reset font color
			m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_FG);

			// Update local nr of players - needed for the client UI
			m_nr_players = settings.players.size();

			// Care about the newly selected file. This has to be done here and not
			// after selection of a new map / saved game, as the clients user
			// interface can only notice the change after the host broadcasted it.
			if (settings.savegame) {
				load_previous_playerdata();
			} else {
				load_map_info();
				if (settings.scenario)
					set_scenario_values();
			}
			//Try to translate the map name.
			//This will work on every official map as expected
			//and 'fail silently' (not find a translation) for already translated campaign map names.
			//It will also translate 'false-positively' on any user-made map which shares a name with
			//the official maps, but this should not be a problem to worry about.
			i18n::Textdomain td("maps");
			m_mapname.set_text(_(settings.mapname));
		}
	} else {
		// Write client infos
		std::string client_info =
			(settings.playernum >= 0) && (settings.playernum < MAX_PLAYERS) ?
					(boost::format(_("You are Player %i.")) % (settings.playernum + 1)).str() :
					_("You are a spectator.");
		m_client_info.set_text(client_info);
	}

	m_ok.set_enabled(m_settings->can_launch());

	m_change_map_or_save.set_enabled(m_settings->can_change_map());
	m_change_map_or_save.set_visible(m_settings->can_change_map());

	m_wincondition.set_enabled
		(m_settings->can_change_map() && !settings.savegame && !settings.scenario);

	win_condition_update();

	// Update the multi player setup group
	m_mpsg->refresh();
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchMPG::set_scenario_values()
{
	const GameSettings & settings = m_settings->settings();
	if (settings.mapfilename.empty())
		throw wexception
			("settings()->scenario was set to true, but no map is available");
	Widelands::Map map; //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(settings.mapfilename));
	map.set_filename(settings.mapfilename);
	ml->preload_map(true);
	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		m_settings->set_player_tribe    (i, map.get_scenario_player_tribe    (i + 1));
		m_settings->set_player_closeable(i, map.get_scenario_player_closeable(i + 1));
		std::string ai(map.get_scenario_player_ai(i + 1));
		if (!ai.empty()) {
			m_settings->set_player_state(i, PlayerSettings::stateComputer);
			m_settings->set_player_ai   (i, ai);
		} else if
			(settings.players.at(i).state != PlayerSettings::stateHuman
			 &&
			 settings.players.at(i).state != PlayerSettings::stateOpen)
		{
			m_settings->set_player_state(i, PlayerSettings::stateOpen);
		}
	}
}

/**
 * load all playerdata from savegame and update UI accordingly
 */
void FullscreenMenuLaunchMPG::load_previous_playerdata()
{
	std::unique_ptr<FileSystem> l_fs(g_fs->make_sub_file_system(m_settings->settings().mapfilename.c_str()));
	Profile prof;
	prof.read("map/player_names", nullptr, *l_fs);
	std::string infotext = _("Saved players are:");
	std::string player_save_name [MAX_PLAYERS];
	std::string player_save_tribe[MAX_PLAYERS];
	std::string player_save_ai   [MAX_PLAYERS];

	uint8_t i = 1;
	for (; i <= m_nr_players; ++i) {
		infotext += "\n* ";
		Section & s = prof.get_safe_section((boost::format("player_%u")
														 % static_cast<unsigned int>(i)).str());
		player_save_name [i - 1] = s.get_string("name");
		player_save_tribe[i - 1] = s.get_string("tribe");
		player_save_ai   [i - 1] = s.get_string("ai");

		infotext += (boost::format(_("Player %u")) % static_cast<unsigned int>(i)).str();
		if (player_save_tribe[i - 1].empty()) {
			std::string closed_string =
				(boost::format("\\<%s\\>") % _("closed")).str();
			infotext += ":\n    ";
			infotext += closed_string;
			// Close the player
			m_settings->set_player_state(i - 1, PlayerSettings::stateClosed);
			continue; // if tribe is empty, the player does not exist
		}

		// Set team to "none" - to get the real team, we would need to load the savegame completely
		// Do we want that? No! So we just reset teams to not confuse the clients.
		m_settings->set_player_team(i - 1, 0);

		if (player_save_ai[i - 1].empty()) {
			// Assure that player is open
			if (m_settings->settings().players.at(i - 1).state != PlayerSettings::stateHuman)
				m_settings->set_player_state(i - 1, PlayerSettings::stateOpen);
		} else {
			m_settings->set_player_state(i - 1, PlayerSettings::stateComputer);
			m_settings->set_player_ai(i - 1, player_save_ai[i - 1]);
		}

		// Set player's tribe
		m_settings->set_player_tribe(i - 1, player_save_tribe[i - 1]);

		// get translated tribename
		Profile tribe((new std::string("tribes/" + player_save_tribe[i - 1] + "/conf"))->c_str(),
				nullptr, "tribe_" + player_save_tribe[i - 1]);
		Section & global = tribe.get_safe_section("tribe");
		player_save_tribe[i - 1] = global.get_safe_string("name");
		infotext += " (";
		infotext += player_save_tribe[i - 1];
		infotext += "):\n    ";
		// Check if this is a list of names, or just one name:
		if (player_save_name[i - 1].compare(0, 1, " "))
			infotext += player_save_name[i - 1];
		else {
			std::string temp = player_save_name[i - 1];
			bool firstrun = true;
			while (temp.find(' ', 1) < temp.size()) {
				if (firstrun)
					firstrun = false;
				else
					infotext += "\n    ";
				uint32_t x = temp.find(' ', 1);
				infotext += temp.substr(1, x);
				temp = temp.substr(x + 1, temp.size());
			}
		}
	}
	m_map_info.set_text(infotext);
	m_filename_proof = m_settings->settings().mapfilename;
}

/**
 * load map informations and update the UI
 */
void FullscreenMenuLaunchMPG::load_map_info()
{
	Widelands::Map map; //  MapLoader needs a place to put its preload data

	std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(m_settings->settings().mapfilename);
	if (!ml) {
		throw WLWarning("There was an error!", "The map file seems to be invalid!");
	}

	map.set_filename(m_settings->settings().mapfilename);
	{
		i18n::Textdomain td("maps");
		ml->preload_map(true);
	}

	std::string infotext;
	infotext += std::string(_("Map details:")) + "\n";
	infotext += std::string("• ") + (boost::format(_("Size: %1$u x %2$u"))
					 % map.get_width() % map.get_height()).str() + "\n";
	infotext += std::string("• ") + (boost::format(ngettext("%u Player", "%u Players", m_nr_players))
					 % m_nr_players).str() + "\n";
	if (m_settings->settings().scenario)
		infotext += std::string("• ") + (boost::format(_("Scenario mode selected"))).str() + "\n";
	infotext += "\n";
	infotext += map.get_description();
	infotext += "\n";
	infotext += map.get_hint();

	m_map_info.set_text(infotext);
	m_filename_proof = m_settings->settings().mapfilename;

	m_suggested_teams_box->hide();
	m_suggested_teams_box->show(map.get_suggested_teams());
	m_suggested_teams_box->set_pos(
				Point(m_suggested_teams_box->get_x(),
						m_back.get_y() - m_padding - m_suggested_teams_box->get_h() - m_padding));
}

/// Show help
void FullscreenMenuLaunchMPG::help_clicked() {
	if (m_help)
		delete m_help;
	m_help = new UI::HelpWindow(this, _("Multiplayer Game Setup"), m_fs);
	m_help->add_paragraph(_("You are in the multiplayer launch game menu."));
	m_help->add_heading(_("Client settings"));
	m_help->add_paragraph
		(_
		 ("On the left side is a list of all clients including you. You can set your role "
		  "with the button following your nickname. Available roles are:"));
	m_help->add_picture_li
		(_
		 ("The player with the color of the flag. If more than one client selected the same color, these "
		  "share control over the player (‘shared kingdom mode’)."),
		 "pics/genstats_enable_plr_08.png");
	m_help->add_picture_li
		(_("Spectator mode, meaning you can see everything, but cannot control any player"),
		"pics/menu_tab_watch.png");
	m_help->add_heading(_("Player settings"));
	m_help->add_paragraph
		(_
		 ("In the middle are the settings for the players. To start a game, each player must be one of the "
		  "following:"));
	m_help->add_picture_li
		(_("Connected to one or more clients (see ‘Client settings’)."), "pics/genstats_nrworkers.png");
	m_help->add_picture_li
		(_
		 ("Connected to a computer player (the face in the picture as well as the mouse hover texts "
		  "indicate the strength of the currently selected computer player)."),
		"pics/ai_Normal.png");
	m_help->add_picture_li(_("Set as shared in starting position for another player."), "pics/shared_in.png");
	m_help->add_picture_li(_("Closed."), "pics/stop.png");
	m_help->add_block
		(_
		 ("The latter three can only be set by the hosting client by left-clicking the ‘type’ button of a "
		  "player. Hosting players can also set the initialization of each player (the set of buildings, "
		  "wares and workers the player starts with) and the tribe and team for computer players"));
	m_help->add_block
		(_
		 ("Every client connected to a player (the set ‘role’ player) can set the tribe and the team "
		  "for that player"));
	m_help->add_heading(_("Map details"));
	m_help->add_paragraph
		(_
		 ("You can see information about the selected map or savegame on the right-hand side. "
		  "A button next to the map name allows the host to change to a different map. "
		  "Furthermore, the host is able to set a specific win condition, and finally "
		  "can start the game as soon as all players are set up."));
}
