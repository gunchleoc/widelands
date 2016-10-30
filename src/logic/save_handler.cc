/*
 * Copyright (C) 2002-2009, 2015 by the Widelands Development Team
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

#include "logic/save_handler.h"

#include <cstring>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/scoped_timer.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "wlapplication.h"
#include "wui/interactive_base.h"

using Widelands::GameSaver;

/**
* Check if autosave is not needed.
 */
void SaveHandler::think(Widelands::Game& game) {
	uint32_t realtime = SDL_GetTicks();
	initialize(realtime);
	std::string filename = autosave_filename_;

	if (!allow_saving_) {
		return;
	}
	if (game.is_replay()) {
		return;
	}

	// Are we saving now?
	if (saving_next_tick_ || save_requested_) {
		if (save_requested_) {
			// Requested by user
			if (!save_filename_.empty()) {
				filename = save_filename_;
			}
			log("Gamesave: save requested: %s\n", filename.c_str());
			save_requested_ = false;
			save_filename_ = "";
		} else {
			// Autosave ...
			// Roll savefiles
			int32_t number_of_rolls =
			   g_options.pull_section("global").get_int("rolling_autosave", 5) - 1;
			log("Autosave: Rolling savefiles (count): %d\n", number_of_rolls + 1);
			std::string filename_previous = create_file_name(
			   get_base_dir(), (boost::format("%s_%02d") % filename % number_of_rolls).str());
			if (number_of_rolls > 0 && g_fs->file_exists(filename_previous)) {
				g_fs->fs_unlink(filename_previous);
				log("Autosave: Deleted %s\n", filename_previous.c_str());
			}
			number_of_rolls--;
			while (number_of_rolls >= 0) {
				const std::string filename_next = create_file_name(
				   get_base_dir(), (boost::format("%s_%02d") % filename % number_of_rolls).str());
				if (g_fs->file_exists(filename_next)) {
					g_fs->fs_rename(filename_next, filename_previous);
					log("Autosave: Rolled %s to %s\n", filename_next.c_str(), filename_previous.c_str());
				}
				filename_previous = filename_next;
				number_of_rolls--;
			}
			filename = (boost::format("%s_00") % autosave_filename_).str();
			log("Autosave: saving as %s\n", filename.c_str());
		}

		// Saving now
		const std::string complete_filename = create_file_name(get_base_dir(), filename);
		std::string backup_filename;

		// always overwrite a file
		if (g_fs->file_exists(complete_filename)) {
			filename += "2";
			backup_filename = create_file_name(get_base_dir(), filename);
			if (g_fs->file_exists(backup_filename)) {
				g_fs->fs_unlink(backup_filename);
			}
			g_fs->fs_rename(complete_filename, backup_filename);
		}

		std::string error;
		if (!save_game(game, complete_filename, &error)) {
			log("Autosave: ERROR! - %s\n", error.c_str());
			game.get_ibase()->log_message(_("Saving failed!"));

			// if backup file was created, move it back
			if (backup_filename.length() > 0) {
				if (g_fs->file_exists(complete_filename)) {
					g_fs->fs_unlink(complete_filename);
				}
				g_fs->fs_rename(backup_filename, complete_filename);
			}
			// Wait 30 seconds until next save try
			last_saved_realtime_ = last_saved_realtime_ + 30000;
			return;
		} else {
			// if backup file was created, time to remove it
			if (backup_filename.length() > 0 && g_fs->file_exists(backup_filename))
				g_fs->fs_unlink(backup_filename);
		}

		log("Autosave: save took %d ms\n", SDL_GetTicks() - realtime);
		game.get_ibase()->log_message(_("Game saved"));
		last_saved_realtime_ = realtime;
		saving_next_tick_ = false;

	} else {
		// Perhaps save is due now?
		const int32_t autosave_interval_in_seconds =
		   g_options.pull_section("global").get_int("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
		if (autosave_interval_in_seconds <= 0) {
			return;  // no autosave requested
		}

		const int32_t elapsed = (realtime - last_saved_realtime_) / 1000;
		if (elapsed < autosave_interval_in_seconds) {
			return;
		}

		// check if game is paused (in any way)
		if (game.game_controller()->is_paused_or_zero_speed()) {
			// Wait 30 seconds until next save try
			last_saved_realtime_ = last_saved_realtime_ + 30000;
			return;
		}
		log("Autosave: %d s interval elapsed, current gametime: %s, saving...\n", elapsed,
		    gametimestring(game.get_gametime(), true).c_str());

		saving_next_tick_ = true;
		game.get_ibase()->log_message(_("Saving game…"));
	}
}

/**
* Initialize autosave timer
 */
void SaveHandler::initialize(uint32_t realtime) {
	if (initialized_)
		return;

	last_saved_realtime_ = realtime;
	initialized_ = true;
}

/*
 * Calculate the name of the save file
 */
std::string SaveHandler::create_file_name(const std::string& dir,
                                          const std::string& filename) const {
	// Append directory name.
	std::string complete_filename = dir + g_fs->file_separator() + filename;
	// Trim it for preceding/trailing whitespaces in user input
	boost::trim(complete_filename);

	// Now check if the extension matches (ignoring case)
	if (!boost::iends_with(filename, WLGF_SUFFIX)) {
		complete_filename += WLGF_SUFFIX;
	}

	return complete_filename;
}

/*
 * Save the game
 *
 * returns true if saved
 */
bool SaveHandler::save_game(Widelands::Game& game,
                            const std::string& complete_filename,
                            std::string* const error) {
	ScopedTimer save_timer("SaveHandler::save_game() took %ums");

	bool const binary = !g_options.pull_section("global").get_bool("nozip", false);
	// Make sure that the base directory exists
	g_fs->ensure_directory_exists(get_base_dir());

	// Make a filesystem out of this
	std::unique_ptr<FileSystem> fs;
	if (!binary) {
		fs.reset(g_fs->create_sub_file_system(complete_filename, FileSystem::DIR));
	} else {
		fs.reset(g_fs->create_sub_file_system(complete_filename, FileSystem::ZIP));
	}

	bool result = true;
	GameSaver gs(*fs, game);
	try {
		gs.save();
	} catch (const std::exception& e) {
		if (error)
			*error = e.what();
		result = false;
	}

	return result;
}
