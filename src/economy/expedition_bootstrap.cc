/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "economy/expedition_bootstrap.h"

#include <memory>

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/portdock.h"
#include "io/filewrite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

struct ExpeditionBootstrap::ExpeditionWorker {
	// Ownership is taken.
	ExpeditionWorker(Request* r) : request(r), worker(nullptr) {
	}

	std::unique_ptr<Request> request;

	// we never own the worker. Either he is in the PortDock in which case it
	// owns it or it is on the ship.
	Worker* worker;
};

ExpeditionBootstrap::ExpeditionBootstrap(PortDock* const portdock)
   : portdock_(portdock), economy_(portdock->get_economy()) {
}

ExpeditionBootstrap::~ExpeditionBootstrap() {
	assert(workers_.empty());
	assert(wares_.empty());
}

void ExpeditionBootstrap::is_ready(Game& game) {
	for (std::unique_ptr<WaresQueue>& wq : wares_) {
		if (wq->get_max_fill() != wq->get_filled())
			return;
	}

	for (std::unique_ptr<ExpeditionWorker>& ew : workers_) {
		if (ew->request)
			return;
	}

	// If this point is reached, all needed wares and workers are stored and waiting for a ship
	portdock_->expedition_bootstrap_complete(game);
}

// static
void ExpeditionBootstrap::ware_callback(Game& game,
                                        WaresQueue*,
                                        DescriptionIndex,
                                        void* const data) {
	ExpeditionBootstrap* eb = static_cast<ExpeditionBootstrap*>(data);
	eb->is_ready(game);
}

// static
void ExpeditionBootstrap::worker_callback(
   Game& game, Request& r, DescriptionIndex, Worker* worker, PlayerImmovable& pi) {
	Warehouse* warehouse = static_cast<Warehouse*>(&pi);

	warehouse->get_portdock()->expedition_bootstrap()->handle_worker_callback(game, r, worker);
}

void ExpeditionBootstrap::handle_worker_callback(Game& game, Request& request, Worker* worker) {
	for (std::unique_ptr<ExpeditionWorker>& ew : workers_) {
		if (ew->request.get() == &request) {
			ew->request.reset(nullptr);  // deletes &request.
			ew->worker = worker;

			// This is not really correct. All the wares are not in the portdock,
			// so why should the worker be there. Well, it has to be somewhere
			// (location != nullptr) and it should be in our economy so he is
			// properly accounted for, so why not in the portdock. Alternatively
			// we could kill him and recreate him as soon as we bord the ship -
			// this should be no problem as workers are not gaining experience.
			worker->set_location(portdock_);
			worker->reset_tasks(game);
			worker->start_task_idle(game, 0, -1);

			is_ready(game);
			return;
		}
	}
	// Never here, otherwise we would have a callback for a request we know
	// nothing about.
	NEVER_HERE();
}

void ExpeditionBootstrap::start() {
	assert(workers_.empty());
	assert(wares_.empty());

	// Load the buildcosts for the port building + builder
	Warehouse* const warehouse = portdock_->get_warehouse();

	const std::map<DescriptionIndex, uint8_t>& buildcost = warehouse->descr().buildcost();
	size_t const buildcost_size = buildcost.size();

	// Issue request for wares for this expedition.
	// TODO(sirver): could try to get some of these directly form the warehouse.
	// But this is really a premature optimization and should probably be
	// handled in the economy code.
	wares_.resize(buildcost_size);
	std::map<DescriptionIndex, uint8_t>::const_iterator it = buildcost.begin();
	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		WaresQueue* wq = new WaresQueue(*warehouse, it->first, it->second);
		wq->set_callback(ware_callback, this);
		wares_[i].reset(wq);
	}

	// Issue the request for the workers (so far only a builder).
	workers_.emplace_back(
	   new ExpeditionWorker(new Request(*warehouse, warehouse->owner().tribe().builder(),
	                                    ExpeditionBootstrap::worker_callback, wwWORKER)));

	// Update the user interface
	if (upcast(InteractiveGameBase, igb, warehouse->owner().egbase().get_ibase()))
		warehouse->refresh_options(*igb);
}

void ExpeditionBootstrap::cancel(Game& game) {
	// Put all wares from the WaresQueues back into the warehouse
	Warehouse* const warehouse = portdock_->get_warehouse();
	for (std::unique_ptr<WaresQueue>& wq : wares_) {
		warehouse->insert_wares(wq->get_ware(), wq->get_filled());
		wq->cleanup();
	}
	wares_.clear();

	// Send all workers from the expedition list back inside the warehouse
	for (std::unique_ptr<ExpeditionWorker>& ew : workers_) {
		if (ew->worker) {
			warehouse->incorporate_worker(game, ew->worker);
		}
	}
	workers_.clear();

	// Update the user interface
	if (upcast(InteractiveGameBase, igb, warehouse->owner().egbase().get_ibase())) {
		warehouse->refresh_options(*igb);
	}
	Notifications::publish(NoteExpeditionCanceled(this));
}

void ExpeditionBootstrap::cleanup(EditorGameBase& /* egbase */) {
	// This will delete all the requests. We do nothing with the workers as we
	// do not own them.
	workers_.clear();

	for (std::unique_ptr<WaresQueue>& wq : wares_) {
		wq->cleanup();
	}
	wares_.clear();
}

WaresQueue& ExpeditionBootstrap::waresqueue(DescriptionIndex index) const {
	for (const std::unique_ptr<WaresQueue>& wq : wares_) {
		if (wq->get_ware() == index) {
			return *wq.get();
		}
	}
	NEVER_HERE();
}

std::vector<WaresQueue*> ExpeditionBootstrap::wares() const {
	std::vector<WaresQueue*> return_value;
	for (const std::unique_ptr<WaresQueue>& wq : wares_) {
		return_value.emplace_back(wq.get());
	}
	return return_value;
}

void ExpeditionBootstrap::set_economy(Economy* new_economy) {
	if (new_economy == economy_)
		return;

	// Transfer the wares.
	for (std::unique_ptr<WaresQueue>& wq : wares_) {
		if (economy_)
			wq->remove_from_economy(*economy_);
		if (new_economy)
			wq->add_to_economy(*new_economy);
	}

	// Transfer the workers.
	for (std::unique_ptr<ExpeditionWorker>& ew : workers_) {
		if (ew->request) {
			ew->request->set_economy(new_economy);
		}
		if (ew->worker)
			ew->worker->set_economy(new_economy);
	}

	economy_ = new_economy;
}

void ExpeditionBootstrap::get_waiting_workers_and_wares(Game& game,
                                                        const TribeDescr& tribe,
                                                        std::vector<Worker*>* return_workers,
                                                        std::vector<WareInstance*>* return_wares) {
	for (std::unique_ptr<WaresQueue>& wq : wares_) {
		const DescriptionIndex ware_index = wq->get_ware();
		for (uint32_t j = 0; j < wq->get_filled(); ++j) {
			WareInstance* temp = new WareInstance(ware_index, tribe.get_ware_descr(ware_index));
			temp->init(game);
			temp->set_location(game, portdock_);
			return_wares->emplace_back(temp);
		}
		wq->set_filled(0);
		wq->set_max_fill(0);
	}

	for (std::unique_ptr<ExpeditionWorker>& ew : workers_) {
		assert(ew->worker != nullptr);
		assert(!ew->request);
		return_workers->emplace_back(ew->worker);
	}

	cleanup(game);
}

void ExpeditionBootstrap::save(FileWrite& fw, Game& game, MapObjectSaver& mos) {
	// Expedition workers
	fw.unsigned_8(workers_.size());
	for (std::unique_ptr<ExpeditionWorker>& ew : workers_) {
		fw.unsigned_8(ew->request.get() != nullptr);
		if (ew->request.get() != nullptr) {
			ew->request->write(fw, game, mos);
		} else {
			assert(mos.is_object_known(*ew->worker));
			fw.unsigned_32(mos.get_object_file_index(*ew->worker));
		}
	}

	// Expedition WaresQueues
	fw.unsigned_8(wares_.size());
	for (std::unique_ptr<WaresQueue>& wq : wares_) {
		wq->write(fw, game, mos);
	}
}

void ExpeditionBootstrap::load(Warehouse& warehouse,
                               FileRead& fr,
                               Game& game,
                               MapObjectLoader& mol) {
	// Expedition workers
	const uint8_t num_workers = fr.unsigned_8();
	for (uint8_t i = 0; i < num_workers; ++i) {
		workers_.emplace_back(new ExpeditionWorker(nullptr));
		if (fr.unsigned_8() == 1) {
			Request* worker_request =
			   new Request(warehouse, 0, ExpeditionBootstrap::worker_callback, wwWORKER);
			workers_.back()->request.reset(worker_request);
			worker_request->read(fr, game, mol);
			workers_.back()->worker = nullptr;
		} else {
			workers_.back()->worker = &mol.get<Worker>(fr.unsigned_32());
		}
	}

	// Expedition WaresQueues
	assert(wares_.empty());
	const uint8_t num_wares = fr.unsigned_8();
	for (uint8_t i = 0; i < num_wares; ++i) {
		WaresQueue* wq = new WaresQueue(warehouse, INVALID_INDEX, 0);
		wq->read(fr, game, mol);
		wq->set_callback(ware_callback, this);

		if (wq->get_ware() == INVALID_INDEX) {
			delete wq;
		} else {
			wares_.emplace_back(wq);
		}
	}
}

}  // namespace Widelands
