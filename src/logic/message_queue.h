/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_MESSAGE_QUEUE_H
#define WL_LOGIC_MESSAGE_QUEUE_H

#include <cassert>
#include <map>

#include "base/macros.h"
#include "logic/message.h"
#include "logic/message_id.h"

namespace Widelands {

struct MessageQueue : private std::map<MessageId, Message*> {
	friend class MapPlayersMessagesPacket;

	MessageQueue() {
		counts_[static_cast<int>(Message::Status::kNew)] = 0;       //  C++0x:
		counts_[static_cast<int>(Message::Status::kRead)] = 0;      //  C++0x:
		counts_[static_cast<int>(Message::Status::kArchived)] = 0;  //  C++0x:
	}                                                              //  C++0x:

	~MessageQueue() {
		while (size()) {
			delete begin()->second;
			erase(begin());
		}
	}

	//  Make some selected inherited members public.
	MessageQueue::const_iterator begin() const {
		return std::map<MessageId, Message*>::begin();
	}
	MessageQueue::const_iterator end() const {
		return std::map<MessageId, Message*>::end();
	}
	size_type count(uint32_t const i) const {
		assert_counts();
		return std::map<MessageId, Message*>::count(MessageId(i));
	}

	/// \returns a pointer to the message if it exists, otherwise 0.
	Message const* operator[](const MessageId& id) const {
		assert_counts();
		MessageQueue::const_iterator const it = find(MessageId(id));
		return it != end() ? it->second : nullptr;
	}

	/// \returns the number of messages with the given status.
	uint32_t nr_messages(Message::Status const status) const {
		assert_counts();
		assert(static_cast<int>(status) < 3);
		return counts_[static_cast<int>(status)];
	}

	/// Adds the message. Takes ownership of the message. Assumes that it has
	/// been allocated in a separate memory block (not as a component of an
	/// array or struct) with operator new, so that it can be deallocated with
	/// operator delete.
	///
	/// \returns the id of the added message.
	///
	/// The loading code calls this function to add messages form the map file.
	MessageId add_message(Message& message) {
		assert_counts();
		assert(static_cast<int>(message.status()) < 3);
		++counts_[static_cast<int>(message.status())];
		insert(std::map<MessageId, Message*>::end(),
		       std::pair<MessageId, Message*>(++current_message_id_, &message));
		assert_counts();
		return current_message_id_;
	}

	/// Sets the status of the message with the given id, if it exists.
	void set_message_status(const MessageId& id, Message::Status const status) {
		assert_counts();
		assert(static_cast<int>(status) < 3);
		MessageQueue::iterator const it = find(id);
		if (it != end()) {
			Message& message = *it->second;
			assert(static_cast<int>(it->second->status()) < 3);
			assert(counts_[static_cast<int>(message.status())]);
			--counts_[static_cast<int>(message.status())];
			++counts_[static_cast<int>(message.set_status(status))];
		}
		assert_counts();
	}

	/// Delete the message with the given id so that it no longer exists.
	/// Assumes that a message with the given id exists.
	void delete_message(const MessageId& id) {
		assert_counts();
		MessageQueue::iterator const it = find(id);
		if (it == end()) {
			// Messages can be deleted when the linked MapObject is removed. Two delete commands
			// will be executed, and the message will not be present for the second one.
			// So we assume here that the message was removed from an earlier delete cmd.
			return;
		}
		Message& message = *it->second;
		assert(static_cast<int>(message.status()) < 3);
		assert(counts_[static_cast<int>(message.status())]);
		--counts_[static_cast<int>(message.status())];
		delete &message;
		erase(it);
		assert_counts();
	}

	MessageId current_message_id() const {
		return current_message_id_;
	}

	/// \returns whether all messages with id 1, 2, 3, ..., current_message_id
	/// exist. This should be the case when messages have been loaded from a map
	/// file/savegame but the simulation has not started to run yet.
	bool is_continuous() const {
		assert_counts();
		return current_message_id().value() == size();
	}

private:
	/// Only for working around bugs in map loading code. If something has
	/// accidentally been added to the queue during load, it can be worked
	/// around by clearing the queue before the saved messages are loaded into
	/// it.
	void clear() {
		assert_counts();
		current_message_id_ = MessageId::null();
		counts_[static_cast<int>(Message::Status::kNew)] = 0;
		counts_[static_cast<int>(Message::Status::kRead)] = 0;
		counts_[static_cast<int>(Message::Status::kArchived)] = 0;
		std::map<MessageId, Message*>::clear();
		assert_counts();
	}

	/// The id of the most recently added message, or null if none has been
	/// added yet.
	MessageId current_message_id_;

	/// Number of messages with each status (new, read, deleted).
	/// Indexed by Message::Status.
	uint32_t counts_[3];

	void assert_counts() const {
		assert(size() ==
		       counts_[static_cast<int>(Message::Status::kNew)] +
		          counts_[static_cast<int>(Message::Status::kRead)] +
		          counts_[static_cast<int>(Message::Status::kArchived)]);
	}

	DISALLOW_COPY_AND_ASSIGN(MessageQueue);
};
}

#endif  // end of include guard: WL_LOGIC_MESSAGE_QUEUE_H
