/*
 * Copyright (C) 2007-2018 by the Widelands Development Team
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

#include "wui/interactive_gamebase.h"

#include <memory>

#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "logic/findbob.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "wui/constructionsitewindow.h"
#include "wui/dismantlesitewindow.h"
#include "wui/game_summary.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/militarysitewindow.h"
#include "wui/productionsitewindow.h"
#include "wui/shipwindow.h"
#include "wui/trainingsitewindow.h"
#include "wui/unique_window_handler.h"
#include "wui/warehousewindow.h"

namespace {

std::string speed_string(int const speed) {
	if (speed) {
		return (boost::format("%u.%ux") % (speed / 1000) % (speed / 100 % 10)).str();
	}
	return _("PAUSE");
}

// Draws census and statistics on screen for the given map object
void draw_mapobject_infotext(RenderTarget* dst, const Vector2i& init_position, float scale, Widelands::MapObject* mapobject, const TextToDraw text_to_draw) {
	const std::string census_string = mapobject->info_string(Widelands::MapObject::InfoStringType::kCensus);
	if (census_string.empty()) {
		// If there is no census available for the map object, we also won't have any statistics.
		return;
	}

	const std::string statistics_string = (text_to_draw & TextToDraw::kStatistics) ? mapobject->info_string(Widelands::MapObject::InfoStringType::kStatistics) : "";
	if (census_string.empty() && statistics_string.empty()) {
		// Nothing to do
		return;
	}

	const int font_size = scale * UI_FONT_SIZE_SMALL;

	// We always render this so we can have a stable position for the statistics string.
	std::shared_ptr<const UI::RenderedText> rendered_census =
	   UI::g_fh->render(as_condensed(census_string, UI::Align::kCenter, font_size), 120 * scale);
	Vector2i position = init_position - Vector2i(0, 48) * scale;
	if (text_to_draw & TextToDraw::kCensus) {
		rendered_census->draw(*dst, position, UI::Align::kCenter);
	}

	if (!statistics_string.empty()) {
		std::shared_ptr<const UI::RenderedText> rendered_statistics =
		   UI::g_fh->render(as_condensed(statistics_string, UI::Align::kCenter, font_size));
		position.y += rendered_census->height() + text_height(font_size) / 4;
		rendered_statistics->draw(*dst, position, UI::Align::kCenter);
	}
}

}  // namespace

InteractiveGameBase::InteractiveGameBase(Widelands::Game& g,
                                         Section& global_s,
                                         PlayerType pt,
                                         bool const multiplayer)
   : InteractiveBase(g, global_s),
     chat_provider_(nullptr),
     multiplayer_(multiplayer),
     playertype_(pt) {
	buildingnotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) {
		   switch (note.action) {
		   case Widelands::NoteBuilding::Action::kFinishWarp: {
			   if (upcast(
			          Widelands::Building const, building, game().objects().get_object(note.serial))) {
				   const Widelands::Coords coords = building->get_position();
				   // Check whether the window is wanted
				   if (wanted_building_windows_.count(coords.hash()) == 1) {
					   const WantedBuildingWindow& wanted_building_window =
					      *wanted_building_windows_.at(coords.hash()).get();
					   UI::UniqueWindow* building_window =
					      show_building_window(coords, true, wanted_building_window.show_workarea);
					   building_window->set_pos(wanted_building_window.window_position);
					   if (wanted_building_window.minimize) {
						   building_window->minimize();
					   }
					   wanted_building_windows_.erase(coords.hash());
				   }
			   }
		   } break;
		   default:
			   break;
		   }
		});
}

/// \return a pointer to the running \ref Game instance.
Widelands::Game* InteractiveGameBase::get_game() const {
	return dynamic_cast<Widelands::Game*>(&egbase());
}

Widelands::Game& InteractiveGameBase::game() const {
	return dynamic_cast<Widelands::Game&>(egbase());
}

void InteractiveGameBase::set_chat_provider(ChatProvider& chat) {
	chat_provider_ = &chat;
	chat_overlay()->set_chat_provider(chat);
}

ChatProvider* InteractiveGameBase::get_chat_provider() {
	return chat_provider_;
}

void InteractiveGameBase::draw_overlay(RenderTarget& dst) {
	InteractiveBase::draw_overlay(dst);

	GameController* game_controller = game().game_controller();
	// Display the gamespeed.
	if (game_controller != nullptr) {
		std::string game_speed;
		uint32_t const real = game_controller->real_speed();
		uint32_t const desired = game_controller->desired_speed();
		if (real == desired) {
			if (real != 1000) {
				game_speed = as_condensed(speed_string(real));
			}
		} else {
			game_speed = as_condensed((boost::format
			                           /** TRANSLATORS: actual_speed (desired_speed) */
			                           (_("%1$s (%2$s)")) %
			                           speed_string(real) % speed_string(desired))
			                             .str());
		}

		if (!game_speed.empty()) {
			std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh->render(game_speed);
			rendered_text->draw(dst, Vector2i(get_w() - 5, 5), UI::Align::kRight);
		}
	}
}

void InteractiveGameBase::add_wanted_infotext(Vector2i coords, Widelands::MapObject* mapobject) {
	wanted_infotexts_.push_back(std::make_pair(coords, mapobject));
}

void InteractiveGameBase::draw_wanted_infotexts(RenderTarget* dst, float scale, const Widelands::Player* plr) {
	const auto text_to_draw = get_text_to_draw();
	// Rendering text is expensive, so let's just do it for only a few sizes.
	// The formula is a bit fancy to avoid too much text overlap.
	const float scale_for_text = std::round(2.f * (scale > 1.f ? std::sqrt(scale) : std::pow(scale, 2.f))) / 2.f;
	if (scale_for_text < 1.f) {
		return;
	}

	for (const auto& draw_my_text : wanted_infotexts_) {
		TextToDraw draw_text_for_this_mapobject = text_to_draw;
		const Widelands::Player* owner = draw_my_text.second->get_owner();
		if (owner != nullptr && plr != nullptr && !plr->see_all() && plr->is_hostile(*owner)) {
			draw_text_for_this_mapobject =
			   static_cast<TextToDraw>(draw_text_for_this_mapobject & ~TextToDraw::kStatistics);
		}
		if (draw_text_for_this_mapobject != TextToDraw::kNone) {
			draw_mapobject_infotext(dst, draw_my_text.first, scale_for_text, draw_my_text.second, draw_text_for_this_mapobject);
		}
	}
	wanted_infotexts_.clear();
}

void InteractiveGameBase::draw_immovable_and_enqueue_bobs_for_visible_field(RenderTarget* dst, float scale, const FieldsToDraw::Field& field, bool has_drawable_immovable, Widelands::BaseImmovable* imm) {
	// Draw immovable and enqueue its infotext
	if (has_drawable_immovable) {
		imm->draw(
		   egbase().get_gametime(), field.rendertarget_pixel, scale, dst);
		add_wanted_infotext(field.rendertarget_pixel.cast<int>(), imm);
	}
	// Draw bobs or enqueue them if we want to draw them later
	draw_or_enqueue_bobs(dst, scale, field);
}

void InteractiveGameBase::draw_or_enqueue_bobs(RenderTarget* dst, float scale, const FieldsToDraw::Field& field) {
	for (Widelands::Bob* bob = field.fcoords.field->get_first_bob(); bob;
		 bob = bob->get_next_bob()) {
		const Vector2f bob_drawpos = bob->calc_drawpos(egbase(), field.rendertarget_pixel, scale);
		// Defer drawing of bobs so that they won't disappear behind rocks
		// Ignore anything on a road - those bobs stay with the road
		if (bob_drawpos.y > field.rendertarget_pixel.y) {
			// Fix z-layering for walking nw and ne
			bobs_walking_north_.push_back(std::make_tuple(field.rendertarget_pixel, static_cast<Widelands::RoadType>(field.fcoords.field->get_roads()), bob));
		} else if (field.fcoords.field->get_roads() == Widelands::RoadType::kNone && (bob_drawpos.x > field.rendertarget_pixel.x)) {
			// Fix z-layering for walking w
			bobs_walking_west_.push_back(std::make_pair(field.rendertarget_pixel, bob));
		} else {
			bob->draw(egbase(), field.rendertarget_pixel, scale, dst);
		}
		add_wanted_infotext(bob_drawpos.cast<int>(), bob);
	}
}

void InteractiveGameBase::draw_bobs_queue(RenderTarget* dst,
										  float scale,
										  const FieldsToDraw::Field& field,
										  bool has_big_immovable,
										  bool has_building) {
	// Draw bobs from previous iteration that would have been hidden
	// We use this boolean to prevent critters from walking on top of trees
	for (auto bobs_iter = bobs_walking_west_.begin(); bobs_iter != bobs_walking_west_.end();) {
		const Vector2f& original_pixel = bobs_iter->first;
		// Only consider drawing if we're in the correct column, so that we can use the check for the immovable
		if (std::abs(original_pixel.x - field.rendertarget_pixel.x) < kTriangleWidth) {
			// This will prevent stonemasons from walking underneath rocks when walking back west to their quarry
			if (!has_big_immovable || (original_pixel.x > field.rendertarget_pixel.x)) {
				bobs_iter->second->draw(egbase(), original_pixel, scale, dst);
				bobs_iter = bobs_walking_west_.erase(bobs_iter);
			} else {
				++bobs_iter;
			}
		} else {
			++bobs_iter;
		}
	}

	for (auto bobs_iter = bobs_walking_north_.begin(); bobs_iter != bobs_walking_north_.end();) {
		const Vector2f& original_pixel = std::get<0>(*bobs_iter);
		const Widelands::RoadType roadtype = std::get<1>(*bobs_iter);
		Widelands::Bob* bob = std::get<2>(*bobs_iter);
		// Only consider drawing if we're in the correct column, so that we can use the check for the immovable
		const float horizontal_distance = std::abs(original_pixel.x - field.rendertarget_pixel.x);
		if ((horizontal_distance < kTriangleWidth) && roadtype != Widelands::RoadType::kNone) {
			// If the bob is on a road, only defer further if there is a big building involved. This keeps carriers on road behind rocks and carriers beside big buildings on top of the building when they pass the hotspot.
			if (!has_building || !has_big_immovable) {
				bob->draw(egbase(), original_pixel, scale, dst);
				bobs_iter = bobs_walking_north_.erase(bobs_iter);
			} else {
				++bobs_iter;
			}
		} else if ((horizontal_distance < kTriangleWidth / 2) && (!has_big_immovable || (original_pixel.y < field.rendertarget_pixel.y - kTriangleHeight))) {
			// This will prevent stonemasons from walking underneath rocks when walking back north-east or north-west to their quarry
			bob->draw(egbase(), original_pixel, scale, dst);
			bobs_iter = bobs_walking_north_.erase(bobs_iter);
		} else {
			++bobs_iter;
		}
	}
}

void InteractiveGameBase::draw_all_remaining_bobs(RenderTarget* dst, float scale) {
	// Make sure that we don't skip any bobs at the bottom and right edges
	for (const auto& drawme : bobs_walking_west_) {
		drawme.second->draw(egbase(), drawme.first, scale, dst);
	}
	bobs_walking_west_.clear();
	for (const auto& drawme : bobs_walking_north_) {
		std::get<2>(drawme)->draw(egbase(), std::get<0>(drawme), scale, dst);
	}
	bobs_walking_north_.clear();
}


/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 */
void InteractiveGameBase::postload() {
	show_buildhelp(false);
	on_buildhelp_changed(buildhelp());

	// Recalc whole map for changed owner stuff
	egbase().mutable_map()->recalc_whole_map(egbase().world());

	// Close game-relevant UI windows (but keep main menu open)
	fieldaction_.destroy();
	hide_minimap();
}

void InteractiveGameBase::start() {
	// Multiplayer games don't save the view position, so we go to the starting position instead
	if (is_multiplayer()) {
		Widelands::PlayerNumber pln = player_number();
		const Widelands::PlayerNumber max = game().map().get_nrplayers();
		if (pln == 0) {
			// Spectator, use the view of the first viable player
			for (pln = 1; pln <= max; ++pln) {
				if (game().get_player(pln)) {
					break;
				}
			}
		}
		// Adding a check, just in case there was no viable player found for spectator
		if (game().get_player(pln)) {
			map_view()->scroll_to_field(game().map().get_starting_pos(pln), MapView::Transition::Jump);
		}
	}
}

void InteractiveGameBase::on_buildhelp_changed(const bool value) {
	toggle_buildhelp_->set_perm_pressed(value);
}

void InteractiveGameBase::add_wanted_building_window(const Widelands::Coords& coords,
                                                     const Vector2i point,
                                                     bool was_minimal) {
	wanted_building_windows_.insert(std::make_pair(
	   coords.hash(), std::unique_ptr<const WantedBuildingWindow>(new WantedBuildingWindow(
	                     point, was_minimal, has_workarea_preview(coords)))));
}

UI::UniqueWindow* InteractiveGameBase::show_building_window(const Widelands::Coords& coord,
                                                            bool avoid_fastclick,
                                                            bool workarea_preview_wanted) {
	Widelands::BaseImmovable* immovable = game().map().get_immovable(coord);
	upcast(Widelands::Building, building, immovable);
	assert(building);
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("building_%d") % building->serial()).str());

	switch (building->descr().type()) {
	case Widelands::MapObjectType::CONSTRUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new ConstructionSiteWindow(*this, registry,
			                           *dynamic_cast<Widelands::ConstructionSite*>(building),
			                           avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::DISMANTLESITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new DismantleSiteWindow(
			   *this, registry, *dynamic_cast<Widelands::DismantleSite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::MILITARYSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new MilitarySiteWindow(*this, registry, *dynamic_cast<Widelands::MilitarySite*>(building),
			                       avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::PRODUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new ProductionSiteWindow(*this, registry,
			                         *dynamic_cast<Widelands::ProductionSite*>(building),
			                         avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::TRAININGSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new TrainingSiteWindow(*this, registry, *dynamic_cast<Widelands::TrainingSite*>(building),
			                       avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::WAREHOUSE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new WarehouseWindow(*this, registry, *dynamic_cast<Widelands::Warehouse*>(building),
			                    avoid_fastclick, workarea_preview_wanted);
		};
		break;
	// TODO(sirver,trading): Add UI for market.
	default:
		log("Unable to show window for building '%s', type '%s'.\n", building->descr().name().c_str(),
		    to_string(building->descr().type()).c_str());
		NEVER_HERE();
	}
	registry.create();
	return registry.window;
}

/**
 * See if we can reasonably open a ship window at the current selection position.
 * If so, do it and return true; otherwise, return false.
 */
bool InteractiveGameBase::try_show_ship_window() {
	const Widelands::Map& map = game().map();
	Widelands::Area<Widelands::FCoords> area(map.get_fcoords(get_sel_pos().node), 1);

	if (!(area.field->nodecaps() & Widelands::MOVECAPS_SWIM)) {
		return false;
	}

	std::vector<Widelands::Bob*> ships;
	if (map.find_bobs(area, &ships, Widelands::FindBobShip())) {
		for (Widelands::Bob* ship : ships) {
			if (can_see(ship->owner().player_number())) {
				// FindBobShip should have returned only ships
				assert(ship->descr().type() == Widelands::MapObjectType::SHIP);
				show_ship_window(dynamic_cast<Widelands::Ship*>(ship));
				return true;
			}
		}
	}
	return false;
}

void InteractiveGameBase::show_ship_window(Widelands::Ship* ship) {
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("ship_%d") % ship->serial()).str());
	registry.open_window = [this, &registry, ship] { new ShipWindow(*this, registry, ship); };
	registry.create();
}

void InteractiveGameBase::show_game_summary() {
	if (game_summary_.window) {
		game_summary_.window->set_visible(true);
		game_summary_.window->think();
		return;
	}
	new GameSummaryScreen(this, &game_summary_);
}
