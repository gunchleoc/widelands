/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/game_message_menu.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/time_string.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "logic/map_objects/map_object.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "wui/interactive_player.h"
#include "wlapplication.h" // NOCOM Back reference to Widelands. We need to find a better place to put this. Graphic?

using Widelands::Message;
using Widelands::MessageId;
using Widelands::MessageQueue;

inline InteractivePlayer & GameMessageMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

constexpr int kWindowWidth = 355;
constexpr int kWindowHeight = 375;
constexpr int kTableHeight = 125;
constexpr int kPadding = 5;
constexpr int kButtonSize = 34;
constexpr int kMessageBodyY = kButtonSize + 3 * kPadding + kTableHeight;


GameMessageMenu::GameMessageMenu
	(InteractivePlayer & plr, UI::UniqueWindow::Registry & registry)
	:
	UI::UniqueWindow
		(&plr, "messages", &registry, kWindowWidth, kWindowHeight, _("Messages: Inbox")),
	hotkey_scope_("game_message_menu"),
	all_messages_hotkey_(WLApplication::get()->hotkeys()->add_hotkey(hotkey_scope_, "all_messages", _("All Messages"), SDLK_0)),
	message_body
		(this,
		 kPadding,
		 kMessageBodyY,
		 kWindowWidth - 2 * kPadding,
		 get_inner_h() - kMessageBodyY - 2 * kPadding - kButtonSize,
		 "", UI::Align::kLeft, 1),
	mode(Inbox)
{

	list = new UI::Table<uintptr_t>(
				 this,
				 kPadding,
				 kButtonSize + 2 * kPadding,
				 kWindowWidth - 2 * kPadding,
				 kTableHeight);
	list->selected.connect(boost::bind(&GameMessageMenu::selected, this, _1));
	list->double_clicked.connect(boost::bind(&GameMessageMenu::double_clicked, this, _1));
	list->add_column(kWindowWidth - 2 * kPadding - 60 - 60 - 75, _("Title"));
	list->add_column (60, pgettext("message", "Type"), "", UI::Align::kHCenter, true);
	list->add_column (60, _("Status"), "", UI::Align::kHCenter);
	/** TRANSLATORS: We have very little space here. You can also translate this as "Time" or "Time Sent" */
	/** TRANSLATORS: This is used in the game messages menu - please open an issue if you need more space. */
	list->add_column(75, pgettext("message", "Sent"), "", UI::Align::kRight);
	list->focus();

	// Buttons for message types
	/** TRANSLATORS: Tooltip in the messages window */
	const std::string pressed_tooltip = _("Show all messages");

	geologistsbtn_ =
			new UI::Button
				(this, "filter_geologists_messages",
				 kPadding, kPadding, kButtonSize, kButtonSize,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_geologist.png"),
				 /** TRANSLATORS: Tooltip in the messages window */
				 _("Show geologists' messages only"),
				 true);
	geologistsbtn_->set_pressed_tooltip(pressed_tooltip);
	geologistsbtn_->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kGeologists));

	economybtn_ =
			new UI::Button
				(this, "filter_economy_messages",
				 2 * kPadding + kButtonSize, kPadding, kButtonSize, kButtonSize,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/genstats_nrwares.png"),
				 /** TRANSLATORS: Tooltip in the messages window */
				 _("Show economy messages only"),
				 true);
	economybtn_->set_pressed_tooltip(pressed_tooltip);
	economybtn_->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kEconomy));

	seafaringbtn_ =
			new UI::Button
				(this, "filter_seafaring_messages",
				 3 * kPadding + 2 * kButtonSize, kPadding, kButtonSize, kButtonSize,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/start_expedition.png"),
				 /** TRANSLATORS: Tooltip in the messages window */
				 _("Show seafaring messages only"),
				 true);
	seafaringbtn_->set_pressed_tooltip(pressed_tooltip);
	seafaringbtn_->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kSeafaring));

	warfarebtn_ =
			new UI::Button
				(this, "filter_warfare_messages",
				 4 * kPadding + 3 * kButtonSize, kPadding, kButtonSize, kButtonSize,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/messages_warfare.png"),
				 /** TRANSLATORS: Tooltip in the messages window */
				 _("Show warfare messages only"),
				 true);
	warfarebtn_->set_pressed_tooltip(pressed_tooltip);
	warfarebtn_->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kWarfare));

	scenariobtn_ =
			new UI::Button
				(this, "filter_scenario_messages",
				 5 * kPadding + 4 * kButtonSize, kPadding, kButtonSize, kButtonSize,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_objectives.png"),
				 /** TRANSLATORS: Tooltip in the messages window */
				 _("Show scenario messages only"),
				 true);
	scenariobtn_->set_pressed_tooltip(pressed_tooltip);
	scenariobtn_->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kScenario));

	message_filter_ = Widelands::Message::Type::kAllMessages;

	// End: Buttons for message types

	archivebtn_ =
		new UI::Button
			(this, "archive_or_restore_selected_messages",
			 kPadding, kWindowHeight - kPadding - kButtonSize, kButtonSize, kButtonSize,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_archive.png"),
			 /** TRANSLATORS: Tooltip in the messages window */
			 _("Archive selected messages"));
	archivebtn_->sigclicked.connect
		(boost::bind(&GameMessageMenu::archive_or_restore, this));

	togglemodebtn_ =
		new UI::Button
			(this, "toggle_between_inbox_or_archive",
			 archivebtn_->get_x() + archivebtn_->get_w() + kPadding,
			 archivebtn_->get_y(),
			 kButtonSize,
			 kButtonSize,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_archived.png"),
			 _("Show Archive"));
	togglemodebtn_->sigclicked.connect
		(boost::bind(&GameMessageMenu::toggle_mode, this));

	centerviewbtn_ =
		new UI::Button
			(this, "center_main_mapview_on_location",
			 kWindowWidth - kPadding - kButtonSize, archivebtn_->get_y(), kButtonSize, kButtonSize,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/menu_goto.png"),
			 /** TRANSLATORS: Tooltip in the messages window */
			 _("Center main mapview on location"),
			 false);
	centerviewbtn_->sigclicked.connect(boost::bind(&GameMessageMenu::center_view, this));

	if (get_usedefaultpos())
		center_to_parent();

	list->set_column_compare
		(ColTitle, boost::bind(&GameMessageMenu::compare_title, this, _1, _2));
	list->set_column_compare
		(ColStatus, boost::bind(&GameMessageMenu::compare_status, this, _1, _2));
	list->set_column_compare
		(ColType,
		 boost::bind(&GameMessageMenu::compare_type, this, _1, _2));
	list->set_column_compare
		(ColTimeSent,
		 boost::bind(&GameMessageMenu::compare_time_sent, this, _1, _2));

	list->set_sort_column(ColTimeSent);

	set_can_focus(true);
	focus();

	// NOCOM hotkeys are broken.
	geologistsbtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_1), all_messages_hotkey_);
	economybtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_2), all_messages_hotkey_);
	seafaringbtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_3), all_messages_hotkey_);
	warfarebtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_4), all_messages_hotkey_);
	scenariobtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_5), all_messages_hotkey_);

	archivebtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_DELETE));
	centerviewbtn_->set_hotkey(hotkey_scope_, UI::Hotkeys::HotkeyCode(SDLK_g));

}

/**
 * When comparing messages by title, order is alphabetical.
 * If both are identical, sort by time sent.
 */
bool GameMessageMenu::compare_title(uint32_t a, uint32_t b)
{
	MessageQueue & mq = iplayer().player().messages();
	const Message * msga = mq[MessageId((*list)[a])];
	const Message * msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		if (msga->title() == msgb->title()) {
			return compare_time_sent(a, b);
		}
		return msga->title() < msgb->title();
	}
	return false; // shouldn't happen
}

/**
 * When comparing messages by status, new messages come before others.
 * If both are identical, sort by time sent.
 */
bool GameMessageMenu::compare_status(uint32_t a, uint32_t b)
{
	MessageQueue & mq = iplayer().player().messages();
	const Message * msga = mq[MessageId((*list)[a])];
	const Message * msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		if (msga->status() == msgb->status()) {
			return compare_time_sent(a, b);
		}
		return msga->status() == Message::Status::kNew && msgb->status() != Message::Status::kNew;
	}
	return false; // shouldn't happen
}

/**
 * When comparing messages by type, order is the same as in the enum class.
 * If both are identical, sort by time sent.
 */
bool GameMessageMenu::compare_type(uint32_t a, uint32_t b)
{
	MessageQueue & mq = iplayer().player().messages();
	const Message * msga = mq[MessageId((*list)[a])];
	const Message * msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		const Widelands::Message::Type cat_a = msga->message_type_category();
		const Widelands::Message::Type cat_b = msgb->message_type_category();
		if (cat_a == cat_b) {
			return compare_time_sent(a, b);
		}
		return static_cast<int>(cat_a) < static_cast<int>(cat_b);
	}
	return false; // shouldn't happen
}

/**
 * When comparing messages by time sent, older messages come before others.
 */
bool GameMessageMenu::compare_time_sent(uint32_t a, uint32_t b)
{
	MessageQueue & mq = iplayer().player().messages();
	const Message * msga = mq[MessageId((*list)[a])];
	const Message * msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		return msga->sent() > msgb->sent();
	}
	return false; // shouldn't happen
}


static char const * const status_picture_filename[] = {
	"pics/message_new.png",
	"pics/message_read.png",
	"pics/message_archived.png"
};

void GameMessageMenu::show_new_message
	(MessageId const id, const Widelands::Message & message)
{
	assert(iplayer().player().messages()[id] == &message);
	assert(!list->find(id.value()));
	Message::Status const status = message.status();
	if ((mode == Archive) != (status == Message::Status::kArchived))
		toggle_mode();
	UI::Table<uintptr_t>::EntryRecord & te = list->add(id.value(), true);
	update_record(te, message);
	list->sort();
}

void GameMessageMenu::think()
{
	MessageQueue & mq = iplayer().player().messages();

	// Update messages in the list and remove messages
	// that should no longer be shown
	for (uint32_t j = list->size(); j; --j) {
		MessageId id_((*list)[j - 1]);
		if (Message const * const message = mq[id_]) {
			if ((mode == Archive) != (message->status() == Message::Status::kArchived)) {
				list->remove(j - 1);
			} else {
				update_record(list->get_record(j - 1), *message);
			}
		} else {
			list->remove(j - 1);
		}
	}

	// Add new messages to the list
	for (const std::pair<MessageId, Message *>& temp_message : mq) {
		MessageId      const id      =  temp_message.first;
		const Message &       message = *temp_message.second;
		Message::Status const status  = message.status();
		if ((mode == Archive) != (status == Message::Status::kArchived))
			continue;
		if (!list->find(id.value())) {
			UI::Table<uintptr_t>::EntryRecord & er = list->add(id.value());
			update_record(er, message);
			list->sort();
		}
	}

	// Filter message type
	if (message_filter_ != Message::Type::kAllMessages) {
		for (uint32_t j = list->size(); j; --j) {
			MessageId id_((*list)[j - 1]);
			if (Message const * const message = mq[id_]) {
				if (message->message_type_category() != message_filter_) {
					list->remove(j - 1);
				}
			}
		}
	}

	if (list->size()) {
		if (!list->has_selection())
			list->select(0);
	} else {
		centerviewbtn_->set_enabled(false);
		message_body.set_text(std::string());
	}
}

void GameMessageMenu::update_record(UI::Table<uintptr_t>::EntryRecord& er, const Widelands::Message& message)
{
	er.set_picture(ColType, g_gr->images().get(display_message_type_icon(message)));
	er.set_picture
		(ColStatus,
		 g_gr->images().get(status_picture_filename[static_cast<int>(message.status())]));
	er.set_picture(ColTitle, message.icon(), message.title());

	const uint32_t time = message.sent();
	er.set_string(ColTimeSent, gametimestring(time));
}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	Widelands::Player & player = iplayer().player  ();
	MessageQueue      & mq     = player   .messages();
	if (t != UI::Table<uintptr_t>::no_selection_index()) {
		MessageId const id = MessageId((*list)[t]);
		if (Message const * const message = mq[id]) {
			//  Maybe the message was removed since think?
			if (message->status() == Message::Status::kNew) {
				Widelands::Game & game = iplayer().game();
				game.send_player_command
					(*new Widelands::CmdMessageSetStatusRead
					 	(game.get_gametime(), player.player_number(), id));
			}
			centerviewbtn_->set_enabled(message->position());

			message_body.set_text(
						(boost::format("<rt><p font-size=18 font-weight=bold font-color=D1D1D1>%s<br></p>"
											"<p font-size=8> <br></p></rt>%s")
						 % message->heading()
						 % message->body()).str());
			return;
		}
	}
	centerviewbtn_->set_enabled(false);
	message_body.set_text(std::string());
}

/**
 * a message was double clicked
 */
void GameMessageMenu::double_clicked(uint32_t const /* t */) {
	if (centerviewbtn_->enabled()) center_view();
}

/**
 * Handle message menu hotkeys.
 */
bool GameMessageMenu::handle_key(bool down, SDL_Keysym code)
{
	if (down) {
		// Don't forget to change the tooltips if any of these get reassigned
		// NOCOM allow key combination
		if (WLApplication::get()->hotkeys()->is_hotkey_pressed(all_messages_hotkey_, code)) {
			filter_messages(Widelands::Message::Type::kAllMessages);
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(geologistsbtn_->get_hotkey(), code)) {
			filter_messages(Widelands::Message::Type::kGeologists);
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(economybtn_->get_hotkey(), code)) {
			filter_messages(Widelands::Message::Type::kEconomy);
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(seafaringbtn_->get_hotkey(), code)) {
			filter_messages(Widelands::Message::Type::kSeafaring);
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(warfarebtn_->get_hotkey(), code)) {
			filter_messages(Widelands::Message::Type::kWarfare);
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(scenariobtn_->get_hotkey(), code)) {
			filter_messages(Widelands::Message::Type::kScenario);
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(centerviewbtn_->get_hotkey(), code)) {
			if (centerviewbtn_->enabled()) {
				center_view();
			}
			return true;
		} else if (WLApplication::get()->hotkeys()->is_hotkey_pressed(archivebtn_->get_hotkey(), code)) {
			archive_or_restore();
			return true;
		}

		// NOCOM what it this for?
		switch (code.sym) {
			case SDL_SCANCODE_KP_PERIOD:
			case SDLK_KP_PERIOD:
				if (WLApplication::get()->hotkeys()->use_numlock())
					break;
				/* no break */
			default:
				break; // not handled
		}
	}

	return list->handle_key(down, code);
}

void GameMessageMenu::archive_or_restore()
{
	Widelands::Game         &       game     = iplayer().game();
	uint32_t                  const gametime = game.get_gametime();
	Widelands::Player       &       player   = iplayer().player();
	Widelands::PlayerNumber  const plnum    = player.player_number();
	bool work_done = false;

	switch (mode) {
	case Inbox:
		//archive highlighted message
		if (!work_done) {
			if (!list->has_selection()) return;

			game.send_player_command
				(*new Widelands::CmdMessageSetStatusArchived
					(gametime, plnum, MessageId(list->get_selected())));
		}
		break;
	case Archive:
		//restore highlighted message
		if (!work_done) {
			if (!list->has_selection()) return;

			game.send_player_command
				(*new Widelands::CmdMessageSetStatusRead
					(gametime, plnum, MessageId(list->get_selected())));
		}
		break;
	}
}

void GameMessageMenu::center_view()
{
	size_t const selection = list->selection_index();
	assert(selection < list->size());
	if
		(Message const * const message =
		 	iplayer().player().messages()[MessageId((*list)[selection])])
	{
		assert(message->position());
		iplayer().move_view_to(message->position());
	}
}

/**
 * Show only messages of a certain type
 * @param msgtype the types of messages to show
 */
void GameMessageMenu::filter_messages(Widelands::Message::Type const msgtype) {
	switch (msgtype) {
		case Widelands::Message::Type::kGeologists:
			toggle_filter_messages_button(*geologistsbtn_, msgtype);
			break;
		case Widelands::Message::Type::kEconomy:
			toggle_filter_messages_button(*economybtn_, msgtype);
			break;
		case Widelands::Message::Type::kSeafaring:
			toggle_filter_messages_button(*seafaringbtn_, msgtype);
			break;
		case Widelands::Message::Type::kWarfare:
			toggle_filter_messages_button(*warfarebtn_, msgtype);
			break;
		case Widelands::Message::Type::kScenario:
			toggle_filter_messages_button(*scenariobtn_, msgtype);
			break;
		case Widelands::Message::Type::kNoMessages:
		case Widelands::Message::Type::kAllMessages:
		case Widelands::Message::Type::kGameLogic:
		case Widelands::Message::Type::kGeologistsCoal:
		case Widelands::Message::Type::kGeologistsGold:
		case Widelands::Message::Type::kGeologistsStones:
		case Widelands::Message::Type::kGeologistsIron:
		case Widelands::Message::Type::kGeologistsWater:
		case Widelands::Message::Type::kEconomySiteOccupied:
		case Widelands::Message::Type::kWarfareSiteDefeated:
		case Widelands::Message::Type::kWarfareSiteLost:
		case Widelands::Message::Type::kWarfareUnderAttack:
			// NOCOM set_filter_messages_tooltips();
			message_filter_ = Widelands::Message::Type::kAllMessages;
			geologistsbtn_->set_perm_pressed(false);
			economybtn_   ->set_perm_pressed(false);
			seafaringbtn_ ->set_perm_pressed(false);
			warfarebtn_   ->set_perm_pressed(false);
			scenariobtn_  ->set_perm_pressed(false);
			break;
	}
	think();
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::toggle_filter_messages_button(UI::Button & button, Widelands::Message::Type msgtype) {
	if (button.get_perm_pressed()) {
		button.set_perm_pressed(false);
		message_filter_ = Widelands::Message::Type::kAllMessages;
	} else {
		geologistsbtn_->set_perm_pressed(false);
		economybtn_->set_perm_pressed(false);
		seafaringbtn_->set_perm_pressed(false);
		warfarebtn_->set_perm_pressed(false);
		scenariobtn_->set_perm_pressed(false);
		button.set_perm_pressed(true);
		message_filter_ = msgtype;
	}
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::set_filter_messages_tooltips() {
	// NOCOM get hotkey tooltips from the database
	geologistsbtn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
											/** TRANSLATORS: Tooltip in the messages window */
											% _("Show geologists' messages only")
											% "1").str());
	economybtn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										/** TRANSLATORS: Tooltip in the messages window */
										% _("Show economy messages only")
										% "2").str());
	seafaringbtn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										  /** TRANSLATORS: Tooltip in the messages window */
										  % _("Show seafaring messages only")
										  % "3").str());
	warfarebtn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										/** TRANSLATORS: Tooltip in the messages window */
										% _("Show warfare messages only")
										% "4").str());
	scenariobtn_->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										 /** TRANSLATORS: Tooltip in the messages window */
										 % _("Show scenario messages only")
										 % "5").str());
}


/**
 * Get the filename for a message category's icon
 */
std::string GameMessageMenu::display_message_type_icon(Widelands::Message message) {
	switch (message.message_type_category()) {
		case Widelands::Message::Type::kGeologists:
			return "pics/menu_geologist.png";
		case Widelands::Message::Type::kEconomy:
			return "pics/genstats_nrwares.png";
		case Widelands::Message::Type::kSeafaring:
			return "pics/start_expedition.png";
		case Widelands::Message::Type::kWarfare:
			return "pics/messages_warfare.png";
		case Widelands::Message::Type::kScenario:
			return "pics/menu_objectives.png";
		case Widelands::Message::Type::kGameLogic:
			return "pics/menu_help.png";
		case Widelands::Message::Type::kNoMessages:
		case Widelands::Message::Type::kAllMessages:
		case Widelands::Message::Type::kGeologistsCoal:
		case Widelands::Message::Type::kGeologistsGold:
		case Widelands::Message::Type::kGeologistsStones:
		case Widelands::Message::Type::kGeologistsIron:
		case Widelands::Message::Type::kGeologistsWater:
		case Widelands::Message::Type::kEconomySiteOccupied:
		case Widelands::Message::Type::kWarfareSiteDefeated:
		case Widelands::Message::Type::kWarfareSiteLost:
		case Widelands::Message::Type::kWarfareUnderAttack:
			return "pics/message_new.png";
	}
	NEVER_HERE();
}


void GameMessageMenu::toggle_mode()
{
	list->clear();
	switch (mode) {
	case Inbox:
		mode = Archive;
		set_title(_("Messages: Archive"));
		archivebtn_->set_pic(g_gr->images().get("pics/message_restore.png"));
		archivebtn_->set_tooltip(_("Restore selected messages"));
		togglemodebtn_->set_pic(g_gr->images().get("pics/message_new.png"));
		togglemodebtn_->set_tooltip(_("Show Inbox"));
		break;
	case Archive:
		mode = Inbox;
		set_title(_("Messages: Inbox"));
		archivebtn_->set_pic(g_gr->images().get("pics/message_archive.png"));
		archivebtn_->set_tooltip(_("Archive selected messages"));
		togglemodebtn_->set_pic(g_gr->images().get("pics/message_archived.png"));
		togglemodebtn_->set_tooltip(_("Show Archive"));
		break;
	}
}
