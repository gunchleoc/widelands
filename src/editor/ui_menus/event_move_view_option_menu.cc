/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include <stdio.h>
#include "event_move_view_option_menu.h"
#include "event_move_view.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "editorinteractive.h"
#include "system.h"
#include "error.h"
#include "map.h"
#include "graphic.h"

Event_Move_View_Option_Menu::Event_Move_View_Option_Menu(Editor_Interactive* parent, Event_Move_View* event) :
   UIWindow(parent, 0, 0, 180, 200, "Event Option Menu") {
   m_parent=parent;
   m_event=event;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Move View Event Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   Coords pt=event->get_coords();
   m_x=pt.x;
   m_y=pt.y;
   const int offsx=5;
   const int offsy=25;
   int spacing=5;
   int posx=offsx;
   int posy=offsy;

   // Name editbox
   new UITextarea(this, spacing, posy, 50, 20, "Name:", Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
   m_name->set_text(event->get_name());
   posy+=20+spacing;

   // Only run once CB
   new UITextarea(this, spacing, posy, 150, 20, "Only run once: ", Align_CenterLeft);
   m_is_one_time_event=new UICheckbox(this, spacing+150, posy);
   m_is_one_time_event->set_state(m_event->is_one_time_event());

   posy+=20+spacing;
   // Set Field Buttons
   new UITextarea(this, spacing, posy, get_inner_w(), 15, "Current position: ", Align_CenterLeft);
   posy+=20+spacing;
   // X
   UIButton* b=new UIButton(this, spacing+20, posy, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+20, posy+40, 20, 20, 0, 4);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+40, posy, 20, 20, 0, 5);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+40, posy+40, 20, 20, 0, 6);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+60, posy, 20, 20, 0, 7);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+60, posy+40, 20, 20, 0, 8);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   new UITextarea(this, spacing+20, posy+20, 20, 20, "X: ", Align_CenterLeft);
   m_x_ta=new UITextarea(this, spacing+40, posy+20, 20, 20, "X: ", Align_CenterLeft);

   // Y
   int oldspacing=spacing;
   spacing=get_inner_w()/2+spacing;
   b=new UIButton(this, spacing, posy, 20, 20, 0, 9);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing, posy+40, 20, 20, 0, 10);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+20, posy, 20, 20, 0, 11);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+20, posy+40, 20, 20, 0, 12);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+40, posy, 20, 20, 0, 13);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   b=new UIButton(this, spacing+40, posy+40, 20, 20, 0, 14);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   new UITextarea(this, spacing, posy+20, 20, 20, "Y: ", Align_CenterLeft);
   m_y_ta=new UITextarea(this, spacing+20, posy+20, 20, 20, "Y: ", Align_CenterLeft);
   spacing=oldspacing;

   // Ok/Cancel Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy=get_inner_h()-20-spacing;
   b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title("Ok");
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Event_Move_View_Option_Menu::clicked);

   center_to_parent();
   update();
}

/*
 * cleanup
 */
Event_Move_View_Option_Menu::~Event_Move_View_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Event_Move_View_Option_Menu::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable

}

/*
 * a button has been clicked
 */
void Event_Move_View_Option_Menu::clicked(int i) {
   switch(i) {
      case 0:
         {
            // Cancel has been clicked
            end_modal(0);
            return;
         }
         break;

      case 1:
         {
            // ok button
            m_event->set_is_one_time_event(m_is_one_time_event->get_state());
            if(m_name->get_text())
               m_event->set_name(m_name->get_text());
            m_event->set_coords(Coords(m_x,m_y));
            end_modal(1);
            return;
         }
         break;

      case 3: m_x+=100; break;
      case 4: m_x-=100; break;
      case 5: m_x+=10; break;
      case 6: m_x-=10; break;
      case 7: m_x+=1; break;
      case 8: m_x-=1; break;
      case 9: m_y+=100; break;
      case 10: m_y-=100; break;
      case 11: m_y+=10; break;
      case 12: m_y-=10; break;
      case 13: m_y+=1; break;
      case 14: m_y-=1; break;
   }
   update();
}

/*
 * update function: update all UI elements
 */
void Event_Move_View_Option_Menu::update(void) {
   if(m_x<0) m_x=0;
   if(m_y<0) m_y=0;
   if(m_x>=((int)m_parent->get_map()->get_width())) m_x=m_parent->get_map()->get_width()-1;
   if(m_y>=((int)m_parent->get_map()->get_height())) m_y=m_parent->get_map()->get_height()-1;

   char buf[200];
   sprintf(buf, "%i", m_x);
   m_x_ta->set_text(buf);
   sprintf(buf, "%i", m_y);
   m_y_ta->set_text(buf);
}

