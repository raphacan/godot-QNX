/**************************************************************************/
/*  screen_thread.h                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/input/input_event.h"
#include "core/os/thread.h"
#include "core/templates/local_vector.h"
#include "servers/display/display_server_enums.h"

#include <screen/screen.h>

class QnxScreenThread {
public:
	struct ScreenData {
		screen_display_t display = nullptr;
		Rect2i geometry;
		float scale = 1.0f;
		float refresh_rate = -1.0f; // fallback value
		int dpi = 96;
	};

	struct PointerEvent {
		int pos[2];
		int displacement[2];
		int buttonMask;
		int wheel_vertical;
		int wheel_horizontal;
		int modifiers;

		Point2 get_pos() const {
			return Point2(pos[0], pos[1]);
		}

		void set_pos(const Point2 &p_pos) {
			pos[0] = static_cast<int>(p_pos.x);
			pos[1] = static_cast<int>(p_pos.y);
		}

		Point2 get_displacement() const {
			return Point2(displacement[0], displacement[1]);
		}

		void set_displacement(const Point2 &p_disp) {
			displacement[0] = static_cast<int>(p_disp.x);
			displacement[1] = static_cast<int>(p_disp.y);
		}
	};

	struct KeyboardEvent {
		int cap;
		int sym;
		int flags;
		int modifiers;
	};

	struct TouchEvent {
		int id;
		int pos[2];
		float pressure;
		float size;

		Point2 get_pos() const {
			return Point2(pos[0], pos[1]);
		}

		void set_pos(const Point2 &p_pos) {
			pos[0] = static_cast<int>(p_pos.x);
			pos[1] = static_cast<int>(p_pos.y);
		}
	};

	struct PropertyEvent {
		int object_type;
		int name;
	};

	struct ScreenEvent {
		int type = SCREEN_EVENT_NONE;
		screen_window_t screen_window = nullptr;
		union {
			PointerEvent pointer;
			KeyboardEvent keyboard;
			TouchEvent touch;
			PropertyEvent property;
		};
	};

private:
	screen_context_t screen_context = nullptr;
	screen_event_t m_event = nullptr;

	Vector<ScreenData> screens;

	SafeFlag thread_done;
	Mutex mutex;

	LocalVector<ScreenEvent> polled_events;

	Thread events_thread;

	bool initialized = false;

	bool alt_mem = false;
	bool shift_mem = false;
	bool control_mem = false;
	bool meta_mem = false;

	BitField<MouseButtonMask> buttons_state = MouseButtonMask::NONE;

	Point2i last_mouse_pos;
	Point2i last_click_pos = Point2i(-100, -100);
	uint64_t last_click_ms = 0;
	MouseButton last_click_button_index = MouseButton::NONE;

	Vector<TouchEvent> touch_events;

	static void _poll_events_thread(void *p_data);

	void _poll_events();

	void _wait_for_event();

	void _check_pending_events(LocalVector<ScreenEvent> &r_events);

	void _set_key_modifier_state(Ref<InputEventWithModifiers> r_state, Key p_keycode);

	static BitField<MouseButtonMask> _qnx_button_mask_to_godot_button_mask(int p_qnx_button_mask);
	static MouseButton _button_index_from_mask(BitField<MouseButtonMask> p_button_mask);

	static PointerEvent _get_pointer_event(screen_event_t p_screen_event);
	static KeyboardEvent _get_keyboard_event(screen_event_t p_screen_event);
	static TouchEvent _get_touch_event(screen_event_t p_screen_event);
	static PropertyEvent _get_property_event(screen_event_t p_screen_event);

public:
	Error init(screen_context_t p_screen_context);

	void destroy();

	int get_screen_count() const;
	Rect2i get_screen_geometry(int p_index) const;
	float get_screen_scale(int p_index) const;
	float get_screen_refresh_rate(int p_index) const;
	screen_display_t get_screen_handle(int p_index) const;

	LocalVector<ScreenEvent> get_polled_events();

	Point2i mouse_get_position() const;

	BitField<MouseButtonMask> mouse_get_button_state() const;

	void process_keyboard_event(const KeyboardEvent &p_keyboard_event, DisplayServerEnums::WindowID p_window_id);
	void process_pointer_event(const PointerEvent &p_pointer_event, DisplayServerEnums::WindowID p_window_id, const Rect2i &p_window_rect, const Point2i &p_screen_pos);
	void process_touch_event(const TouchEvent &p_touch_event, int p_type, DisplayServerEnums::WindowID p_window_id, const Rect2i &p_window_rect);
};
