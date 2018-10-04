/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include "ui_basic/multilineeditbox.h"

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "base/utf8.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"

// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace {
// NOCOM rename and use properly
static const uint32_t RICHTEXT_MARGIN = 2;

/// Escape, convert newlines and apply editorfont
std::string make_richtext(const std::string& text) {
	std::string temp = richtext_escape(text);
	boost::replace_all(temp, "\n", "<br>");
	return as_editorfont(temp);
}
} // namespace

namespace UI {

struct MultilineEditbox::Data {
	Scrollbar scrollbar;

	/// The text in the edit box
	std::string text;

	/// The pre-rendered text in the edit box
	std::shared_ptr<const UI::RenderedText> rendered_text;

	/// Background color and texture
	const UI::PanelStyleInfo* background_style;

	/// Position of the cursor inside the text.
	/// 0 indicates that the cursor is before the first character,
	/// text.size() inidicates that the cursor is after the last character.
	uint32_t caret_index;

	const int lineheight;

	/// Maximum length of the text string, in bytes
	const uint32_t maxbytes;

	Data(MultilineEditbox&, const UI::PanelStyleInfo* style);
	void update_rendered_text();

	void scroll_cursor_into_view();
	void set_cursor_pos(uint32_t caret_index);

	uint32_t prev_char(uint32_t cursor);
	uint32_t next_char(uint32_t cursor);
	uint32_t snap_to_char(uint32_t cursor);

	void erase_bytes(uint32_t start, uint32_t end);
	void insert(uint32_t where, const std::string& s);

private:
	MultilineEditbox& owner;
};

/**
 * Initialize an editbox that supports multiline strings.
*/
MultilineEditbox::MultilineEditbox(
   Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style)
   : Panel(parent, x, y, w, h), d_(new Data(*this, g_gr->styles().editbox_style(style))) {
	set_handle_mouse(true);
	set_can_focus(true);
	set_thinks(false);
	set_handle_textinput();
}

MultilineEditbox::Data::Data(MultilineEditbox& o, const UI::PanelStyleInfo* style)
   : scrollbar(
        &o, o.get_w() - Scrollbar::kSize, 0, Scrollbar::kSize, o.get_h(), UI::PanelStyle::kWui),
     background_style(style),
     caret_index(0),
     lineheight(text_height()),
     maxbytes(std::min(g_gr->max_texture_size_for_font_rendering() *
                          g_gr->max_texture_size_for_font_rendering() /
                          (lineheight * lineheight),
                       std::numeric_limits<int32_t>::max())),
     owner(o) {
	scrollbar.set_pagesize(owner.get_h() - 2 * lineheight);
	scrollbar.set_singlestepsize(lineheight);
	update_rendered_text();
}

/**
 * Return the text currently stored by the editbox.
 */
const std::string& MultilineEditbox::get_text() const {
	return d_->text;
}

/**
 * Replace the currently stored text with something else.
 */
void MultilineEditbox::set_text(const std::string& text) {
	if (text == d_->text)
		return;

	d_->text = text;
	while (d_->text.size() > d_->maxbytes) {
		d_->erase_bytes(d_->prev_char(d_->text.size()), d_->text.size());
	}

	d_->update_rendered_text();
	d_->set_cursor_pos(0);

	changed();
}

/**
 * Erase the given range of bytes, adjust the cursor position, and update.
 */
void MultilineEditbox::Data::erase_bytes(uint32_t start, uint32_t end) {
	assert(start <= end);
	assert(end <= text.size());

	uint32_t nbytes = end - start;
	text.erase(start, nbytes);
	update_rendered_text();

	if (caret_index >= end) {
		set_cursor_pos(caret_index - nbytes);
	} else if (caret_index > start) {
		set_cursor_pos(start);
	}
}

/**
 * Find the starting byte of the previous character
 */
uint32_t MultilineEditbox::Data::prev_char(uint32_t cursor) {
	assert(cursor <= text.size());

	if (cursor == 0)
		return cursor;

	do {
		--cursor;
		// TODO(GunChleoc): See if we can go full ICU here.
	} while (cursor > 0 && Utf8::is_utf8_extended(text[cursor]));

	return cursor;
}

/**
 * Find the starting byte of the next character
 */
uint32_t MultilineEditbox::Data::next_char(uint32_t cursor) {
	assert(cursor <= text.size());

	if (cursor >= text.size())
		return cursor;

	do {
		++cursor;
	} while (cursor < text.size() && Utf8::is_utf8_extended(text[cursor]));

	return cursor;
}

/**
 * Return the starting offset of the (multi-byte) character that @p cursor points to.
 */
uint32_t MultilineEditbox::Data::snap_to_char(uint32_t cursor) {
	while (cursor > 0 && Utf8::is_utf8_extended(text[cursor]))
		--cursor;
	return cursor;
}

/**
 * The mouse was clicked on this editbox
*/
bool MultilineEditbox::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		return true;
	}
	return false;
}

/**
 * This is called by the UI code whenever a key press or release arrives
 */
bool MultilineEditbox::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		case SDLK_TAB:
			// Let the panel handle the tab key
			return get_parent()->handle_key(true, code);
		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DELETE:
			if (d_->caret_index < d_->text.size()) {
				d_->erase_bytes(d_->caret_index, d_->next_char(d_->caret_index));
				changed();
			}
			break;

		case SDLK_BACKSPACE:
			if (d_->caret_index > 0) {
				d_->erase_bytes(d_->prev_char(d_->caret_index), d_->caret_index);
				changed();
			}
			break;

		case SDLK_KP_4:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_LEFT: {
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				uint32_t newpos = d_->prev_char(d_->caret_index);
				while (newpos > 0 && isspace(d_->text[newpos]))
					newpos = d_->prev_char(newpos);
				while (newpos > 0) {
					uint32_t prev = d_->prev_char(newpos);
					if (isspace(d_->text[prev]))
						break;
					newpos = prev;
				}
				d_->set_cursor_pos(newpos);
			} else {
				d_->set_cursor_pos(d_->prev_char(d_->caret_index));
			}
			break;
		}

		case SDLK_KP_6:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_RIGHT:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				uint32_t newpos = d_->next_char(d_->caret_index);
				while (newpos < d_->text.size() && isspace(d_->text[newpos]))
					newpos = d_->next_char(newpos);
				while (newpos < d_->text.size() && !isspace(d_->text[newpos]))
					newpos = d_->next_char(newpos);
				d_->set_cursor_pos(newpos);
			} else {
				d_->set_cursor_pos(d_->next_char(d_->caret_index));
			}
			break;

		case SDLK_KP_2:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DOWN:
			if (d_->caret_index < d_->text.size()) {
				const int new_position = d_->rendered_text->skip_caret(d_->caret_index, RenderedText::LineSkip::kLineDown);
				log("NOCOM skip line forward to: %d\n", new_position);
				d_->set_cursor_pos(new_position);
			}
			break;

		case SDLK_KP_8:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_UP:
			if (d_->caret_index > 0) {
				d_->set_cursor_pos(d_->rendered_text->skip_caret(d_->caret_index, RenderedText::LineSkip::kLineUp));
			}
			break;

		case SDLK_KP_7:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_HOME:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				d_->set_cursor_pos(0);
			} else {
				d_->set_cursor_pos(d_->rendered_text->skip_caret(d_->caret_index, RenderedText::LineSkip::kStartOfLine));
			}
			break;

		case SDLK_KP_1:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_END:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				d_->set_cursor_pos(d_->text.size());
			} else {
				const int new_position = d_->rendered_text->skip_caret(d_->caret_index, RenderedText::LineSkip::kEndOfLine);
				d_->set_cursor_pos(new_position > -1 ? new_position : d_->text.size());
			}
			break;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			d_->insert(d_->caret_index, "\n");
			d_->update_rendered_text();
			d_->scroll_cursor_into_view();
			changed();
			break;

		default:
			break;
		}
		return true;
	}

	return Panel::handle_key(down, code);
}

bool MultilineEditbox::handle_textinput(const std::string& input_text) {
	if (d_->text.size() + input_text.size() <= d_->maxbytes) {
		d_->insert(d_->caret_index, input_text);
		changed();
		d_->update_rendered_text();
		d_->scroll_cursor_into_view();
	}
	return true;
}

/**
 * Grab the focus and redraw.
 */
void MultilineEditbox::focus(bool topcaller) {
	Panel::focus(topcaller);
}

/**
 * Redraw the Editbox
 */
void MultilineEditbox::draw(RenderTarget& dst) {
	draw_background(dst, *d_->background_style);

	// Draw border.
	if (get_w() >= 4 && get_h() >= 4) {
		static const RGBColor black(0, 0, 0);

		// bottom edge
		dst.brighten_rect(Recti(0, get_h() - 2, get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect(Recti(get_w() - 2, 0, 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Recti(0, 0, get_w() - 1, 1), black);
		dst.fill_rect(Recti(0, 1, get_w() - 2, 1), black);
		// left edge
		dst.fill_rect(Recti(0, 0, 1, get_h() - 1), black);
		dst.fill_rect(Recti(1, 0, 1, get_h() - 2), black);
	}

	if (has_focus())
		dst.brighten_rect(Recti(0, 0, get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);

	d_->rendered_text->draw(
	   dst, Vector2i::zero(), Recti(0, d_->scrollbar.get_scrollpos(), d_->rendered_text->width(),
									   d_->rendered_text->height() - d_->scrollbar.get_scrollpos()), g_fh->fontset()->is_rtl() ? UI::Align::kRight : UI::Align::kLeft);
	if (has_focus()) {
		//log("NOCOM ************************************\ntext to render: %s\n", d_->text.c_str());
		// NOCOM Vector2i caret_position = d_->rendered_text->calculate_caret_position(d_->cursor_pos);
		//log("NOCOM caret pos: %d, %d\n", d_->caret_position.x, d_->caret_position.y);

		d_->rendered_text->handle_caret(d_->caret_index, Vector2i(0, 0 - d_->scrollbar.get_scrollpos()), &dst);
	}
}

/**
 * Insert the given string starting at cursor position @p where.
 * Update the cursor so that it stays in the same place, but note that the cursor is
 * "right-magnetic":
 * If @p where is equal to the current cursor position, then the cursor is moved.
 * This is usually what one wants.
 */
void MultilineEditbox::Data::insert(uint32_t where, const std::string& s) {
	text.insert(where, s);
	if (caret_index >= where) {
		set_cursor_pos(caret_index + s.size());
	}
}

/**
 * Change the position of the cursor, cause a display refresh and scroll the cursor
 * into view when necessary.
 */
void MultilineEditbox::Data::set_cursor_pos(uint32_t newpos) {
	assert(newpos <= text.size());

	if (caret_index == newpos) {
		return;
	}

	caret_index = newpos;
	scroll_cursor_into_view();
}

/**
 * Ensure that the cursor is visible.
 */
void MultilineEditbox::Data::scroll_cursor_into_view() {
	const uint32_t top = rendered_text->handle_caret(caret_index).y;

	if (top < scrollbar.get_scrollpos()) {
		scrollbar.set_scrollpos(top - lineheight);
	} else if (top + lineheight > scrollbar.get_scrollpos() + owner.get_h()) {
		scrollbar.set_scrollpos(top - owner.get_h() + 2 * lineheight);
	}
}

/**
 * Re-wrap the string and update the scrollbar range accordingly.
 */
void MultilineEditbox::Data::update_rendered_text() {
	rendered_text = UI::g_fh->render(make_richtext(text), owner.get_w() - Scrollbar::kSize - 2 * RICHTEXT_MARGIN);
	scrollbar.set_steps(rendered_text->height() - owner.get_h());
}

}  // namespace UI
