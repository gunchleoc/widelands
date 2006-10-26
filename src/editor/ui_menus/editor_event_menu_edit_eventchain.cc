/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "constants.h"
#include "editorinteractive.h"
#include "editor_event_menu_edit_eventchain.h"
#include "editor_event_menu_edit_trigger_conditional.h"
#include "error.h"
#include "event.h"
#include "event_chain.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "map_event_manager.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "trigger_conditional.h"
#include "util.h"

Editor_Event_Menu_Edit_EventChain::Editor_Event_Menu_Edit_EventChain(Editor_Interactive* parent, EventChain* chain) :
UIWindow(parent, 0, 0, 505, 340, _("Edit Event Chain").c_str()),
m_parent(parent),
m_event_chain(chain)
{

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Edit Event Chain Menu"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   const int ls_width = 200;

   // Name
   new UITextarea( this, posx, posy, 60, 20, _("Name: "), Align_CenterLeft);
   m_name = new UIEdit_Box( this, posx + 60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
   m_name->set_text( m_event_chain->get_name() );
   posy += 20 + spacing;

   // More then once
   new UITextarea( this, posx + STATEBOX_WIDTH + spacing, posy, 120, STATEBOX_HEIGHT, _("Runs multiple times"), Align_CenterLeft);
   m_morethanonce = new UICheckbox( this, posx, posy );
   m_morethanonce->set_state( m_event_chain->get_repeating() );
   posy += STATEBOX_HEIGHT + spacing;
   const int lsoffsy = posy;

   // Event List
   new UITextarea(this, posx, lsoffsy, _("Events: "), Align_Left);
   m_events= new UIListselect<Event &>(this, spacing, lsoffsy+20, ls_width, get_inner_h()-lsoffsy-55);
   m_events->selected.set(this, &Editor_Event_Menu_Edit_EventChain::cs_selected);
   m_events->double_clicked.set(this, &Editor_Event_Menu_Edit_EventChain::cs_double_clicked);
   posx += ls_width + spacing;

   posy = 75;
   UIButton* b = new UIButton(this, posx, posy, 80, 20, 0);
   b->set_title(_("Conditional").c_str());
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_edit_trigger_contitional);
   posy += 20 + spacing + spacing;
   b = new UIButton(this, posx, posy, 80, 20, 0);
   b->set_title(_("New Event").c_str());
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_new_event);
   posy += 20 + spacing + spacing;
   b = new UIButton(this, posx, posy, 80, 20, 0);
   b->set_title("<-");
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_ins_event);
   posy += 20 + spacing + spacing;
   b->set_enabled( false );
   m_insert_btn = b;
   b = new UIButton(this, posx, posy, 80, 20, 0);
   b->set_title(_("Delete").c_str());
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_del_event);
   b->set_enabled( false );
   m_delete_btn = b;
   posy += 20 + spacing + spacing + spacing;

   b = new UIButton(this, posx+5, posy, 24, 24, 0);
   b->set_pic(g_gr->get_picture( PicMod_UI, "pics/scrollbar_up.png"));
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_move_up);
   b->set_enabled( false );
   m_mvup_btn = b;
   b = new UIButton(this, posx+51, posy, 24, 24, 0);
   b->set_pic(g_gr->get_picture( PicMod_UI, "pics/scrollbar_down.png"));
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_move_down);
   b->set_enabled( false );
   m_mvdown_btn = b;
   posy += 24 + spacing + spacing;

   posx += 80 + spacing;
   new UITextarea(this, posx, lsoffsy, _("Available Events: "), Align_Left);
   m_available_events=new UIListselect<Event &>(this, posx, lsoffsy+20, ls_width, get_inner_h()-lsoffsy-55);
   m_available_events->selected.set(this, &Editor_Event_Menu_Edit_EventChain::tl_selected);
   m_available_events->double_clicked.set(this, &Editor_Event_Menu_Edit_EventChain::tl_double_clicked);
	const MapEventManager & mem = parent->get_egbase()->get_map()->get_mem();
	const MapEventManager::Index nr_events = mem.get_nr_events();
	for (MapEventManager::Index i = 0; i < nr_events; ++i) {
		Event & event = mem.get_event_by_nr(i);
		m_available_events->add_entry(event.get_name(), event);
   }
   m_available_events->sort();

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;
   b=new UIButton(this, posx, posy, 80, 20, 0);
   b->set_title(_("Ok").c_str());
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_ok);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 80, 20, 1);
   b->set_title(_("Cancel").c_str());
   b->clicked.set(this, &Editor_Event_Menu_Edit_EventChain::clicked_cancel);

   for( uint i = 0; i < m_event_chain->get_nr_events(); i++ ) {
		Event & event = *m_event_chain->get_event(i);
		m_events->add_entry(event.get_name(), event);
   }

   m_edit_trigcond = m_event_chain->get_trigcond() ? false : true;

   center_to_parent();
}

/*
 * cleanup
 */
Editor_Event_Menu_Edit_EventChain::~Editor_Event_Menu_Edit_EventChain(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Editor_Event_Menu_Edit_EventChain::handle_mouseclick
(const Uint8 btn, const bool down, int, int)
{
	if (btn & SDL_BUTTON_RIGHT and down) {
      clicked_cancel();
      return true;
   } else
      return false; // we're not dragable
}

/*
 * Think.
 *
 * Maybe we have to simulate a click
 */
void Editor_Event_Menu_Edit_EventChain::think()
{if (m_edit_trigcond) clicked_edit_trigger_contitional();}

void Editor_Event_Menu_Edit_EventChain::clicked_cancel() {end_modal(0);}

void Editor_Event_Menu_Edit_EventChain::clicked_ok() {
      // Name
      m_event_chain->set_name( m_name->get_text() );
      // Repeating
      m_event_chain->set_repeating( m_morethanonce->get_state() );
      // Trigger Conditional is always updated
      // Events
      m_event_chain->clear_events();
	const uint nr_events = m_events->get_nr_entries();
	for (uint i = 0; i < nr_events; i++)
		m_event_chain->add_event(&m_events->get_entry(i));
      end_modal(1);
}

void Editor_Event_Menu_Edit_EventChain::clicked_new_event() {
      // TODO
}

void Editor_Event_Menu_Edit_EventChain::clicked_edit_trigger_contitional() {
      Editor_Event_Menu_Edit_TriggerConditional* menu = new Editor_Event_Menu_Edit_TriggerConditional( m_parent, m_event_chain->get_trigcond(), m_event_chain );
      int code = menu->run();
      if( code ) {
         if( m_event_chain->get_trigcond() ) {
            m_event_chain->get_trigcond()->unreference_triggers( m_event_chain );
            delete m_event_chain->get_trigcond();
         }
         m_event_chain->set_trigcond( menu->get_trigcond() );
      }
      delete menu;
}


void Editor_Event_Menu_Edit_EventChain::clicked_ins_event() {
	Event & event = m_available_events->get_selection();
	m_events->add_entry(event.get_name(), event, true);
}


void Editor_Event_Menu_Edit_EventChain::clicked_del_event() {
      m_events->remove_entry( m_events->get_selection_index() );
      m_mvup_btn->set_enabled( false );
      m_mvdown_btn->set_enabled( false );
      m_delete_btn->set_enabled( false );
}


void Editor_Event_Menu_Edit_EventChain::clicked_move_up() {
	assert(m_events->has_selection());  //  Button should have been disabled.
	const uint n = m_events->get_selection_index();
	assert(n != 0);  //  Button should have been disabled.
         m_events->switch_entries( n, n - 1);
}


void Editor_Event_Menu_Edit_EventChain::clicked_move_down() {
	assert(m_events->has_selection());  //  Button should have been disabled.
	const uint n = m_events->get_selection_index();
	assert(n != m_events->get_nr_entries() - 1);  //  Button should have been disabled.
         m_events->switch_entries( n, n + 1);
}

/*
 * the listbox got selected
 */
void Editor_Event_Menu_Edit_EventChain::tl_selected(int) {
   m_insert_btn->set_enabled( true );
}
void Editor_Event_Menu_Edit_EventChain::cs_selected(int) {
   m_mvdown_btn->set_enabled( true );
   m_mvup_btn->set_enabled( true );
   m_delete_btn->set_enabled( true );
}

/*
 * listbox got double clicked
 */
void Editor_Event_Menu_Edit_EventChain::tl_double_clicked(int)
{clicked_ins_event();}

void Editor_Event_Menu_Edit_EventChain::cs_double_clicked(int)
{clicked_del_event();}
