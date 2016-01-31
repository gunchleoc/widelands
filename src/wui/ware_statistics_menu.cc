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

#include "wui/ware_statistics_menu.h"

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/constants.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_player.h"
#include "wui/plot_area.h"
#include "wui/waresdisplay.h"

#define PLOT_HEIGHT 100

#define INACTIVE 0

static const char pic_tab_production[] = "pics/menu_tab_wares_production.png";
static const char pic_tab_consumption[] = "pics/menu_tab_wares_consumption.png";
static const char pic_tab_economy[] = "pics/menu_tab_wares_econ_health.png";
static const char pic_tab_stock[] = "pics/menu_tab_wares_stock.png"; // TODO(unknown): replace place holder

static const RGBColor colors[] = {
	RGBColor(115, 115, 115), //inactive
	RGBColor(255,   0,   0),
	RGBColor  (0, 144,  12),
	RGBColor  (0,   0, 255),
	RGBColor(249, 125,   6),
	RGBColor(255, 204,   0),
	RGBColor(101,   0,  33),
	RGBColor  (0, 143, 255),
	RGBColor  (0,  70,  31),
	RGBColor(106,  44,   5),
	RGBColor(143,   1,   1),
	RGBColor  (0, 255,   0),
	RGBColor (85,   0, 111),
	RGBColor  (0, 243, 255),
	RGBColor (32,  36,  32),
	RGBColor(189, 118,  24),
	RGBColor(255,   0,  51),
	RGBColor(153, 204,   0),
	RGBColor  (0,  60,  77),
	RGBColor (77,  15, 224),
	RGBColor(255,   0, 208),
	RGBColor(255,  51,   0),
	RGBColor (43,  72, 183),
	RGBColor(255,  93,  93),
	RGBColor (15, 177,  18),
	RGBColor(252,  89, 242),
	RGBColor(255, 255, 255),
	RGBColor (61, 214, 128),
	RGBColor(102, 102,   0),
	RGBColor(169, 118,  93),
	RGBColor(255, 204, 102),
	RGBColor (65,  50, 106),
	RGBColor(220, 255,   0),
	RGBColor(204,  51,   0),
	RGBColor(192,   0,  67),
	RGBColor(120, 203,  65),
	RGBColor(204, 255,  51),
	RGBColor (20, 104, 109),
	RGBColor(129,  80,  21),
	RGBColor(153, 153,   0),
	RGBColor(221,  69,  77),
	RGBColor (34, 136,  83),
	RGBColor(106, 107, 207),
	RGBColor(204, 102,  51),
	RGBColor(127,  28,  75),
	RGBColor(111, 142, 204),
	RGBColor(174,  10,   0),
	RGBColor (96, 143,  71),
	RGBColor(163,  74, 128),
	RGBColor(183, 142,  10),
	RGBColor(105, 155, 160),//shark infested water, run!
};

static const uint32_t colors_length = sizeof(colors) / sizeof(RGBColor);


struct StatisticWaresDisplay : public AbstractWaresDisplay {
private:
	std::vector<uint8_t> & color_map_;
public:
	StatisticWaresDisplay
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y,
		 const Widelands::TribeDescr & tribe,
		 boost::function<void(Widelands::DescriptionIndex, bool)> callback_function,
		 std::vector<uint8_t> & color_map)
	:
		 AbstractWaresDisplay(parent, x, y, tribe, Widelands::wwWARE, true, callback_function),
		 color_map_(color_map)
	{
		int w, h;
		get_desired_size(&w, &h);
		set_size(w, h);
	}
protected:
	std::string info_for_ware(Widelands::DescriptionIndex const /* ware */) override {
		return "";
	}

	RGBColor info_color_for_ware(Widelands::DescriptionIndex const ware) override
	{
		size_t index = static_cast<size_t>(ware);

		return colors[color_map_[index]];
	}
};

WareStatisticsMenu::WareStatisticsMenu
	(InteractivePlayer & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow
	(&parent, "ware_statistics", &registry, 400, 270, _("Ware Statistics")),
parent_(&parent)
{
	uint8_t const nr_wares = parent.get_player()->egbase().tribes().nrwares();

	//init color sets
	color_map_.resize(nr_wares);
	std::fill(color_map_.begin(), color_map_.end(), INACTIVE);
	active_colors_.resize(colors_length);
	std::fill(active_colors_.begin(), active_colors_.end(), 0);

	//  First, we must decide about the size.
	UI::Box * box = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, 5);
	box->set_border(5, 5, 5, 5);
	set_center_panel(box);

	//setup plot widgets
	//create a tabbed environment for the different plots
	uint8_t const tab_offset = 30;
	uint8_t const spacing = 5;
	uint8_t const plot_width = get_inner_w() - 2 * spacing;
	uint8_t const plot_height = PLOT_HEIGHT + tab_offset + spacing;

	UI::TabPanel * tabs =
		 new UI::TabPanel
			 (box, spacing, 0, g_gr->images().get("pics/but1.png"));


	plot_production_ =
		new WuiPlotArea
			(tabs,
			 0, 0, plot_width, plot_height);
	plot_production_->set_sample_rate(STATISTICS_SAMPLE_TIME);
	plot_production_->set_plotmode(WuiPlotArea::PLOTMODE_RELATIVE);

	tabs->add
		("production", g_gr->images().get(pic_tab_production),
			plot_production_, _("Production"));

	plot_consumption_ =
		new WuiPlotArea
			(tabs,
			 0, 0, plot_width, plot_height);
	plot_consumption_->set_sample_rate(STATISTICS_SAMPLE_TIME);
	plot_consumption_->set_plotmode(WuiPlotArea::PLOTMODE_RELATIVE);

	tabs->add
		("consumption", g_gr->images().get(pic_tab_consumption),
			plot_consumption_, _("Consumption"));

	plot_economy_ =
		new DifferentialPlotArea
			(tabs,
			 0, 0, plot_width, plot_height);
	plot_economy_->set_sample_rate(STATISTICS_SAMPLE_TIME);
	plot_economy_->set_plotmode(WuiPlotArea::PLOTMODE_RELATIVE);

	tabs->add
		("economy_health", g_gr->images().get(pic_tab_economy),
			plot_economy_, _("Economy Health"));

	plot_stock_ = new WuiPlotArea
			(tabs,
			 0, 0, plot_width, plot_height);
	plot_stock_->set_sample_rate(STATISTICS_SAMPLE_TIME);
	plot_stock_->set_plotmode(WuiPlotArea::PLOTMODE_ABSOLUTE);

	tabs->add
		("stock", g_gr->images().get(pic_tab_stock),
			plot_stock_, _("Stock"));

	tabs->activate(0);

	//add tabbed environment to box
	box->add(tabs, UI::Align::kLeft, true);

	//register statistics data
	for (Widelands::DescriptionIndex cur_ware = 0; cur_ware < nr_wares; ++cur_ware) {
		plot_production_->register_plot_data
			(cur_ware,
				parent.get_player()->get_ware_production_statistics
				(Widelands::DescriptionIndex(cur_ware)),
				colors[cur_ware]);

		plot_consumption_->register_plot_data
			(cur_ware,
				parent.get_player()->get_ware_consumption_statistics
				(Widelands::DescriptionIndex(cur_ware)),
				colors[cur_ware]);

		plot_economy_->register_plot_data
			(cur_ware,
				parent.get_player()->get_ware_production_statistics
				(Widelands::DescriptionIndex(cur_ware)),
				colors[cur_ware]);

		plot_economy_->register_negative_plot_data
			(cur_ware,
				parent.get_player()->get_ware_consumption_statistics
				(Widelands::DescriptionIndex(cur_ware)));

		plot_stock_->register_plot_data
			(cur_ware,
				parent.get_player()->get_ware_stock_statistics
				(Widelands::DescriptionIndex(cur_ware)),
				colors[cur_ware]);
	}

	box->add
		(new StatisticWaresDisplay
			(box, 0, 0, parent.get_player()->tribe(),
			 boost::bind(&WareStatisticsMenu::cb_changed_to, boost::ref(*this), _1, _2),
			 color_map_),
		 UI::Align::kLeft, true);
	box->add
		(new WuiPlotGenericAreaSlider
			(this, *plot_production_, this,
			0, 0, 100, 45,
			g_gr->images().get("pics/but1.png")),
		 UI::Align::kLeft, true);

}

/**
 * Callback for the ware buttons. Change the state of all ware statistics
 * simultaneously.
 */
void WareStatisticsMenu::cb_changed_to(Widelands::DescriptionIndex id, bool what) {
	if (what) { //activate ware
		//search lowest free color
		uint8_t color_index = INACTIVE;

		for (uint32_t i = 0; i < active_colors_.size(); ++i) {
			if (!active_colors_[i]) {
				//prevent index out of bounds
				color_index = std::min(i + 1, colors_length - 1);
				active_colors_[i] = 1;
				break;
			}
		}

		//assign color
		color_map_[static_cast<size_t>(id)] = color_index;
		plot_production_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);
		plot_consumption_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);
		plot_economy_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);
		plot_stock_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);

	} else { //deactivate ware
		uint8_t old_color = color_map_[static_cast<size_t>(id)];
		if (old_color != INACTIVE) {
			active_colors_[old_color - 1] = 0;
			color_map_[static_cast<size_t>(id)] = INACTIVE;
		}
	}

	plot_production_->show_plot(static_cast<size_t>(id), what);
	plot_consumption_->show_plot(static_cast<size_t>(id), what);
	plot_economy_->show_plot(static_cast<size_t>(id), what);
	plot_stock_->show_plot(static_cast<size_t>(id), what);
}

/**
 * Callback for the time buttons. Change the time axis of all ware
 * statistics simultaneously.
 */
void WareStatisticsMenu::set_time(int32_t timescale) {
	plot_production_->set_time_id(timescale);
	plot_consumption_->set_time_id(timescale);
	plot_economy_->set_time_id(timescale);
	plot_stock_->set_time_id(timescale);
}
