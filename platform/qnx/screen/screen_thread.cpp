/**************************************************************************/
/*  screen_thread.cpp                                                     */
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

#include "screen_thread.h"
#include "key_mapping_qnx_screen.h"

#include <sys/keycodes.h>

Error QnxScreenThread::init(screen_context_t p_screen_context) {
	if (nullptr == p_screen_context) {
		return ERR_INVALID_PARAMETER;
	}
	screen_context = p_screen_context;

	if (0 != screen_create_event(&m_event)) {
		return ERR_CANT_CREATE;
	}

	int screen_count = 0;
	if (0 != screen_get_context_property_iv(screen_context, SCREEN_PROPERTY_DISPLAY_COUNT, &screen_count)) {
		return FAILED;
	}

	Vector<screen_display_t> displays;
	displays.resize(screen_count);
	if (0 != screen_get_context_property_pv(screen_context, SCREEN_PROPERTY_DISPLAYS, (void **)displays.ptrw())) {
		return FAILED;
	}

	screens.resize(screen_count);

	for (int i = 0; i < screen_count; i++) {
		ScreenData &sd = screens.write[i];
		sd.display = displays[i];

		int viewport_position[2] = { 0, 0 };
#if __QNX__ >= 800
		// Starting with QNX 8, a position property can be directly assigned to displays via the graphics.conf
		if (0 != screen_get_display_property_iv(sd.display, SCREEN_PROPERTY_POSITION, &viewport_position[0])) {
			ERR_PRINT(vformat("Failed to get viewport position for screen %d.", i));
		}
#else
		// As a workaround for older QNX versions, we expect the definition of a window class named "display{display_id}"
		// for each display in graphics.conf in order to make the display position configurable/queryable.
		// If this fails, we will fall back to assuming a horizontal layout of displays based on their widths
		int display_id = 0;
		if (0 != screen_get_display_property_iv(sd.display, SCREEN_PROPERTY_ID, &display_id)) {
			ERR_PRINT(vformat("Failed to get display ID for screen %d.", i));
		}

		bool screen_position_obtained = false;
		screen_window_t temp_window;
		if (0 != screen_create_window(&temp_window, screen_context)) {
			ERR_PRINT(vformat("Failed to create temporary window for screen %d.", i));
		} else {
			String display_name = vformat("display%d", display_id);
			if (0 != screen_set_window_property_cv(temp_window, SCREEN_PROPERTY_CLASS, display_name.length(), display_name.ascii().get_data())) {
				WARN_PRINT(vformat("Could not read window properties for screen %d.", display_id));
			} else {
				if (0 != screen_get_window_property_iv(temp_window, SCREEN_PROPERTY_POSITION, &viewport_position[0])) {
					ERR_PRINT(vformat("Failed to get viewport position for screen %d: %s.", display_id, strerror(errno)));
				} else {
					screen_position_obtained = true;
				}
			}
		}
		if (!screen_position_obtained) {
			for (int j = 0; j < i; j++) {
				viewport_position[0] += screens[j].geometry.size.width;
			}
			WARN_PRINT(vformat("Using fallback position (%d, %d) for screen %d.", viewport_position[0], viewport_position[1], i));
		}
		screen_destroy_window(temp_window);
#endif
		int viewport_size[2] = { 0, 0 };
		if (0 != screen_get_display_property_iv(sd.display, SCREEN_PROPERTY_SIZE, &viewport_size[0])) {
			ERR_PRINT(vformat("Failed to get viewport size for screen %d.", i));
		}

		screen_display_mode_t display_mode;
		if (0 != screen_get_display_property_pv(sd.display, SCREEN_PROPERTY_MODE, (void **)&display_mode)) {
			ERR_PRINT(vformat("Failed to get display mode for screen %d.", i));
		} else {
			sd.refresh_rate = display_mode.refresh;
		}

		Rect2i rect;
		rect.position.x = viewport_position[0];
		rect.position.y = viewport_position[1];
		rect.size.width = viewport_size[0];
		rect.size.height = viewport_size[1];
		sd.geometry = rect;

		print_verbose(vformat("Screen %d: %s, refresh=%dHz", i, sd.geometry, sd.refresh_rate));
	}

	events_thread.start(_poll_events_thread, this);

	initialized = true;
	return OK;
}

void QnxScreenThread::destroy() {
	if (!initialized) {
		return;
	}

	if (events_thread.is_started()) {
		thread_done.set();

		events_thread.wait_to_finish();
	}

	if (m_event) {
		screen_destroy_event(m_event);
		m_event = nullptr;
	}
}

int QnxScreenThread::get_screen_count() const {
	return screens.size();
}

screen_display_t QnxScreenThread::get_screen_handle(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, screens.size(), nullptr);
	return screens[p_index].display;
}

Rect2i QnxScreenThread::get_screen_geometry(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, screens.size(), Rect2i());
	return screens[p_index].geometry;
}

float QnxScreenThread::get_screen_scale(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, screens.size(), 1.0f);
	return screens[p_index].scale;
}

float QnxScreenThread::get_screen_refresh_rate(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, screens.size(), -1.0f);
	return screens[p_index].refresh_rate;
}

LocalVector<QnxScreenThread::ScreenEvent> QnxScreenThread::get_polled_events() {
	MutexLock lock(mutex);
	LocalVector<ScreenEvent> events = polled_events;
	polled_events.clear();
	return events;
}

Point2i QnxScreenThread::mouse_get_position() const {
	return last_mouse_pos;
}

BitField<MouseButtonMask> QnxScreenThread::mouse_get_button_state() const {
	return buttons_state;
}

void QnxScreenThread::_poll_events_thread(void *p_data) {
	QnxScreenThread *self = static_cast<QnxScreenThread *>(p_data);
	self->_poll_events();
}

void QnxScreenThread::_poll_events() {
	while (!thread_done.is_set()) {
		_wait_for_event();

		{
			MutexLock lock(mutex);
			_check_pending_events(polled_events);
		}
	}
}

void QnxScreenThread::_wait_for_event() {
	const uint64_t timeout_ns = 1 * 1000 * 1000 * 1000; // 1 second
	int result = screen_get_event(screen_context, m_event, timeout_ns);
	ERR_FAIL_COND_MSG(0 != result, "Error while getting screen event");
}

void QnxScreenThread::_check_pending_events(LocalVector<ScreenEvent> &r_events) {
	int eventType = SCREEN_EVENT_NONE;
	const auto result = screen_get_event_property_iv(m_event, SCREEN_PROPERTY_TYPE, &eventType);
	if ((0 != result) || (SCREEN_EVENT_NONE == eventType)) {
		return;
	}

	ScreenEvent event;
	switch (eventType) {
		case SCREEN_EVENT_MTOUCH_PRETOUCH:
		case SCREEN_EVENT_MTOUCH_TOUCH:
		case SCREEN_EVENT_MTOUCH_MOVE:
		case SCREEN_EVENT_MTOUCH_RELEASE: {
			event.type = eventType;
			event.touch = _get_touch_event(m_event);
			break;
		}
		case SCREEN_EVENT_POINTER: {
			event.type = eventType;
			event.pointer = _get_pointer_event(m_event);
			break;
		}
		case SCREEN_EVENT_KEYBOARD: {
			event.type = eventType;
			event.keyboard = _get_keyboard_event(m_event);
			break;
		}
		case SCREEN_EVENT_PROPERTY: {
			event.type = eventType;
			event.property = _get_property_event(m_event);
			break;
		}
		default:
			break;
	}

	if (event.type != SCREEN_EVENT_NONE) {
		if (0 != screen_get_event_property_pv(m_event, SCREEN_PROPERTY_WINDOW, (void **)&event.screen_window)) {
			print_error(vformat("Failed to retrieve window pointer from event: %s", strerror(errno)));
		}
		r_events.push_back(event);
	}
}

void QnxScreenThread::_set_key_modifier_state(Ref<InputEventWithModifiers> ev, Key p_keycode) {
	if (p_keycode != Key::SHIFT) {
		ev->set_shift_pressed(shift_mem);
	}
	if (p_keycode != Key::ALT) {
		ev->set_alt_pressed(alt_mem);
	}
	if (p_keycode != Key::META) {
		ev->set_meta_pressed(meta_mem);
	}
	if (p_keycode != Key::CTRL) {
		ev->set_ctrl_pressed(control_mem);
	}
}

BitField<MouseButtonMask> QnxScreenThread::_qnx_button_mask_to_godot_button_mask(int p_qnx_button_mask) {
	BitField<MouseButtonMask> godot_button_mask = MouseButtonMask::NONE;

	if (p_qnx_button_mask & SCREEN_LEFT_MOUSE_BUTTON) {
		godot_button_mask.set_flag(MouseButtonMask::LEFT);
	}
	if (p_qnx_button_mask & SCREEN_MIDDLE_MOUSE_BUTTON) {
		godot_button_mask.set_flag(MouseButtonMask::MIDDLE);
	}
	if (p_qnx_button_mask & SCREEN_RIGHT_MOUSE_BUTTON) {
		godot_button_mask.set_flag(MouseButtonMask::RIGHT);
	}

	return godot_button_mask;
}

MouseButton QnxScreenThread::_button_index_from_mask(BitField<MouseButtonMask> p_button_mask) {
	switch (p_button_mask) {
		case MouseButtonMask::LEFT:
			return MouseButton::LEFT;
		case MouseButtonMask::RIGHT:
			return MouseButton::RIGHT;
		case MouseButtonMask::MIDDLE:
			return MouseButton::MIDDLE;
		case MouseButtonMask::MB_XBUTTON1:
			return MouseButton::MB_XBUTTON1;
		case MouseButtonMask::MB_XBUTTON2:
			return MouseButton::MB_XBUTTON2;
		default:
			return MouseButton::NONE;
	}
}

QnxScreenThread::PointerEvent QnxScreenThread::_get_pointer_event(screen_event_t p_screen_event) {
	PointerEvent pointer_event;

	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_POSITION, &pointer_event.pos[0]);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_DISPLACEMENT, &pointer_event.displacement[0]);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_BUTTONS, &pointer_event.buttonMask);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_MOUSE_WHEEL, &pointer_event.wheel_vertical);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_MOUSE_HORIZONTAL_WHEEL, &pointer_event.wheel_horizontal);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_MODIFIERS, &pointer_event.modifiers);

	return pointer_event;
}

QnxScreenThread::KeyboardEvent QnxScreenThread::_get_keyboard_event(screen_event_t p_screen_event) {
	KeyboardEvent keyboard_event;

	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_KEY_CAP, &keyboard_event.cap);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_SYM, &keyboard_event.sym);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_FLAGS, &keyboard_event.flags);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_MODIFIERS, &keyboard_event.modifiers);

	return keyboard_event;
}

QnxScreenThread::TouchEvent QnxScreenThread::_get_touch_event(screen_event_t p_screen_event) {
	TouchEvent touch_event;

	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_POSITION, &touch_event.pos[0]);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_TOUCH_ID, &touch_event.id);

	return touch_event;
}

QnxScreenThread::PropertyEvent QnxScreenThread::_get_property_event(screen_event_t p_screen_event) {
	PropertyEvent property_event;

	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_OBJECT_TYPE, &property_event.object_type);
	screen_get_event_property_iv(p_screen_event, SCREEN_PROPERTY_NAME, &property_event.name);

	return property_event;
}

void QnxScreenThread::process_pointer_event(const PointerEvent &p_pointer_event, DisplayServer::WindowID p_window_id, const Rect2i &p_window_rect, const Point2i &p_screen_pos) {
	BitField<MouseButtonMask> new_buttons_state = _qnx_button_mask_to_godot_button_mask(p_pointer_event.buttonMask);

	if (p_pointer_event.wheel_vertical != 0) {
		MouseButton button = p_pointer_event.wheel_vertical < 0 ? MouseButton::WHEEL_UP : MouseButton::WHEEL_DOWN;
		new_buttons_state.set_flag(mouse_button_to_mask(button));
	}
	if (p_pointer_event.wheel_horizontal != 0) {
		MouseButton button = p_pointer_event.wheel_horizontal < 0 ? MouseButton::WHEEL_LEFT : MouseButton::WHEEL_RIGHT;
		new_buttons_state.set_flag(mouse_button_to_mask(button));
	}

	if ((new_buttons_state != buttons_state)) {
		BitField<MouseButtonMask> changed_buttons = buttons_state.get_different(new_buttons_state);
		buttons_state = new_buttons_state;

		constexpr MouseButton buttons_to_test[] = {
			MouseButton::LEFT,
			MouseButton::MIDDLE,
			MouseButton::RIGHT,
			MouseButton::WHEEL_UP,
			MouseButton::WHEEL_DOWN,
			MouseButton::WHEEL_LEFT,
			MouseButton::WHEEL_RIGHT
		};

		for (MouseButton test_button : buttons_to_test) {
			BitField<MouseButtonMask> test_button_mask = mouse_button_to_mask(test_button);
			if (changed_buttons.has_flag(test_button_mask)) {
				Ref<InputEventMouseButton> ev;
				ev.instantiate();
				_set_key_modifier_state(ev, Key::NONE);
				ev->set_window_id(p_window_id);
				ev->set_position(p_pointer_event.get_pos() - p_window_rect.position);
				ev->set_global_position(ev->get_position() + p_screen_pos);
				ev->set_button_mask(new_buttons_state);
				ev->set_button_index(test_button);
				ev->set_pressed(buttons_state.has_flag(test_button_mask));

				if (ev->is_pressed()) {
					uint64_t diff = OS::get_singleton()->get_ticks_usec() / 1000 - last_click_ms;
					if (last_click_button_index == ev->get_button_index()) {
						if (diff < 400 && Vector2(last_click_pos).distance_to(ev->get_position()) < 5) {
							last_click_ms = 0;
							last_click_pos = Point2i(-100, -100);
							last_click_button_index = MouseButton::NONE;
							ev->set_double_click(true);
						}
					} else if (ev->get_button_index() < MouseButton::WHEEL_UP || ev->get_button_index() == MouseButton::WHEEL_RIGHT) {
						last_click_button_index = ev->get_button_index();
					}

					if (!ev->is_double_click()) {
						last_click_ms += diff;
						last_click_pos = ev->get_position();
					}
					print_verbose(vformat("Mouse click at (%d, %d), window ID: %d", (int)ev->get_position().x, (int)ev->get_position().y, p_window_id));
				}

				Input::get_singleton()->parse_input_event(ev);

				if (ev->is_pressed()) {
					// Send an event resetting immediately the wheel key.
					if (test_button == MouseButton::WHEEL_UP || test_button == MouseButton::WHEEL_DOWN || test_button == MouseButton::WHEEL_LEFT || test_button == MouseButton::WHEEL_RIGHT) {
						Ref<InputEventMouseButton> evd = ev->duplicate();
						_set_key_modifier_state(evd, Key::NONE);
						evd->set_pressed(false);
						BitField<MouseButtonMask> reset_buttons_mask = buttons_state;
						reset_buttons_mask.clear_flag(test_button_mask);
						evd->set_button_mask(reset_buttons_mask);
						Input::get_singleton()->parse_input_event(evd);
					}
				}
			}
		}
	}

	if (p_pointer_event.displacement[0] != 0 || p_pointer_event.displacement[1] != 0) {
		Ref<InputEventMouseMotion> ev;
		ev.instantiate();
		_set_key_modifier_state(ev, Key::NONE);
		ev->set_window_id(p_window_id);
		ev->set_position(p_pointer_event.get_pos() - p_window_rect.position);
		ev->set_global_position(ev->get_position() + p_screen_pos);
		ev->set_relative(p_pointer_event.get_displacement());
		ev->set_relative_screen_position(p_pointer_event.get_displacement());
		ev->set_button_mask(buttons_state);
		last_mouse_pos = ev->get_global_position();
		Input::get_singleton()->parse_input_event(ev);
	}
}

void QnxScreenThread::process_keyboard_event(const KeyboardEvent &p_keyboard_event, DisplayServer::WindowID p_window_id) {
	char32_t unicode = p_keyboard_event.sym;
	if (unicode >= KEYCODE_PC_KEYS) {
		unicode = 0;
	}

	Ref<InputEventKey> ev;
	ev.instantiate();
	ev->set_window_id(p_window_id);

	Key physical_keycode = godot_code_from_qnx_code(p_keyboard_event.sym);
	Key keycode = fix_keycode(unicode, physical_keycode);

	bool pressed = (p_keyboard_event.flags & KEY_DOWN) != 0;
	bool echo = (p_keyboard_event.flags & KEY_REPEAT) != 0;

	switch (physical_keycode) {
		case Key::SHIFT: {
			shift_mem = pressed;
		} break;
		case Key::ALT: {
			alt_mem = pressed;
		} break;
		case Key::CTRL: {
			control_mem = pressed;
		} break;
		case Key::META: {
			meta_mem = pressed;
		} break;
		default:
			break;
	}

	ev->set_keycode(keycode);
	ev->set_physical_keycode(physical_keycode);
	ev->set_key_label(fix_key_label(unicode, keycode));
	ev->set_location(godot_location_from_qnx_code(p_keyboard_event.sym));
	ev->set_pressed(pressed);
	ev->set_echo(echo);

	if (pressed) {
		ev->set_unicode(fix_unicode(unicode));
	}

	_set_key_modifier_state(ev, keycode);

	// Taken from DisplayServerX11.
	if (ev->get_keycode() == Key::BACKTAB) {
		// Make it consistent across platforms.
		ev->set_keycode(Key::TAB);
		ev->set_physical_keycode(Key::TAB);
		ev->set_shift_pressed(true);
	}

	Input::get_singleton()->parse_input_event(ev);
}

void QnxScreenThread::process_touch_event(const TouchEvent &p_touch, int p_type, DisplayServer::WindowID p_window_id, const Rect2i &p_window_rect) {
	switch (p_type) {
		case SCREEN_EVENT_MTOUCH_TOUCH: {
			touch_events.push_back(p_touch);

			Ref<InputEventScreenTouch> ev;
			ev.instantiate();
			ev->set_window_id(p_window_id);
			ev->set_index(p_touch.id);
			ev->set_pressed(true);
			ev->set_position(p_touch.get_pos() - p_window_rect.position);
			Input::get_singleton()->parse_input_event(ev);
		} break;
		case SCREEN_EVENT_MTOUCH_PRETOUCH: {
			// Pretouch can be ignored for now.
		} break;
		case SCREEN_EVENT_MTOUCH_MOVE: {
			for (int i = 0; i < touch_events.size(); i++) {
				if (touch_events[i].id != p_touch.id) {
					continue;
				}

				if (touch_events[i].get_pos() == p_touch.get_pos()) {
					continue; // Don't move unnecessarily.
				}

				Ref<InputEventScreenDrag> ev;
				ev.instantiate();
				ev->set_window_id(p_window_id);
				ev->set_index(p_touch.id);
				ev->set_position(p_touch.get_pos() - p_window_rect.position);
				ev->set_relative(p_touch.get_pos() - touch_events[i].get_pos());
				ev->set_relative_screen_position(ev->get_relative());
				// ev->set_pressure(p_points[idx].pressure);
				// ev->set_tilt(p_points[idx].tilt);
				Input::get_singleton()->parse_input_event(ev);
				touch_events.write[i].set_pos(p_touch.get_pos());
			}

		} break;
		case SCREEN_EVENT_MTOUCH_RELEASE: {
			for (int i = 0; i < touch_events.size(); i++) {
				if (touch_events[i].id == p_touch.id) {
					Ref<InputEventScreenTouch> ev;
					ev.instantiate();
					ev->set_window_id(p_window_id);
					ev->set_index(touch_events[i].id);
					ev->set_pressed(false);
					ev->set_position(touch_events[i].get_pos());
					Input::get_singleton()->parse_input_event(ev);
					touch_events.remove_at(i);
					break;
				}
			}
		} break;
		default: {
			WARN_PRINT(vformat("process_touch_event called with unexpected type %d", p_type));
			break;
		}
	}
}
