/*
 * Copyright (C) 2002-2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_MESSAGE_MENU_H
#define WL_WUI_GAME_MESSAGE_MENU_H

#include "base/i18n.h"
#include "logic/message.h"
#include "logic/message_queue.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class Game;
struct Message;
}
class InteractivePlayer;

///  Shows the not already fulfilled objectives.
struct GameMessageMenu : public UI::UniqueWindow {
	GameMessageMenu(InteractivePlayer &, UI::UniqueWindow::Registry &);

	/// Shows a newly created message. Assumes that the message is not yet in
	/// the list (the message was added to the queue after the last time think()
	/// was executed.
	void show_new_message(Widelands::MessageId, const Widelands::Message &);

	enum Mode {Inbox, Archive};
	void think() override;
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	enum Cols {ColSelect, ColStatus, ColTitle, ColTimeSent};
	enum class ReadUnread: uint8_t {allMessages, readMessages, newMessages};

	InteractivePlayer & iplayer() const;
	void selected(uint32_t);
	void double_clicked(uint32_t);

	bool status_compare(uint32_t a, uint32_t b);
	void do_clear_selection();
	void do_invert_selection();
	void archive_or_restore();
	void toggle_mode();
	void center_view();
	void filter_messages(Widelands::Message::Type);
	void toggle_filter_messages_button(UI::Button &, Widelands::Message::Type);
	void set_display_message_type_label(Widelands::Message::Type);
	void update_record(UI::Table<uintptr_t>::EntryRecord & er, const Widelands::Message &);

	std::string hotkey_scope_;
	UI::Hotkeys::HotkeyCode all_messages_hotkey_;

	UI::Table<uintptr_t> * list;
	UI::MultilineTextarea message_body;
	UI::Button * m_archivebtn;
	UI::Button * m_togglemodebtn;
	UI::Button * m_centerviewbtn;
	Mode mode;
	// Buttons for message types
	UI::Button * m_geologistsbtn;
	UI::Button * m_economybtn;
	UI::Button * m_seafaringbtn;
	UI::Button * m_warfarebtn;
	UI::Button * m_scenariobtn;
	Widelands::Message::Type m_message_filter;
	UI::MultilineTextarea * m_display_message_type_label;
};

#endif  // end of include guard: WL_WUI_GAME_MESSAGE_MENU_H
