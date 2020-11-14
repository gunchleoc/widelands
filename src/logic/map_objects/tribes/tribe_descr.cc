/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "logic/map_objects/tribes/tribe_descr.h"

#include <memory>
#include <stack>

#include "base/i18n.h"
#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "graphic/animation/animation_manager.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/immovable_program.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "scripting/lua_table.h"
#include "ui_basic/note_loading_message.h"

namespace {

// Recursively get attributes for immovable growth cycle
void walk_immovables(Widelands::DescriptionIndex index,
                     const Widelands::Descriptions& descriptions,
                     std::set<Widelands::DescriptionIndex>* walked_immovables,
                     const std::set<Widelands::MapObjectDescr::AttributeIndex>& needed_attributes,
                     std::set<std::string>* deduced_immovables,
                     std::set<std::string>* deduced_bobs) {
	// Protect against endless recursion
	if (walked_immovables->count(index) == 1) {
		return;
	}
	walked_immovables->insert(index);

	// Insert this immovable's attributes
	const Widelands::ImmovableDescr* immovable_descr = descriptions.get_immovable_descr(index);
	for (const Widelands::MapObjectDescr::AttributeIndex id : immovable_descr->attributes()) {
		if (needed_attributes.count(id) == 1) {
			deduced_immovables->insert(immovable_descr->name());
		}
	}

	// Check immovables that this immovable can turn into
	for (const auto& imm_becomes : immovable_descr->becomes()) {
		switch (imm_becomes.first) {
		case Widelands::MapObjectType::BOB:
			deduced_bobs->insert(imm_becomes.second);
			// Bobs don't transform further
			return;
		case Widelands::MapObjectType::IMMOVABLE: {
			const Widelands::DescriptionIndex becomes_index =
			   descriptions.immovable_index(imm_becomes.second);
			assert(becomes_index != Widelands::INVALID_INDEX);
			walk_immovables(becomes_index, descriptions, walked_immovables, needed_attributes,
			                deduced_immovables, deduced_bobs);
		} break;
		default:
			NEVER_HERE();
		}
	}
}

// Read helptext from Lua table
void load_helptexts(Widelands::MapObjectDescr* descr,
                    const LuaTable& table,
                    const std::string& tribe_name) {
	std::map<std::string, std::string> helptexts;
	if (table.has_key("helptexts")) {
		std::unique_ptr<LuaTable> helptext_table = table.get_table("helptexts");
		for (const std::string& category_key : helptext_table->keys<std::string>()) {
			LuaTable::DataType datatype = helptext_table->get_datatype(category_key);
			switch (datatype) {
			case LuaTable::DataType::kString: {
				helptexts[category_key] = helptext_table->get_string(category_key);
			} break;
			case LuaTable::DataType::kTable: {
				// Concatenate text from entries with the localized sentence joiner
				std::unique_ptr<LuaTable> category_table = helptext_table->get_table(category_key);
				std::set<int> helptext_keys = category_table->keys<int>();
				if (!helptext_keys.empty()) {
					auto it = helptext_keys.begin();
					std::string helptext = category_table->get_string(*it);
					++it;
					for (; it != helptext_keys.end(); ++it) {
						helptext = i18n::join_sentences(helptext, category_table->get_string(*it));
					}
					helptexts[category_key] = helptext;
				} else {
					log_warn("Empty helptext defined for '%s'", descr->name().c_str());
				}
			} break;
			default:
				log_warn("Wrong helptext data type for '%s', category '%s'. Expecting a table or a "
				         "string.",
				         descr->name().c_str(), category_key.c_str());
			}
		}
		if (helptexts.empty()) {
			log_warn("No helptext defined for '%s'", descr->name().c_str());
		}
	} else {
		log_warn("No helptext defined for '%s'", descr->name().c_str());
	}
	descr->set_helptexts(tribe_name, helptexts);
}

// NOCOM document
void walk_ware_supply_chain(Widelands::TribeDescr* tribe,
							const std::map<Widelands::ProductionCategory, std::set<Widelands::TribeDescr::ScoredDescriptionIndex>>& productionsite_categories,
                            std::set<Widelands::DescriptionIndex>& walked_productionsites,
                            std::set<Widelands::ProductionProgram::WareWorkerId>& assigned_ware_workers,
                            const std::set<Widelands::WeightedProductionCategory>& categories_to_search,
                            std::map<Widelands::ProductionProgram::WareWorkerId,
                                     std::set<Widelands::WeightedProductionCategory>>* target,
                            unsigned distance) {

	for (const Widelands::ProductionProgram::WareWorkerId& ware_worker : assigned_ware_workers) {
		// Find suitable productionsites
		const std::set<Widelands::DescriptionIndex>& candidates =
		   ware_worker.type == Widelands::WareWorker::wwWARE ?
		      tribe->get_ware_descr(ware_worker.index)->producers() :
		      tribe->get_worker_descr(ware_worker.index)->recruiters();

		for (Widelands::DescriptionIndex producer_index : candidates) {
			const Widelands::ProductionSiteDescr* prod =
			   dynamic_cast<const Widelands::ProductionSiteDescr*>(
			      tribe->get_building_descr(producer_index));

			if (!tribe->has_building(producer_index)) {
				continue;
			}

			// Prevent endless loops
			if (walked_productionsites.count(producer_index) == 1) {
				continue;
			}
			walked_productionsites.insert(producer_index);

			if (prod->production_links().empty()) {
				continue;
			}

			// Collect categories for producing site so we can stop on mismatch. This fixes overgeneration to to circularity by Frisian Recycling Center.
			std::set<Widelands::ProductionCategory> producer_categories;
			for (const auto& prod_cat : productionsite_categories) {
				for (const Widelands::TribeDescr::ScoredDescriptionIndex& prod_cat_item : prod_cat.second) {
					if (prod_cat_item.index == producer_index) {
						producer_categories.insert(prod_cat.first);
						break;
					}
				}
			}

			// Check the production links and add any matches
			for (const Widelands::ProductionSiteDescr::ProductionLink& link :
			     prod->production_links()) {
				// Ware/Worker mismatch, so no need to check further for this link
				if (link.outputs.second != ware_worker.type) {
					continue;
				}
				for (const auto& output_item : *link.outputs.first) {
					// Check if the production site's output wareworker is wanted for this category
					if (assigned_ware_workers.count(Widelands::ProductionProgram::WareWorkerId{
					       output_item.first, link.outputs.second}) == 0) {
						continue;
					}
#ifndef NDEBUG
					// Double-check that the tribe has the ware/worker
					switch (link.outputs.second) {
					case Widelands::WareWorker::wwWARE:
						assert(tribe->has_ware(output_item.first));
						break;
					case Widelands::WareWorker::wwWORKER:
						assert(tribe->has_worker(output_item.first));
						break;
					}
#endif
					// Add inputs to production category
					for (const Widelands::ProductionProgram::WareTypeGroup& input : *link.inputs) {
						for (const Widelands::ProductionProgram::WareWorkerId& input_item : input.first) {

							for (Widelands::WeightedProductionCategory ware_category : categories_to_search) {
								assert(ware_category.category != Widelands::ProductionCategory::kNone);


								// Check if building matches. This gets rid of e.g. some cycles from the recycling centers.
								bool addme = producer_categories.empty() || producer_categories.count(ware_category.category) == 1;
								if (!addme) {
									log_dbg("NOCOM +++ %s blocked category %s", prod->name().c_str(), to_string(ware_category.category).c_str());
								}

								// Check first if it's already there.
								if (addme) {
									for (auto target_it = target->begin(); target_it != target->end(); ++target_it) {
										if (target_it->first == input_item) {
											for (auto check_it = target_it->second.begin(); check_it != target_it->second.end(); ++check_it) {
												const Widelands::WeightedProductionCategory& check_second = *check_it;
												if (check_second.category == ware_category.category) {
													if (check_second.distance > distance) {
														// Ours is better. Kick it out - we'll add it back in with the shorter distance
														target_it->second.erase(check_it);
													} else {
														// Their is better. Keep it and stop processing this wareworker.
														addme = false;
													}
													break;
												}
											}
										}
									}
								}
								if (addme) {
									// All good, now insert
									(*target)[input_item].insert(Widelands::WeightedProductionCategory{
									   ware_category.category, distance});

									const std::string test =
									   input_item.type == Widelands::WareWorker::wwWARE ?
									      tribe->get_ware_descr(input_item.index)->name() :
									      tribe->get_worker_descr(input_item.index)->name();

									const std::string test2 =
									   link.outputs.second == Widelands::WareWorker::wwWARE ?
									      tribe->get_ware_descr(output_item.first)->name() :
									      tribe->get_worker_descr(output_item.first)->name();

									log_dbg("%s: %d %s -> %s -> %s", prod->name().c_str(), distance,
									        test.c_str(), test2.c_str(),
									        to_string(ware_category.category).c_str());

									// Call again with new ware added
									assigned_ware_workers.insert(input_item);
									++distance;
									walk_ware_supply_chain(tribe, productionsite_categories, walked_productionsites, assigned_ware_workers,
									                       categories_to_search, target, distance);
								}
							}
						}
					}
				}
			}
		}
	}
}
}  // namespace

namespace Widelands {
// NOCOM we need detection for waterway supplies and ships
void TribeDescr::process_ware_supply_chain() {
	std::set<DescriptionIndex> walked_productionsites;
	// Take a copy for initialization, because we will be adding to the colntainer
	std::map<ProductionProgram::WareWorkerId, std::set<WeightedProductionCategory>>
	   initial_categories = ware_worker_categories_;
	for (const auto& category_info : initial_categories) {

		assert(!category_info.second.empty());
		std::set<ProductionProgram::WareWorkerId> wares{category_info.first};
		walk_ware_supply_chain(
		   this, productionsite_categories_, walked_productionsites, wares, category_info.second, &ware_worker_categories_, 1);
		walked_productionsites.clear();
	}

	log_dbg("********* Missing *********");
	// Ensure all wares have at least an empty category set to prevent crashes
	for (DescriptionIndex ware_index : wares_) {
		const ProductionProgram::WareWorkerId key{ware_index, WareWorker::wwWARE};
		if (ware_worker_categories_.count(key) != 1) {
			const WareDescr* ware_descr = get_ware_descr(ware_index);
			log_err("NOCOM Ware without category: %d %s", ware_index, ware_descr->name().c_str());
			ware_worker_categories_[key];
		}
	}
	// NOCOM code duplication
	for (DescriptionIndex ware_index : workers_) {
		const ProductionProgram::WareWorkerId key{ware_index, WareWorker::wwWORKER};
		if (ware_worker_categories_.count(key) != 1) {
			ware_worker_categories_[key];
		}
	}

	// NOCOM debug, remove this
	/*
	log_dbg("********* Categories *********");
	for (const auto& ware : ware_worker_categories_) {
	   if (ware.first.type == WareWorker::wwWARE) {
	      const WareDescr* test = get_ware_descr(ware.first.index);
	      for (const WeightedProductionCategory& cat : ware.second) {
	         log_dbg("NOCOM ware: %s -> %d %s", test->name().c_str(), cat.distance,
	to_string(cat.category).c_str());
	      }
	   } else {
	      const WorkerDescr* test = get_worker_descr(ware.first.index);
	      for (const WeightedProductionCategory& cat : ware.second) {
	         log_dbg("NOCOM worker: %s -> %d %s", test->name().c_str(), cat.distance,
	to_string(cat.category).c_str());
	      }
	   }
	} */
}

/**
 * The contents of 'table' are documented in
 * /data/tribes/atlanteans.lua
 */
TribeDescr::TribeDescr(const Widelands::TribeBasicInfo& info,
                       Descriptions& descriptions,
                       const LuaTable& table,
                       const LuaTable* scenario_table)
   : name_(table.get_string("name")),
     descname_(info.descname),
     military_capacity_script_(table.has_key<std::string>("military_capacity_script") ?
                                  table.get_string("military_capacity_script") :
                                  ""),
     descriptions_(descriptions),
     bridge_height_(table.get_int("bridge_height")),
     builder_(Widelands::INVALID_INDEX),
     carrier_(Widelands::INVALID_INDEX),
     carrier2_(Widelands::INVALID_INDEX),
     geologist_(Widelands::INVALID_INDEX),
     soldier_(Widelands::INVALID_INDEX),
     ship_(Widelands::INVALID_INDEX),
     ferry_(Widelands::INVALID_INDEX),
     port_(Widelands::INVALID_INDEX),
     initializations_(info.initializations) {
	log_info("┏━ Loading %s", name_.c_str());
	ScopedTimer timer("┗━ took %ums");

	if (military_capacity_script_.empty() || !g_fs->file_exists(military_capacity_script_)) {
		// TODO(GunChleoc): API compatibility - require after v 1.0
		log_warn("File '%s' for military_capacity_script for tribe '%s' does not exist",
		         military_capacity_script_.c_str(), name().c_str());
	}

	auto set_progress_message = [this](const std::string& str, int i) {
		Notifications::publish(UI::NoteLoadingMessage(
		   /** TRANSLATORS: Example: Loading Barbarians: Buildings (2/6) */
		   (boost::format(_("Loading %1%: %2% (%3%/%4%)")) % descname() % str % i % 6).str()));
	};

	try {
		log_info("┃    Ships");
		set_progress_message(_("Ships"), 1);
		load_ships(table, descriptions);

		log_info("┃    Immovables");
		set_progress_message(_("Immovables"), 2);
		load_immovables(table, descriptions);

		log_info("┃    Wares");
		set_progress_message(_("Wares"), 3);
		load_wares(table, descriptions);
		if (scenario_table != nullptr && scenario_table->has_key("wares_order")) {
			load_wares(*scenario_table, descriptions);
		}

		log_info("┃    Workers");
		set_progress_message(_("Workers"), 4);
		load_workers(table, descriptions);
		if (scenario_table != nullptr && scenario_table->has_key("workers_order")) {
			load_workers(*scenario_table, descriptions);
		}

		log_info("┃    Buildings");
		set_progress_message(_("Buildings"), 5);
		load_buildings(table, descriptions);
		if (scenario_table != nullptr && scenario_table->has_key("buildings")) {
			load_buildings(*scenario_table, descriptions);
		}

		set_progress_message(_("Finishing"), 6);

		log_info("┃    Frontiers, flags and roads");
		load_frontiers_flags_roads(table);

		log_info("┃    Finalizing");
		if (table.has_key<std::string>("toolbar")) {
			toolbar_image_set_.reset(new ToolbarImageset(*table.get_table("toolbar")));
		}
		finalize_loading(descriptions);
	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}
}

void TribeDescr::load_frontiers_flags_roads(const LuaTable& table) {

	std::unique_ptr<LuaTable> items_table = table.get_table("roads");
	const auto load_roads = [&items_table](
	                           const std::string& road_type, std::vector<std::string>* images) {
		images->clear();
		std::vector<std::string> roads =
		   items_table->get_table(road_type)->array_entries<std::string>();
		for (const std::string& filename : roads) {
			if (g_fs->file_exists(filename)) {
				images->push_back(filename);
			} else {
				throw GameDataError(
				   "File '%s' for %s road texture doesn't exist", filename.c_str(), road_type.c_str());
			}
		}
		if (images->empty()) {
			throw GameDataError("Tribe has no %s roads.", road_type.c_str());
		}
	};

	// Add textures for roads/waterways.
	// Note: Road and flag texures are loaded in "graphic/build_texture_atlas.h"
	std::vector<std::string> road_images;

	load_roads("normal", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_normal_road_texture(g_image_cache->get(texture_path));
	}

	load_roads("busy", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_busy_road_texture(g_image_cache->get(texture_path));
	}

	load_roads("waterway", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_waterway_texture(g_image_cache->get(texture_path));
	}

	const auto load_bridge_if_present = [this](const LuaTable& animations_table,
	                                           const std::string& animation_directory,
	                                           Animation::Type animation_type,
	                                           const std::string& s_dir, const std::string& s_type,
	                                           uint32_t* id) {
		const std::string directional_name("bridge_" + s_type + "_" + s_dir);
		if (animations_table.has_key(directional_name)) {
			std::unique_ptr<LuaTable> animation_table = animations_table.get_table(directional_name);
			*id =
			   g_animation_manager->load(name_ + std::string("_") + directional_name, *animation_table,
			                             directional_name, animation_directory, animation_type);
		}
	};
	// Frontier and flag animations can be a mix of file and spritesheet animations
	const auto load_animations = [this, load_bridge_if_present](
	                                const LuaTable& animations_table,
	                                const std::string& animation_directory,
	                                Animation::Type animation_type) {
		if (animations_table.has_key("frontier")) {
			std::unique_ptr<LuaTable> animation_table = animations_table.get_table("frontier");
			frontier_animation_id_ =
			   g_animation_manager->load(name_ + std::string("_frontier"), *animation_table,
			                             "frontier", animation_directory, animation_type);
		}
		if (animations_table.has_key("flag")) {
			std::unique_ptr<LuaTable> animation_table = animations_table.get_table("flag");
			flag_animation_id_ =
			   g_animation_manager->load(name_ + std::string("_flag"), *animation_table, "flag",
			                             animation_directory, animation_type);
		}
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "e", "normal", &bridges_normal_.e);
		load_bridge_if_present(animations_table, animation_directory, animation_type, "se", "normal",
		                       &bridges_normal_.se);
		load_bridge_if_present(animations_table, animation_directory, animation_type, "sw", "normal",
		                       &bridges_normal_.sw);
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "e", "busy", &bridges_busy_.e);
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "se", "busy", &bridges_busy_.se);
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "sw", "busy", &bridges_busy_.sw);
	};

	std::string animation_directory = table.get_string("animation_directory");
	if (table.has_key("animations")) {
		load_animations(*table.get_table("animations"), animation_directory, Animation::Type::kFiles);
	}
	if (table.has_key("spritesheets")) {
		load_animations(
		   *table.get_table("spritesheets"), animation_directory, Animation::Type::kSpritesheet);
	}
}

void TribeDescr::load_ships(const LuaTable& table, Descriptions& descriptions) {
	const std::string shipname(table.get_string("ship"));
	try {
		ship_ = descriptions.load_ship(shipname);
	} catch (const WException& e) {
		throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
	}
}

void TribeDescr::load_wares(const LuaTable& table, Descriptions& descriptions) {
	std::unique_ptr<LuaTable> items_table = table.get_table("wares_order");

	for (const int column_key : items_table->keys<int>()) {
		std::vector<DescriptionIndex> column;
		std::unique_ptr<LuaTable> column_table = items_table->get_table(column_key);
		for (const int ware_key : column_table->keys<int>()) {
			std::unique_ptr<LuaTable> ware_table = column_table->get_table(ware_key);
			std::string ware_name = ware_table->get_string("name");
			try {
				DescriptionIndex wareindex = descriptions.load_ware(ware_name);
				if (has_ware(wareindex)) {
					throw GameDataError("Duplicate definition of ware");
				}

				// log_dbg("NOCOM loaded ware %d %s", wareindex, ware_name.c_str());

				ware_preciousness_.insert(std::make_pair(wareindex, 1.f));

				// Set default_target_quantity (optional) and preciousness
				WareDescr* ware_descr = descriptions.get_mutable_ware_descr(wareindex);
				if (ware_table->has_key("default_target_quantity")) {
					ware_descr->set_default_target_quantity(
					   name(), ware_table->get_int("default_target_quantity"));
				}
				ware_descr->set_preciousness(name(), ware_table->get_int("preciousness"));

				// Add helptexts
				load_helptexts(ware_descr, *ware_table, name());

				// Add to tribe
				wares_.insert(wareindex);
				column.push_back(wareindex);
			} catch (const WException& e) {
				throw GameDataError("Failed adding ware '%s': %s", ware_name.c_str(), e.what());
			}
		}
		if (!column.empty()) {
			wares_order_.push_back(column);
		}
	}
}

void TribeDescr::load_immovables(const LuaTable& table, Descriptions& descriptions) {
	for (const auto& immovable_table :
	     table.get_table("immovables")->array_entries<std::unique_ptr<LuaTable>>()) {
		const std::string immovablename(immovable_table->get_string("name"));
		try {
			DescriptionIndex index = descriptions.load_immovable(immovablename);
			if (immovables_.count(index) == 1) {
				throw GameDataError("Duplicate definition of immovable '%s'", immovablename.c_str());
			}
			immovables_.insert(index);
			// Add helptext
			load_helptexts(descriptions.get_mutable_immovable_descr(index), *immovable_table, name());

		} catch (const WException& e) {
			throw GameDataError("Failed adding immovable '%s': %s", immovablename.c_str(), e.what());
		}
	}

	std::unique_ptr<LuaTable> items_table = table.get_table("resource_indicators");
	for (const std::string& resource : items_table->keys<std::string>()) {
		ResourceIndicatorList resis;
		std::unique_ptr<LuaTable> tbl = items_table->get_table(resource);
		const std::set<int> keys = tbl->keys<int>();
		for (int upper_limit : keys) {
			resis[upper_limit] = descriptions.load_immovable(tbl->get_string(upper_limit));
		}
		if (resis.empty()) {
			throw GameDataError("Tribe has no indicators for resource %s.", resource.c_str());
		}
		resource_indicators_[resource] = resis;
	}
}

void TribeDescr::load_workers(const LuaTable& table, Descriptions& descriptions) {
	std::unique_ptr<LuaTable> items_table = table.get_table("workers_order");

	for (const int column_key : items_table->keys<int>()) {
		std::vector<DescriptionIndex> column;
		std::unique_ptr<LuaTable> column_table = items_table->get_table(column_key);
		for (const int worker_key : column_table->keys<int>()) {
			std::unique_ptr<LuaTable> worker_table = column_table->get_table(worker_key);
			std::string workername = worker_table->get_string("name");
			try {
				DescriptionIndex workerindex = descriptions.load_worker(workername);
				if (has_worker(workerindex)) {
					throw GameDataError("Duplicate definition of worker");
				}

				// log_dbg("NOCOM loaded worker %d %s", workerindex, workername.c_str());

				// Set default_target_quantity and preciousness (both optional)
				WorkerDescr* worker_descr = descriptions.get_mutable_worker_descr(workerindex);
				if (worker_table->has_key("default_target_quantity")) {
					if (!worker_table->has_key("preciousness")) {
						throw GameDataError(
						   "It has a default_target_quantity but no preciousness for tribe '%s'",
						   name().c_str());
					}
					worker_descr->set_default_target_quantity(
					   worker_table->get_int("default_target_quantity"));
				}
				if (worker_table->has_key("preciousness")) {
					worker_descr->set_preciousness(name(), worker_table->get_int("preciousness"));
				}

				// Add helptexts
				load_helptexts(worker_descr, *worker_table, name());

				// Add to tribe
				workers_.insert(workerindex);
				column.push_back(workerindex);

				if (worker_descr->is_buildable() && worker_descr->buildcost().empty()) {
					worker_types_without_cost_.push_back(workerindex);
				}
			} catch (const WException& e) {
				throw GameDataError("Failed adding worker '%s': %s", workername.c_str(), e.what());
			}
		}
		if (!column.empty()) {
			workers_order_.push_back(column);
		}
	}

	if (table.has_key("builder")) {
		builder_ = add_special_worker(
		   table.get_string("builder"), ProductionCategory::kConstruction, descriptions);
		// log_dbg("NOCOM add special construction worker %d", builder_);
	}
	if (table.has_key("carrier")) {
		carrier_ =
		   add_special_worker(table.get_string("carrier"), ProductionCategory::kRoads, descriptions);
		// log_dbg("NOCOM add special road worker %d", carrier_);
	}
	if (table.has_key("carrier2")) {
		carrier2_ =
		   add_special_worker(table.get_string("carrier2"), ProductionCategory::kRoads, descriptions);
		// log_dbg("NOCOM add special road worker %d", carrier2_);
	}
	if (table.has_key("geologist")) {
		geologist_ = add_special_worker(
		   table.get_string("geologist"), ProductionCategory::kMining, descriptions);
		// log_dbg("NOCOM add special mining worker %d", geologist_);
	}
	if (table.has_key("soldier")) {
		soldier_ = add_special_worker(
		   table.get_string("soldier"), ProductionCategory::kTraining, descriptions);
		// log_dbg("NOCOM add special training worker %d", soldier_);
	}
	if (table.has_key("ferry")) {
		ferry_ = add_special_worker(
		   table.get_string("ferry"), ProductionCategory::kWaterways, descriptions);
		// log_dbg("NOCOM add special waterway worker %d", ferry_);
	}
}

void TribeDescr::load_buildings(const LuaTable& table, Descriptions& descriptions) {
	for (const auto& building_table :
	     table.get_table("buildings")->array_entries<std::unique_ptr<LuaTable>>()) {
		const std::string buildingname(building_table->get_string("name"));
		try {
			DescriptionIndex index = descriptions.load_building(buildingname);
			if (has_building(index)) {
				throw GameDataError("Duplicate definition of building '%s'", buildingname.c_str());
			}
			buildings_.insert(index);

			BuildingDescr* building_descr = descriptions.get_mutable_building_descr(index);

			// Add helptexts
			load_helptexts(building_descr, *building_table, name());

			// Register at enhanced building
			const DescriptionIndex& enhancement = building_descr->enhancement();
			if (enhancement != INVALID_INDEX) {
				descriptions.get_mutable_building_descr(enhancement)->set_enhanced_from(index);
			}

			// Register trainigsites & production categories
			switch (building_descr->type()) {
			case MapObjectType::TRAININGSITE:
				trainingsites_.push_back(index);
				building_ui_categories_[ProductionUICategory::kTraining].insert(index);
				break;
			case MapObjectType::MILITARYSITE:
				building_ui_categories_[ProductionUICategory::kMilitary].insert(index);
				break;
			case MapObjectType::MARKET:
			case MapObjectType::WAREHOUSE:
				building_ui_categories_[ProductionUICategory::kTransport].insert(index);
				break;
			default:
				;
			}

			// Register construction materials
			for (const auto& build_cost : building_descr->buildcost()) {
				if (!is_construction_material(build_cost.first)) {
					construction_materials_.insert(build_cost.first);
				}
			}
			for (const auto& enhancement_cost : building_descr->enhancement_cost()) {
				if (!is_construction_material(enhancement_cost.first)) {
					construction_materials_.insert(enhancement_cost.first);
				}
			}
		} catch (const WException& e) {
			throw GameDataError("Failed adding building '%s': %s", buildingname.c_str(), e.what());
		}
	}

	if (table.has_key("port")) {
		port_ = add_special_building(table.get_string("port"), descriptions);
	}
}

/**
 * Access functions
 */

const std::string& TribeDescr::name() const {
	return name_;
}
const std::string& TribeDescr::descname() const {
	return descname_;
}
const std::string& TribeDescr::military_capacity_script() const {
	return military_capacity_script_;
}

size_t TribeDescr::get_nrwares() const {
	return wares_.size();
}
size_t TribeDescr::get_nrworkers() const {
	return workers_.size();
}

const std::set<DescriptionIndex>& TribeDescr::buildings() const {
	return buildings_;
}
const std::set<DescriptionIndex>& TribeDescr::wares() const {
	return wares_;
}
const std::set<DescriptionIndex>& TribeDescr::workers() const {
	return workers_;
}
const std::set<DescriptionIndex>& TribeDescr::immovables() const {
	return immovables_;
}
const ResourceIndicatorSet& TribeDescr::resource_indicators() const {
	return resource_indicators_;
}

bool TribeDescr::has_building(const DescriptionIndex& index) const {
	return std::find(buildings_.begin(), buildings_.end(), index) != buildings_.end();
}
bool TribeDescr::has_ware(const DescriptionIndex& index) const {
	return wares_.count(index) == 1;
}
bool TribeDescr::has_worker(const DescriptionIndex& index) const {
	return workers_.count(index) == 1;
}
bool TribeDescr::has_immovable(const DescriptionIndex& index) const {
	return immovables_.count(index) == 1;
}
bool TribeDescr::is_construction_material(const DescriptionIndex& index) const {
	return construction_materials_.count(index) == 1;
}

DescriptionIndex TribeDescr::building_index(const std::string& buildingname) const {
	return descriptions_.building_index(buildingname);
}

DescriptionIndex TribeDescr::immovable_index(const std::string& immovablename) const {
	return descriptions_.immovable_index(immovablename);
}
DescriptionIndex TribeDescr::ware_index(const std::string& warename) const {
	return descriptions_.ware_index(warename);
}
DescriptionIndex TribeDescr::worker_index(const std::string& workername) const {
	return descriptions_.worker_index(workername);
}

DescriptionIndex TribeDescr::safe_building_index(const std::string& buildingname) const {
	return descriptions_.safe_building_index(buildingname);
}

DescriptionIndex TribeDescr::safe_ware_index(const std::string& warename) const {
	return descriptions_.safe_ware_index(warename);
}
DescriptionIndex TribeDescr::safe_worker_index(const std::string& workername) const {
	return descriptions_.safe_worker_index(workername);
}

WareDescr const* TribeDescr::get_ware_descr(const DescriptionIndex& index) const {
	return descriptions_.get_ware_descr(index);
}
WorkerDescr const* TribeDescr::get_worker_descr(const DescriptionIndex& index) const {
	return descriptions_.get_worker_descr(index);
}

BuildingDescr const* TribeDescr::get_building_descr(const DescriptionIndex& index) const {
	return descriptions_.get_building_descr(index);
}
ImmovableDescr const* TribeDescr::get_immovable_descr(const DescriptionIndex& index) const {
	return descriptions_.get_immovable_descr(index);
}

DescriptionIndex TribeDescr::builder() const {
	assert(descriptions_.worker_exists(builder_));
	return builder_;
}
DescriptionIndex TribeDescr::carrier() const {
	assert(descriptions_.worker_exists(carrier_));
	return carrier_;
}
DescriptionIndex TribeDescr::carrier2() const {
	assert(descriptions_.worker_exists(carrier2_));
	return carrier2_;
}
DescriptionIndex TribeDescr::geologist() const {
	assert(descriptions_.worker_exists(geologist_));
	return geologist_;
}
DescriptionIndex TribeDescr::soldier() const {
	assert(descriptions_.worker_exists(soldier_));
	return soldier_;
}
DescriptionIndex TribeDescr::ship() const {
	assert(descriptions_.ship_exists(ship_));
	return ship_;
}
DescriptionIndex TribeDescr::port() const {
	assert(descriptions_.building_exists(port_));
	return port_;
}
DescriptionIndex TribeDescr::ferry() const {
	assert(descriptions_.worker_exists(ferry_));
	return ferry_;
}

const std::vector<DescriptionIndex>& TribeDescr::trainingsites() const {
	return trainingsites_;
}
const std::vector<DescriptionIndex>& TribeDescr::worker_types_without_cost() const {
	return worker_types_without_cost_;
}

uint32_t TribeDescr::frontier_animation() const {
	return frontier_animation_id_;
}

uint32_t TribeDescr::flag_animation() const {
	return flag_animation_id_;
}

uint32_t TribeDescr::bridge_animation(uint8_t dir, bool busy) const {
	switch (dir) {
	case WALK_E:
		return (busy ? bridges_busy_ : bridges_normal_).e;
	case WALK_SE:
		return (busy ? bridges_busy_ : bridges_normal_).se;
	case WALK_SW:
		return (busy ? bridges_busy_ : bridges_normal_).sw;
	default:
		NEVER_HERE();
	}
}

uint32_t TribeDescr::bridge_height() const {
	return bridge_height_;
}

const RoadTextures& TribeDescr::road_textures() const {
	return road_textures_;
}

/*
==============
Find the best matching indicator for the given amount.
==============
*/
DescriptionIndex TribeDescr::get_resource_indicator(ResourceDescription const* const res,
                                                    const ResourceAmount amount) const {
	if (!res || !amount) {
		auto list = resource_indicators_.find("");
		if (list == resource_indicators_.end() || list->second.empty()) {
			throw GameDataError("Tribe '%s' has no indicator for no resources!", name_.c_str());
		}
		return list->second.begin()->second;
	}

	auto list = resource_indicators_.find(res->name());
	if (list == resource_indicators_.end() || list->second.empty()) {
		throw GameDataError(
		   "Tribe '%s' has no indicators for resource '%s'!", name_.c_str(), res->name().c_str());
	}

	uint32_t lowest = 0;
	for (const auto& resi : list->second) {
		if (resi.first < amount) {
			continue;
		} else if (lowest < amount || resi.first < lowest) {
			lowest = resi.first;
		}
	}

	if (lowest < amount) {
		throw GameDataError("Tribe '%s' has no indicators for amount %i of resource '%s' (highest "
		                    "possible amount is %i)!",
		                    name_.c_str(), amount, res->name().c_str(), lowest);
	}

	return list->second.find(lowest)->second;
}

float TribeDescr::ware_preciousness(DescriptionIndex ware_index) const {
	assert(ware_preciousness_.count(ware_index) == 1);
	return ware_preciousness_.at(ware_index);
}

const std::set<WeightedProductionCategory>&
TribeDescr::ware_worker_categories(DescriptionIndex index, WareWorker type) const {
	const ProductionProgram::WareWorkerId key{index, type};
	assert(ware_worker_categories_.count(key) == 1);
	return ware_worker_categories_.at(key);
}

const std::map<ProductionCategory, std::set<TribeDescr::ScoredDescriptionIndex>>&
TribeDescr::productionsite_categories() const {
	return productionsite_categories_;
}
const std::map<ProductionUICategory, std::set<DescriptionIndex>>& TribeDescr::building_ui_categories() const {
	return building_ui_categories_;
}

ToolbarImageset* TribeDescr::toolbar_image_set() const {
	return toolbar_image_set_.get();
}

/**
 * Helper functions
 */

DescriptionIndex TribeDescr::add_special_worker(const std::string& workername,
                                                ProductionCategory category,
                                                Descriptions& descriptions) {
	try {
		DescriptionIndex worker = descriptions.load_worker(workername);
		if (!has_worker(worker)) {
			throw GameDataError("This tribe doesn't have the worker '%s'", workername.c_str());
		}

		ware_worker_categories_[ProductionProgram::WareWorkerId{worker, WareWorker::wwWORKER}].insert(
		   WeightedProductionCategory{category, 0});

		return worker;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special worker '%s': %s", workername.c_str(), e.what());
	}
}

DescriptionIndex TribeDescr::add_special_building(const std::string& buildingname,
                                                  Descriptions& descriptions) {
	try {
		DescriptionIndex building = descriptions.load_building(buildingname);
		if (!has_building(building)) {
			throw GameDataError("This tribe doesn't have the building '%s'", buildingname.c_str());
		}
		return building;
	} catch (const WException& e) {
		throw GameDataError(
		   "Failed adding special building '%s': %s", buildingname.c_str(), e.what());
	}
}

void TribeDescr::finalize_loading(Descriptions& descriptions) {
	// Validate special units
	if (builder_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'builder' not defined");
	}
	if (carrier_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'carrier' not defined");
	}
	if (carrier2_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'carrier2' not defined");
	}
	if (geologist_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'geologist' not defined");
	}
	if (soldier_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'soldier' not defined");
	}
	if (ferry_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'ferry' not defined");
	}
	if (port_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special building 'port' not defined");
	}
	if (ship_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special unit 'ship' not defined");
	}

	calculate_trainingsites_proportions(descriptions);
	process_productionsites(descriptions);
}

// Set default trainingsites proportions for AI. Make sure that we get a sum of ca. 100
void TribeDescr::calculate_trainingsites_proportions(const Descriptions& descriptions) {
	unsigned int trainingsites_without_percent = 0;
	int used_percent = 0;
	std::vector<BuildingDescr*> traingsites_with_percent;
	for (const DescriptionIndex& index : trainingsites()) {
		BuildingDescr* descr = descriptions.get_mutable_building_descr(index);
		if (descr->hints().trainingsites_max_percent() == 0) {
			++trainingsites_without_percent;
		} else {
			used_percent += descr->hints().trainingsites_max_percent();
			traingsites_with_percent.push_back(descr);
		}
	}

	// Adjust used_percent if we don't have at least 5% for each remaining trainingsite
	const float limit = 100 - trainingsites_without_percent * 5;
	if (used_percent > limit) {
		const int deductme = (used_percent - limit) / traingsites_with_percent.size();
		used_percent = 0;
		for (BuildingDescr* descr : traingsites_with_percent) {
			descr->set_hints_trainingsites_max_percent(descr->hints().trainingsites_max_percent() -
			                                           deductme);
			used_percent += descr->hints().trainingsites_max_percent();
		}
	}

	// Now adjust for trainingsites that didn't have their max_percent set
	if (trainingsites_without_percent > 0) {
		int percent_to_use = std::ceil((100 - used_percent) / trainingsites_without_percent);
		// We sometimes get below 100% in spite of the ceil call above.
		// A total sum a bit above 100% is fine though, so we increment until it's big enough.
		while ((used_percent + percent_to_use * trainingsites_without_percent) < 100) {
			++percent_to_use;
		}
		if (percent_to_use < 1) {
			throw GameDataError(
			   "%s: Training sites without predefined proportions add up to < 1%% and "
			   "will never be built: %d",
			   name().c_str(), used_percent);
		}
		for (const DescriptionIndex& index : trainingsites()) {
			BuildingDescr* descr = descriptions.get_mutable_building_descr(index);
			if (descr->hints().trainingsites_max_percent() == 0) {
				descr->set_hints_trainingsites_max_percent(percent_to_use);
				used_percent += percent_to_use;
			}
		}
	}
	if (used_percent < 100) {
		throw GameDataError("%s: Final training sites proportions add up to < 100%%: %d",
		                    name().c_str(), used_percent);
	}
}

// Calculate building properties that have circular dependencies
void TribeDescr::process_productionsites(Descriptions& descriptions) {

	// NOCOM construction_materials_ is redundant now!
	for (DescriptionIndex ware_idx : construction_materials_) {
		// log_dbg("NOCOM add construction ware %d", ware_idx);
		ware_worker_categories_[ProductionProgram::WareWorkerId{ware_idx, WareWorker::wwWARE}].insert(
		   WeightedProductionCategory{ProductionCategory::kConstruction, 0});
	}

	for (DescriptionIndex worker_index : workers_) {
		const WorkerDescr* worker = get_worker_descr(worker_index);
		if (worker->is_buildable()) {
			for (const auto& buildcost : worker->buildcost()) {
				const DescriptionIndex ware_idx = ware_index(buildcost.first);
				if (has_ware(ware_idx)) {
					// log_dbg("NOCOM add tool ware %d for worker %s", ware_idx, worker->name().c_str());
					ware_worker_categories_[ProductionProgram::WareWorkerId{
					                           ware_idx, WareWorker::wwWARE}]
					   .insert(WeightedProductionCategory{ProductionCategory::kTool, 0});
				}
			}
		}
	}

	// Get a list of productionsites - we will need to iterate them more than once
	// The temporary use of pointers here is fine, because it doesn't affect the game state.
	std::set<ProductionSiteDescr*> productionsites;
	std::set<DescriptionIndex> categorized_productionsites;

	// Iterate buildings and update circular dependencies
	for (const DescriptionIndex index : buildings()) {
		BuildingDescr* building = descriptions_.get_mutable_building_descr(index);
		assert(building != nullptr);

		// Calculate largest possible workarea radius
		for (const auto& pair : building->workarea_info()) {
			descriptions.increase_largest_workarea(pair.first);
		}

		if (building->get_built_over_immovable() != INVALID_INDEX) {
			buildings_built_over_immovables_.insert(building);
		}

		ProductionSiteDescr* productionsite = dynamic_cast<ProductionSiteDescr*>(building);
		if (productionsite != nullptr) {
			// List productionsite for use below
			productionsites.insert(productionsite);

			// Add consumers and producers to wares.
			for (const WareAmount& ware_amount : productionsite->input_wares()) {
				assert(has_ware(ware_amount.first));
				descriptions.get_mutable_ware_descr(ware_amount.first)->add_consumer(index);
			}
			for (const DescriptionIndex& wareindex : productionsite->output_ware_types()) {
				assert(has_ware(wareindex));
				descriptions.get_mutable_ware_descr(wareindex)->add_producer(index);
			}
			for (const auto& job : productionsite->working_positions()) {
				assert(has_worker(job.first));
				descriptions.get_mutable_worker_descr(job.first)->add_employer(index);
			}
			for (const DescriptionIndex& workerindex : productionsite->output_worker_types()) {
				descriptions.get_mutable_worker_descr(workerindex)->add_recruiter(index);
			}
			// Resource info
			for (const auto& r : productionsite->collected_resources()) {
				used_resources_.insert(r.first);
			}
			for (const std::string& r : productionsite->created_resources()) {
				used_resources_.insert(r);
			}
		}
	}

	// Now that we have gathered all resources we can use, verify the resource indicators
	for (DescriptionIndex resource_index = 0; resource_index < descriptions.nr_resources();
	     resource_index++) {
		const ResourceDescription* res = descriptions.get_resource_descr(resource_index);
		if (res->detectable() && uses_resource(res->name())) {
			// This function will throw an exception if this tribe doesn't
			// have a high enough resource indicator for this resource
			get_resource_indicator(res, res->max_amount());
		}
	}
	// For the "none" indicator
	get_resource_indicator(nullptr, 0);

	const DescriptionMaintainer<ImmovableDescr>& all_immovables = descriptions.immovables();

	ImmovableProgram::postload_immovable_relations(descriptions);

	// Find all attributes that we need to collect from map
	std::set<MapObjectDescr::AttributeIndex> needed_attributes;
	for (ProductionSiteDescr* prod : productionsites) {
		for (const auto& attribinfo : prod->collected_attributes()) {
			const MapObjectType mapobjecttype = attribinfo.first;
			const MapObjectDescr::AttributeIndex attribute_id = attribinfo.second;
			needed_attributes.insert(attribute_id);

			// Add collected entities
			switch (mapobjecttype) {
			case MapObjectType::IMMOVABLE: {
				for (DescriptionIndex i = 0; i < all_immovables.size(); ++i) {
					const ImmovableDescr& immovable_descr = all_immovables.get(i);
					if (immovable_descr.has_attribute(attribute_id)) {
						prod->add_collected_immovable(immovable_descr.name());
						const_cast<ImmovableDescr&>(immovable_descr)
						   .add_collected_by(descriptions, prod->name());
					}
				}
			} break;
			case MapObjectType::BOB: {
				// We only support critters here, because no other bobs are collected so far
				for (DescriptionIndex i = 0; i < descriptions.nr_critters(); ++i) {
					const CritterDescr* critter = descriptions.get_critter_descr(i);
					if (critter->has_attribute(attribute_id)) {
						prod->add_collected_bob(critter->name());
					}
				}
			} break;
			default:
				NEVER_HERE();
			}
		}
	}

	// Register who creates which entities
	std::map<std::string, std::set<ProductionSiteDescr*>> creators;
	auto add_creator = [&creators](const std::string& item, ProductionSiteDescr* productionsite) {
		if (creators.count(item) != 1) {
			creators[item] = {productionsite};
		} else {
			creators[item].insert(productionsite);
		}
	};

	// Register who collects which entities
	std::map<std::string, std::set<ProductionSiteDescr*>> collectors;
	auto add_collector = [&collectors](
	                        const std::string& item, ProductionSiteDescr* productionsite) {
		if (collectors.count(item) != 1) {
			collectors[item] = {productionsite};
		} else {
			collectors[item].insert(productionsite);
		}
	};

	for (ProductionSiteDescr* prod : productionsites) {
		// Add bobs that are created directly
		for (const std::string& bobname : prod->created_bobs()) {
			const CritterDescr* critter = descriptions.get_critter_descr(bobname);
			if (critter == nullptr) {
				if (worker_index(bobname) == Widelands::INVALID_INDEX) {
					throw GameDataError(
					   "Productionsite '%s' has unknown bob '%s' in production or worker program",
					   prod->name().c_str(), bobname.c_str());
				}
			}
			add_creator(bobname, prod);
		}

		// Get attributes and bobs from transformations
		std::set<std::string> deduced_bobs;
		std::set<std::string> deduced_immovables;
		// Remember where we walked in case of circular dependencies
		std::set<DescriptionIndex> walked_immovables;

		for (const auto& attribinfo : prod->created_attributes()) {
			const MapObjectType mapobjecttype = attribinfo.first;
			const MapObjectDescr::AttributeIndex attribute_id = attribinfo.second;
			if (mapobjecttype != MapObjectType::IMMOVABLE) {
				continue;
			}

			for (DescriptionIndex i = 0; i < all_immovables.size(); ++i) {
				const ImmovableDescr& immovable_descr = *get_immovable_descr(i);
				if (immovable_descr.has_attribute(attribute_id)) {
					walk_immovables(i, descriptions, &walked_immovables, needed_attributes,
					                &deduced_immovables, &deduced_bobs);
					if (needed_attributes.count(attribute_id) == 1) {
						prod->add_created_immovable(immovable_descr.name());
						add_creator(immovable_descr.name(), prod);
					}
				}
			}
		}
		// We're done with this site's attributes, let's get some memory back
		prod->clear_attributes();

		// Add deduced bobs & immovables
		for (const std::string& bob_name : deduced_bobs) {
			prod->add_created_bob(bob_name);
			add_creator(bob_name, prod);
		}
		for (const std::string& immovable_name : deduced_immovables) {
			prod->add_created_immovable(immovable_name);
			add_creator(immovable_name, prod);
		}

		// Register remaining creators and collectors
		for (const std::string& resource : prod->created_resources()) {
			add_creator(resource, prod);
		}
		for (const auto& resource : prod->collected_resources()) {
			add_collector(resource.first, prod);
		}
		for (const std::string& bob : prod->collected_bobs()) {
			add_collector(bob, prod);
		}
		for (const std::string& immovable : prod->collected_immovables()) {
			add_collector(immovable, prod);
		}

		// Add ware input categories
		if (prod->get_ismine()) {

			for (const WareAmount& input : prod->input_wares()) {
				// log_dbg("NOCOM add mining ware %d", input.first);
				ware_worker_categories_[ProductionProgram::WareWorkerId{
				                           input.first, WareWorker::wwWARE}]
				   .insert(WeightedProductionCategory{ProductionCategory::kMining, 0});
			}
			for (const auto& input : prod->input_workers()) {
				// log_dbg("NOCOM add mining worker %d", input.first);
				ware_worker_categories_[ProductionProgram::WareWorkerId{
				                           input.first, WareWorker::wwWORKER}]
				   .insert(WeightedProductionCategory{ProductionCategory::kMining, 0});
			}
		} else if (prod->type() == MapObjectType::TRAININGSITE) {
			for (const WareAmount& input : prod->input_wares()) {
				// log_dbg("NOCOM add training ware %d", input.first);
				ware_worker_categories_[ProductionProgram::WareWorkerId{
				                           input.first, WareWorker::wwWARE}]
				   .insert(WeightedProductionCategory{ProductionCategory::kTraining, 0});
			}
			for (const auto& input : prod->input_workers()) {
				// log_dbg("NOCOM add training worker %d", input.first);
				ware_worker_categories_[ProductionProgram::WareWorkerId{
				                           input.first, WareWorker::wwWORKER}]
				   .insert(WeightedProductionCategory{ProductionCategory::kTraining, 0});
			}
			const DescriptionIndex prod_index = building_index(prod->name());
			productionsite_categories_[ProductionCategory::kTraining].insert({prod_index, 0});
			categorized_productionsites.insert(prod_index);
		} else {
			for (const std::string& bobname : prod->created_bobs()) {
				if (descriptions.ship_index(bobname) == ship_) {
					for (const WareAmount& input : prod->input_wares()) {
						ware_worker_categories_[ProductionProgram::WareWorkerId{
						                           input.first, WareWorker::wwWARE}]
						   .insert(WeightedProductionCategory{ProductionCategory::kSeafaring, 0});
					}
					for (const WareAmount& input : prod->input_workers()) {
						ware_worker_categories_[ProductionProgram::WareWorkerId{
						                           input.first, WareWorker::wwWORKER}]
						   .insert(WeightedProductionCategory{ProductionCategory::kSeafaring, 0});
					}
					const DescriptionIndex prod_index = building_index(prod->name());
					productionsite_categories_[ProductionCategory::kSeafaring].insert({prod_index, 0});
					categorized_productionsites.insert(prod_index);
				} else if (descriptions.worker_index(bobname) == ferry_) {
					for (const WareAmount& input : prod->input_wares()) {
						ware_worker_categories_[ProductionProgram::WareWorkerId{
						                           input.first, WareWorker::wwWARE}]
						   .insert(WeightedProductionCategory{ProductionCategory::kWaterways, 0});
					}
					for (const WareAmount& input : prod->input_workers()) {
						ware_worker_categories_[ProductionProgram::WareWorkerId{
						                           input.first, WareWorker::wwWORKER}]
						   .insert(WeightedProductionCategory{ProductionCategory::kWaterways, 0});
					}
					const DescriptionIndex prod_index = building_index(prod->name());
					productionsite_categories_[ProductionCategory::kWaterways].insert({prod_index, 0});
					categorized_productionsites.insert(prod_index);
				}
				add_creator(bobname, prod);
			}
		}
	}

	// Calculate ware supply chain
	process_ware_supply_chain();

	log_dbg("********* Buildings *********");

	std::map<DescriptionIndex, ProductionSiteDescr*> uncategorized_productionsites;

	// Calculate workarea overlaps + AI info
	for (ProductionSiteDescr* prod : productionsites) {
		const DescriptionIndex prod_index = building_index(prod->name());

		std::map<ProductionCategory, unsigned> squashed_categories;
		if (categorized_productionsites.count(prod_index) == 0) {
			for (DescriptionIndex output : prod->output_ware_types()) {
				for (const WeightedProductionCategory& cat :
				     ware_worker_categories(output, WareWorker::wwWARE)) {
					auto category_it = squashed_categories.find(cat.category);
					if (category_it != squashed_categories.end()) {
						category_it->second = std::min(category_it->second, cat.distance);
					} else {
						squashed_categories.insert(std::make_pair(cat.category, cat.distance));
					}
				}
			}
			for (DescriptionIndex output : prod->output_worker_types()) {
				for (const WeightedProductionCategory& cat :
				     ware_worker_categories(output, WareWorker::wwWORKER)) {
					auto category_it = squashed_categories.find(cat.category);
					if (category_it != squashed_categories.end()) {
						category_it->second = std::min(category_it->second, cat.distance);
					} else {
						squashed_categories.insert(std::make_pair(cat.category, cat.distance));
					}
				}
			}

			if (squashed_categories.empty()) {
				// No ware in- or outputs on its own. Use the supported/supporting relationship below to
				// categorize.
				uncategorized_productionsites.insert(std::make_pair(prod_index, prod));
			} else {
				log_dbg("-");
				for (const auto& squashed : squashed_categories) {
					log_dbg("\t%s\t%d\t%s", prod->name().c_str(), squashed.second,
					        to_string(squashed.first).c_str());
					productionsite_categories_[squashed.first].insert({prod_index, squashed.second});
				}
			}
		}

		// Sites that create any immovables should not overlap each other
		if (!prod->created_immovables().empty()) {
			for (const ProductionSiteDescr* other_prod : productionsites) {
				if (!other_prod->created_immovables().empty()) {
					prod->add_competing_productionsite(other_prod->name());
				}
			}
		}
		// Sites that create any resources should not overlap each other
		if (!prod->created_resources().empty()) {
			for (const ProductionSiteDescr* other_prod : productionsites) {
				if (!other_prod->created_resources().empty()) {
					prod->add_competing_productionsite(other_prod->name());
				}
			}
		}

		// Sites that create a bob should not overlap sites that create the same bob
		for (const std::string& item : prod->created_bobs()) {
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					prod->add_competing_productionsite(creator->name());
					creator->add_competing_productionsite(prod->name());
				}
			}
		}

		for (const std::string& item : prod->collected_immovables()) {
			// Sites that collect immovables and sites of other types that create immovables for them
			// should overlap each other
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect immovables should not overlap sites that collect the same immovable
			if (collectors.count(item)) {
				for (const ProductionSiteDescr* collector : collectors.at(item)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
		for (const std::string& item : prod->collected_bobs()) {
			// Sites that collect bobs and sites of other types that create bobs for them should
			// overlap each other
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect bobs should not overlap sites that collect the same bob
			if (collectors.count(item)) {
				for (const ProductionSiteDescr* collector : collectors.at(item)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
		for (const auto& item : prod->collected_resources()) {
			// Sites that collect resources and sites of other types that create resources for them
			// should overlap each other
			if (creators.count(item.first)) {
				for (ProductionSiteDescr* creator : creators.at(item.first)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect resources should not overlap sites that collect the same resource
			if (collectors.count(item.first)) {
				for (const ProductionSiteDescr* collector : collectors.at(item.first)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
	}

	// NOCOM document
	for (const auto& uncategorized : uncategorized_productionsites) {
		std::map<ProductionCategory, unsigned> squashed_categories;
		for (const std::string& supported_name : uncategorized.second->supported_productionsites()) {
			const DescriptionIndex supported_index = building_index(supported_name);
			for (const auto& category : productionsite_categories_) {
				for (const ScoredDescriptionIndex& candidate : category.second) {
					if (candidate.index == supported_index) {
						auto category_it = squashed_categories.find(category.first);
						if (category_it != squashed_categories.end()) {
							category_it->second = std::min(category_it->second, candidate.score);
						} else {
							squashed_categories.insert(std::make_pair(category.first, candidate.score));
						}
					}
				}
			}
		}
		if (squashed_categories.empty()) {
			// Remaining buildings (e.g. scouts) are uncategorized
			log_dbg("NOCOM ############# Missing: %s", uncategorized.second->name().c_str());
			productionsite_categories_[ProductionCategory::kNone].insert({uncategorized.first, 0});
		} else {
			log_dbg("-");
			for (const auto& squashed : squashed_categories) {
				log_dbg("\t%s\t%d\t%s", uncategorized.second->name().c_str(), squashed.second + 1,
						to_string(squashed.first).c_str());
				productionsite_categories_[squashed.first].insert({uncategorized.first, squashed.second + 1});
			}
		}
	}

	// NOCOM document special handling for mines

	for (const auto& category : productionsite_categories_) {
		ProductionUICategory ui_category;
		switch (category.first) {
		case ProductionCategory::kNone:
			ui_category = ProductionUICategory::kNone;
			break;
		case ProductionCategory::kConstruction:
			ui_category = ProductionUICategory::kConstruction;
			break;
		case ProductionCategory::kTool:
			ui_category = ProductionUICategory::kTools;
			break;
		case ProductionCategory::kTraining:
			ui_category = ProductionUICategory::kTraining;
			break;
		case ProductionCategory::kMining:
			// Skip; this is for potential AI use
			continue;
		case ProductionCategory::kRoads:
		case ProductionCategory::kSeafaring:
		case ProductionCategory::kWaterways:
			ui_category = ProductionUICategory::kTransport;
		}
		for (const ScoredDescriptionIndex& item : category.second) {
			building_ui_categories_[ui_category].insert(item.index);
		}
	}


	// NOCOM Calculate ware preciousness
	std::map<DescriptionIndex, const ProductionSiteDescr*> idx2prod;
	for (const ProductionSiteDescr* prod : productionsites) {
		idx2prod.insert((std::make_pair(building_index(prod->name()), prod)));
	}

	auto preciousness_at_building = [this](const ProductionSiteDescr* prodsite, std::set<ProductionProgram::WareWorkerId>* wares) {
		float result = 1.f;
		for (const ProductionSiteDescr::ProductionLink& link :
			 prodsite->production_links()) {
			bool use_this_link = false;
			for (const ProductionProgram::WareTypeGroup& group : *link.inputs) {
				for (const ProductionProgram::WareWorkerId& ware: *wares) {
					if (group.first.count(ware)) {
						use_this_link = true;
						break;
					}
				}
				if (use_this_link) {
					break;
				}
			}
			if (!use_this_link) {
				continue;
			}

			float overall_count = 0.f;
			float input = 1.f;
			for (const ProductionProgram::WareTypeGroup& group : *link.inputs) {
				overall_count += group.second;
				// More precious for bigger amount needed, less precious for alternative wares
				const float current_input_factor = group.second / static_cast<float>(group.first.size());
				for (const ProductionProgram::WareWorkerId& id : group.first) {
					if (wares->count(id) == 1) {
						input *= current_input_factor;
					}
				}
			}
			input /= overall_count;
			result *= input;

			// log_dbg("NOCOM input: %.2f", result);

			// The more output we generate, the more precious we are
			float output_factor = 1.f;
			output_factor *= static_cast<float>(link.outputs.first->size());
			for (const WareAmount& ware_amount : *link.outputs.first) {
				output_factor *= ware_amount.second;
				wares->insert({ware_amount.first, link.outputs.second});
			}
			// log_dbg("NOCOM output factor: %.2f", output_factor);
			result += output_factor;
		}
		return result;
	};

	log_dbg("====== Ware preciousness ========");

	for (const DescriptionIndex ware_index : wares_) {
		std::set<ProductionProgram::WareWorkerId> preciousness_wares;
		ProductionProgram::WareWorkerId ware_id({ware_index, WareWorker::wwWARE});
		preciousness_wares.insert(ware_id);
		const WareDescr* ware = get_ware_descr(ware_index);
		float preciousness = 0.f;

		std::set<DescriptionIndex> walked_productionsites;
		std::stack<DescriptionIndex> productionsites_to_walk;
		for (const auto& producer_idx : ware->consumers()) {
			if (idx2prod.count(producer_idx) == 1) {
				productionsites_to_walk.push(producer_idx);
			}
		}

		while (!productionsites_to_walk.empty()) {
			const DescriptionIndex prod_index = productionsites_to_walk.top();
			productionsites_to_walk.pop();
			walked_productionsites.insert(prod_index);
			preciousness += preciousness_at_building(idx2prod.at(prod_index), &preciousness_wares);

			for (const ProductionProgram::WareWorkerId& new_ware_id : preciousness_wares) {
				if (new_ware_id.type == WareWorker::wwWARE) {
					const WareDescr* new_ware = get_ware_descr(new_ware_id.index);
					for (const DescriptionIndex& new_producer_idx : new_ware->producers()) {
						if (walked_productionsites.count(new_producer_idx) == 0 && idx2prod.count(new_producer_idx) == 1) {
							// NOCOM some cutoff points here for circularity!
							bool walk_this = true;
							for (const auto& category : productionsite_categories_) {
								for (const ScoredDescriptionIndex& scored : category.second) {
									if (scored.score == 0) {
										walk_this = false;
										break;
									}
								}
							}
							if (walk_this) {
								productionsites_to_walk.push(new_producer_idx);
							}
						}
					}
				}
			}
		}

		// Construction
		constexpr float kBuildcostFactor = 1 / 6.f;
		constexpr float kEnhancementcostFactor = 1 / 12.f;
		if (construction_materials_.count(ware_index) == 1) {
			for (const DescriptionIndex building_index : buildings_) {
				const BuildingDescr* building_descr = get_building_descr(building_index);
				for (const auto& building_buildcost : building_descr->buildcost()) {
					if (building_buildcost.first == ware_index) {
						preciousness += building_buildcost.second * kBuildcostFactor;
					}
				}

				for (const auto& building_buildcost : building_descr->enhancement_cost()) {
					if (building_buildcost.first == ware_index) {
						preciousness += building_buildcost.second * kEnhancementcostFactor;
					}
				}
			}
		}

		ware_preciousness_[ware_index] = preciousness;
	}

	// Tools need a separate iteration, because they reuse the scores from the previous preciousness calculation
	for (const DescriptionIndex ware_index : wares_) {
		// Tools
		float preciousness = 0.f;
		const WareDescr* ware_descr = get_ware_descr(ware_index);

		float buildings_divisor = 0.f;

		// Scale by how buildings use the worker and how precious the building output is
		for (const ProductionSiteDescr* prod : productionsites) {
			for (const WareAmount& working_position : prod->working_positions()) {

				const WorkerDescr* worker_descr = get_worker_descr(working_position.first);
				if (worker_descr->is_buildable()) {
					for (const auto& worker_buildcost : worker_descr->buildcost()) {
						if (worker_buildcost.first == ware_descr->name()) {
							preciousness += worker_buildcost.second;

							float building_worker_score = 0.f;
							for (const DescriptionIndex output_ware_index : prod->output_ware_types()) {
								building_worker_score += ware_preciousness_[output_ware_index];
							}
							building_worker_score += prod->output_worker_types().size();
							if (!prod->output_ware_types().empty()) {
								building_worker_score /= static_cast<float>(prod->output_ware_types().size());
							}
							preciousness += building_worker_score * working_position.second;
							buildings_divisor += working_position.second;
						}
					}
				}
			}
		}

		constexpr float kDownscaler = 7.f;

		ware_preciousness_[ware_index] += preciousness / std::max(1.f, buildings_divisor * kDownscaler);

		log_dbg("\t%d\t%.2f\t%s", ware_descr->ai_hints().preciousness(name()), ware_preciousness_[ware_index], ware_descr->name().c_str());

		// NOCOM AI is still using vector instead of map for indices
		for (const auto& prec : ware_preciousness_) {
			descriptions.get_mutable_ware_descr(prec.first)->set_preciousness(name(), std::ceil(prec.second));
		}
	}
}
}  // namespace Widelands
