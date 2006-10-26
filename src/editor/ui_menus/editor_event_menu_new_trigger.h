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

#ifndef __S__EDITOR_EVENT_MENU_NEW_TRIGGER_H
#define __S__EDITOR_EVENT_MENU_NEW_TRIGGER_H

#include "ui_window.h"

class Editor_Interactive;
class Trigger_Descr;
template <typename T> struct UIListselect;
class UIMultiline_Textarea;
class UIButton;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Editor_Event_Menu_New_Trigger : public UIWindow {
   public:
      Editor_Event_Menu_New_Trigger(Editor_Interactive*);
      ~Editor_Event_Menu_New_Trigger();

      bool handle_mouseclick(const Uint8 btn, const bool down, int mx, int my);

   private:
      void clicked(int);
      void selected(int);
      void double_clicked(int);

	UIListselect<Trigger_Descr &> * m_trigger_list;
      UIMultiline_Textarea* m_description;
      Editor_Interactive* m_parent;
      UIButton* m_ok_button;
};

#endif
