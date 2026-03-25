/**************************************************************************/
/*  display_server_wayland.cpp                                            */
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

#include "display_server_qnx.h"
#include "os_qnx.h"

#ifdef QNX_ENABLED

#include "servers/rendering/dummy/rasterizer_dummy.h"

#ifdef VULKAN_ENABLED
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif

#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"

static EGLNativeDisplayType DEFAULT_DISPLAY_ID = EGL_DEFAULT_DISPLAY;
#endif

static const int DEFAULT_SCREEN_DPI = 96;

#define DISPLAY_SERVER_QNX_DEBUG_LOGS_ENABLED
#ifdef DISPLAY_SERVER_QNX_DEBUG_LOGS_ENABLED
#define DEBUG_LOG_QNX(...) print_verbose(__VA_ARGS__)
#else
#define DEBUG_LOG_QNX(...)
#endif

uint64_t DisplayServerQnx::get_qnx_screen_context() {
	return (uint64_t)m_screenContext;
}

Rect2i DisplayServerQnx::_screen_get_rect(int p_screen) const {
	_THREAD_SAFE_METHOD_

	return screen_thread.get_screen_geometry(p_screen);
}

int DisplayServerQnx::_godot_cursor_to_qnx_cursor(CursorShape p_shape) {
	switch (p_shape) {
		case CURSOR_IBEAM:
			return SCREEN_CURSOR_SHAPE_IBEAM;
		case CURSOR_POINTING_HAND:
			return SCREEN_CURSOR_SHAPE_HAND;
		case CURSOR_CROSS:
			return SCREEN_CURSOR_SHAPE_CROSS;
		case CURSOR_WAIT:
			return SCREEN_CURSOR_SHAPE_WAIT;
		case CURSOR_DRAG:
			return SCREEN_CURSOR_SHAPE_GRAB;
		case CURSOR_CAN_DROP:
			return SCREEN_CURSOR_SHAPE_GRABBING;
		case CURSOR_MOVE:
			return SCREEN_CURSOR_SHAPE_MOVE;
		case CURSOR_ARROW:
		case CURSOR_FORBIDDEN:
		case CURSOR_VSIZE:
		case CURSOR_HSIZE:
		case CURSOR_BDIAGSIZE:
		case CURSOR_FDIAGSIZE:
		case CURSOR_BUSY:
		case CURSOR_VSPLIT:
		case CURSOR_HSPLIT:
		case CURSOR_HELP:
		case CURSOR_MAX:
		default:
			return SCREEN_CURSOR_SHAPE_ARROW;
	}
}

// Interface methods.

bool DisplayServerQnx::has_feature(Feature p_feature) const {
	switch (p_feature) {
#ifndef DISABLE_DEPRECATED
		case FEATURE_GLOBAL_MENU: {
			return (native_menu && native_menu->has_feature(NativeMenu::FEATURE_GLOBAL_MENU));
		} break;
#endif
		case FEATURE_MOUSE:
		//case FEATURE_MOUSE_WARP:
		//case FEATURE_CLIPBOARD:
		case FEATURE_CURSOR_SHAPE:
		//case FEATURE_CUSTOM_CURSOR_SHAPE:
		case FEATURE_WINDOW_TRANSPARENCY:
		//case FEATURE_HIDPI:
		case FEATURE_ORIENTATION:
		case FEATURE_TOUCHSCREEN:
		case FEATURE_SWAP_BUFFERS:
		case FEATURE_KEEP_SCREEN_ON:
		case FEATURE_SUBWINDOWS:
			//case FEATURE_IME:
			//case FEATURE_CLIPBOARD_PRIMARY:
			return true;
		//case FEATURE_NATIVE_DIALOG:
		//case FEATURE_NATIVE_DIALOG_INPUT:
		default:
			return false;
	}
}

String DisplayServerQnx::get_name() const {
	return "qnx";
}

int DisplayServerQnx::get_screen_count() const {
	return screen_thread.get_screen_count();
}

int DisplayServerQnx::get_primary_screen() const {
	return 0;
}

Point2i DisplayServerQnx::screen_get_position(int p_screen) const {
	_THREAD_SAFE_METHOD_

	return _screen_get_rect(p_screen).position;
}

Size2i DisplayServerQnx::screen_get_size(int p_screen) const {
	_THREAD_SAFE_METHOD_

	return _screen_get_rect(p_screen).size;
}

Rect2i DisplayServerQnx::screen_get_usable_rect(int p_screen) const {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	int screen_count = get_screen_count();
	ERR_FAIL_INDEX_V(p_screen, screen_count, Rect2i());

	return _screen_get_rect(p_screen);
}

int DisplayServerQnx::screen_get_dpi(int p_screen) const {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	int screen_count = get_screen_count();
	ERR_FAIL_INDEX_V(p_screen, screen_count, DEFAULT_SCREEN_DPI);

	screen_display_t display = screen_thread.get_screen_handle(p_screen);

	int dpi = DEFAULT_SCREEN_DPI;
	int res = screen_get_display_property_iv(display, SCREEN_PROPERTY_DPI, &dpi);
	ERR_FAIL_COND_V_MSG(res != 0, DEFAULT_SCREEN_DPI, "Failed to get DPI for screen " + itos(p_screen) + ".");
	return dpi;
}

float DisplayServerQnx::screen_get_scale(int p_screen) const {
	_THREAD_SAFE_METHOD_

	return screen_thread.get_screen_scale(p_screen);
}

float DisplayServerQnx::screen_get_refresh_rate(int p_screen) const {
	_THREAD_SAFE_METHOD_

	return screen_thread.get_screen_refresh_rate(p_screen);
}

bool DisplayServerQnx::is_touchscreen_available() const {
	return true;
}

void DisplayServerQnx::screen_set_orientation(ScreenOrientation p_orientation, int p_screen) {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	int screen_count = get_screen_count();
	ERR_FAIL_INDEX(p_screen, screen_count);

	screen_display_t display = screen_thread.get_screen_handle(p_screen);

	int orientation = SCREEN_ROTATION_NONE;
	switch (p_orientation) {
		case SCREEN_SENSOR_PORTRAIT:
		case SCREEN_PORTRAIT:
			orientation = SCREEN_ROTATION_90;
			break;
		case SCREEN_REVERSE_LANDSCAPE:
			orientation = SCREEN_ROTATION_180;
			break;
		case SCREEN_REVERSE_PORTRAIT:
			orientation = SCREEN_ROTATION_270;
			break;
		case SCREEN_LANDSCAPE:
		case SCREEN_SENSOR_LANDSCAPE:
		case SCREEN_SENSOR:
		default:
			orientation = SCREEN_ROTATION_NONE;
			break;
	}

	int res = screen_set_display_property_iv(display, SCREEN_PROPERTY_ROTATION, &orientation);
	ERR_FAIL_COND_MSG(res != 0, "Failed to set orientation for screen " + itos(p_screen) + ".");

	DEBUG_LOG_QNX(vformat("Screen %d orientation set to %d.", p_screen, orientation));
}

DisplayServer::ScreenOrientation DisplayServerQnx::screen_get_orientation(int p_screen) const {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	int screen_count = get_screen_count();
	ERR_FAIL_INDEX_V(p_screen, screen_count, SCREEN_LANDSCAPE);

	screen_display_t display = screen_thread.get_screen_handle(p_screen);

	int orientation = SCREEN_ROTATION_NONE;
	int res = screen_get_display_property_iv(display, SCREEN_PROPERTY_ROTATION, &orientation);
	ERR_FAIL_COND_V_MSG(res != 0, SCREEN_LANDSCAPE, "Failed to get orientation for screen " + itos(p_screen) + ".");

	switch (orientation) {
		case SCREEN_ROTATION_90:
			return SCREEN_PORTRAIT;
		case SCREEN_ROTATION_180:
			return SCREEN_REVERSE_LANDSCAPE;
		case SCREEN_ROTATION_270:
			return SCREEN_REVERSE_PORTRAIT;
		case SCREEN_ROTATION_NONE:
		default:
			return SCREEN_LANDSCAPE;
	}
}

void DisplayServerQnx::screen_set_keep_on(bool p_enable) {
}

bool DisplayServerQnx::screen_is_kept_on() const {
	return true;
}

Vector<DisplayServer::WindowID> DisplayServerQnx::get_window_list() const {
	_THREAD_SAFE_METHOD_

	Vector<int> ret;
	for (const KeyValue<WindowID, WindowData> &E : windows) {
		ret.push_back(E.key);
	}
	return ret;
}

DisplayServer::WindowID DisplayServerQnx::create_sub_window(WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect, bool p_exclusive, WindowID p_transient_parent) {
	_THREAD_SAFE_METHOD_

	screen_window_t parent_window = nullptr;
	if (p_transient_parent != INVALID_WINDOW_ID) {
		if (windows.has(p_transient_parent)) {
			parent_window = windows[p_transient_parent].screen_window;
		} else {
			ERR_PRINT("DisplayServerQnx::create_sub_window: transient parent window ID is invalid.");
		}
	}

	WindowID id = _create_window(p_mode, p_vsync_mode, p_flags, p_rect, parent_window);
#ifdef RD_ENABLED
	if (rendering_device) {
		rendering_device->screen_create(id);
	}
#endif

	return id;
}

void DisplayServerQnx::show_window(WindowID p_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_MSG(!windows.has(p_id), "DisplayServerQnx::show_window: window ID is invalid.");

	WindowData &wd = windows[p_id];
	if (wd.is_popup) {
		popup_list.push_back(p_id);
	}

	DEBUG_LOG_QNX(vformat("show_window: %d", p_id));
}

void DisplayServerQnx::delete_sub_window(WindowID p_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_MSG(!windows.has(p_id), "DisplayServerQnx::delete_sub_window: window ID is invalid.");
	ERR_FAIL_COND_MSG(p_id == MAIN_WINDOW_ID, "Main window can't be deleted");

	DEBUG_LOG_QNX(vformat("Deleting window ID %d.", p_id));

	List<WindowID>::Element *E = popup_list.find(p_id);
	while (E) {
		List<WindowID>::Element *next = E->next();
		popup_list.erase(E);
		E = next;
	}

	WindowData &wd = windows[p_id];

#if defined(RD_ENABLED)
	if (rendering_device) {
		rendering_device->screen_free(p_id);
	}

	if (rendering_context) {
		rendering_context->window_destroy(p_id);
	}
#endif
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->window_destroy(p_id);
	}
#endif

	if (0 != screen_destroy_window(wd.screen_window)) {
		ERR_PRINT("Failed to destroy screen window for window ID " + itos(p_id) + ".");
	}

	if (0 != screen_destroy_session(wd.pointer_session)) {
		ERR_PRINT("Failed to destroy pointer session for window ID " + itos(p_id) + ".");
	}

	windows.erase(p_id);
}

DisplayServer::WindowID DisplayServerQnx::window_get_active_popup() const {
	_THREAD_SAFE_METHOD_

	if (!popup_list.is_empty()) {
		return popup_list.back()->get();
	}

	return INVALID_WINDOW_ID;
}

void DisplayServerQnx::window_set_popup_safe_rect(WindowID p_window, const Rect2i &p_rect) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window));

	windows[p_window].safe_rect = p_rect;
}

Rect2i DisplayServerQnx::window_get_popup_safe_rect(WindowID p_window) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window), Rect2i());

	return windows[p_window].safe_rect;
}

int64_t DisplayServerQnx::window_get_native_handle(HandleType p_handle_type, WindowID p_window) const {
	ERR_FAIL_COND_V(!windows.has(p_window), 0);
	switch (p_handle_type) {
		case DISPLAY_HANDLE: {
			return _get_native_display_handle(p_window);
		} break;

		case WINDOW_HANDLE: {
			return (int64_t)windows[p_window].screen_window;
		} break;

		case WINDOW_VIEW: {
			return 0; // Not supported.
		} break;

#ifdef GLES3_ENABLED
		case OPENGL_CONTEXT: {
			if (egl_manager) {
				return (int64_t)egl_manager->get_context(p_window);
			}
			return 0;
		} break;
		case EGL_DISPLAY: {
			if (egl_manager) {
				return (int64_t)egl_manager->get_display(p_window);
			}
			return 0;
		}
		case EGL_CONFIG: {
			if (egl_manager) {
				return (int64_t)egl_manager->get_config(p_window);
			}
			return 0;
		}
#endif // GLES3_ENABLED

		default: {
			return 0;
		} break;
	}
}

int64_t DisplayServerQnx::_get_native_display_handle(WindowID p_window) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window), 0);

	const WindowData &wd = windows[p_window];

	screen_display_t display = nullptr;
	int res = screen_get_window_property_pv(wd.screen_window, SCREEN_PROPERTY_DISPLAY, reinterpret_cast<void **>(&display));
	ERR_FAIL_COND_V_MSG(res != 0, 0, "Can't acquire display information from window.");

	return (int64_t)display;
}

int DisplayServerQnx::window_get_current_screen(DisplayServer::WindowID p_window_id) const {
	_THREAD_SAFE_METHOD_

	int screen_count = get_screen_count();
	if (screen_count < 2) {
		// Early exit with single monitor.
		return 0;
	}

	ERR_FAIL_COND_V(!windows.has(p_window_id), INVALID_SCREEN);
	const WindowData &wd = windows[p_window_id];
	return wd.current_screen;
}

void DisplayServerQnx::window_set_current_screen(int p_screen, DisplayServer::WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));
	WindowData &wd = windows[p_window_id];

	p_screen = _get_screen_index(p_screen);
	int screen_count = get_screen_count();
	ERR_FAIL_INDEX(p_screen, screen_count);

	screen_display_t display = screen_thread.get_screen_handle(p_screen);

	int res = screen_set_window_property_pv(wd.screen_window, SCREEN_PROPERTY_DISPLAY, reinterpret_cast<void **>(&display));
	ERR_FAIL_COND_MSG(res != 0, "Failed to set screen " + itos(p_screen) + " for window " + itos(p_window_id) + ".");
	wd.current_screen = p_screen;
}

Point2i DisplayServerQnx::window_get_position(DisplayServer::WindowID p_window_id) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window_id), Point2i());
	return windows[p_window_id].rect.position;
}

void DisplayServerQnx::gl_window_make_current(DisplayServer::WindowID p_window_id) {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->window_make_current(p_window_id);
	}
#endif
}

void DisplayServerQnx::window_set_transient(DisplayServer::WindowID p_window_id, WindowID p_parent) {
	// Currently unsupported.
}

bool DisplayServerQnx::window_can_draw(DisplayServer::WindowID p_window_id) const {
	return window_get_mode(p_window_id) != WINDOW_MODE_MINIMIZED;
}

bool DisplayServerQnx::can_any_window_draw() const {
	_THREAD_SAFE_METHOD_

	for (const KeyValue<WindowID, WindowData> &E : windows) {
		if (window_get_mode(E.key) != WINDOW_MODE_MINIMIZED) {
			return true;
		}
	}

	return false;
}

// NOTE: While Wayland is supposed to be tear-free, wayland-protocols version
// 1.30 added a protocol for allowing async flips which is supposed to be
// handled by drivers such as Vulkan. We can then just ask to disable v-sync and
// hope for the best. See: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/commit/6394f0b4f3be151076f10a845a2fb131eeb56706
void DisplayServerQnx::window_set_vsync_mode(DisplayServer::VSyncMode p_vsync_mode, DisplayServer::WindowID p_window_id) {
#ifdef VULKAN_ENABLED
	if (rendering_context) {
		rendering_context->window_set_vsync_mode(p_window_id, p_vsync_mode);
	}
#endif // VULKAN_ENABLED

#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->set_use_vsync(p_vsync_mode != DisplayServer::VSYNC_DISABLED);
	}
#endif // GLES3_ENABLED
}

DisplayServer::VSyncMode DisplayServerQnx::window_get_vsync_mode(DisplayServer::WindowID p_window_id) const {
#ifdef VULKAN_ENABLED
	if (rendering_context) {
		return rendering_context->window_get_vsync_mode(p_window_id);
	}
#endif // VULKAN_ENABLED

#ifdef GLES3_ENABLED
	if (egl_manager) {
		return egl_manager->is_using_vsync() ? DisplayServer::VSYNC_ENABLED : DisplayServer::VSYNC_DISABLED;
	}
#endif // GLES3_ENABLED

	return DisplayServer::VSYNC_ENABLED;
}

void DisplayServerQnx::process_events() {
	ERR_FAIL_COND(!Thread::is_main_thread());

	LocalVector<QnxScreenThread::ScreenEvent> events = screen_thread.get_polled_events();

	for (const QnxScreenThread::ScreenEvent &event : events) {
		WindowID window_id = MAIN_WINDOW_ID;
		for (const KeyValue<WindowID, WindowData> &E : windows) {
			if (E.value.screen_window == event.screen_window) {
				window_id = E.key;
				break;
			}
		}
		int current_screen = window_get_current_screen(window_id);
		Point2i screen_pos = screen_get_position(current_screen);

		switch (event.type) {
			case SCREEN_EVENT_MTOUCH_TOUCH:
			case SCREEN_EVENT_MTOUCH_PRETOUCH:
			case SCREEN_EVENT_MTOUCH_MOVE:
			case SCREEN_EVENT_MTOUCH_RELEASE: {
				screen_thread.process_touch_event(event.touch, event.type, window_id, windows[window_id].rect_relative);
			} break;
			case SCREEN_EVENT_POINTER: {
				if (window_id != window_mouseover_id) {
					if ((window_mouseover_id != INVALID_WINDOW_ID) && windows.has(window_mouseover_id)) {
						_send_window_event(windows[window_mouseover_id], WINDOW_EVENT_MOUSE_EXIT);
					}
					window_mouseover_id = window_id;
					_send_window_event(windows[window_id], WINDOW_EVENT_MOUSE_ENTER);
				}
				screen_thread.process_pointer_event(event.pointer, window_id, windows[window_id].rect_relative, screen_pos);
			} break;
			case SCREEN_EVENT_KEYBOARD: {
				screen_thread.process_keyboard_event(event.keyboard, window_id);
			} break;
			case SCREEN_EVENT_PROPERTY: {
				if ((event.property.object_type == SCREEN_OBJECT_TYPE_WINDOW) && (event.property.name == SCREEN_PROPERTY_FOCUS)) {
					int focus = 0;
					if (0 != screen_get_window_property_iv(event.screen_window, SCREEN_PROPERTY_FOCUS, &focus)) {
						ERR_PRINT("Failed to get window focus property.");
					} else {
						if (focus != 0) {
							// window gained focus
							DEBUG_LOG_QNX(vformat("Window ID %d gained focus.", window_id));
							windows[window_id].focused = true;
#ifdef ACCESSKIT_ENABLED
							if (accessibility_driver) {
								accessibility_driver->accessibility_set_window_focused(window_id, true);
							}
#endif
							_send_window_event(windows[window_id], WINDOW_EVENT_FOCUS_IN);
						} else {
							// window lost focus
							DEBUG_LOG_QNX(vformat("Window ID %d lost focus.", window_id));
							windows[window_id].focused = false;
							Input::get_singleton()->release_pressed_events();
#ifdef ACCESSKIT_ENABLED
							if (accessibility_driver) {
								accessibility_driver->accessibility_set_window_focused(window_id, false);
							}
#endif
							_send_window_event(windows[window_id], WINDOW_EVENT_FOCUS_OUT);
						}
					}
				}
			} break;
			default: {
				WARN_PRINT(vformat("DisplayServerQnx::process_events() received unhandled event of type %d", event.type));
			} break;
		}
	} // for each event

	Input::get_singleton()->flush_buffered_events();
}

void DisplayServerQnx::release_rendering_thread() {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->release_current();
	}
#endif
}

void DisplayServerQnx::swap_buffers() {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->swap_buffers();
	}
#endif
}

void DisplayServerQnx::set_context(Context p_context) {
}

bool DisplayServerQnx::is_window_transparency_available() const {
#if defined(RD_ENABLED)
	if (rendering_device && !rendering_device->is_composite_alpha_supported()) {
		return false;
	}
#endif
	return OS::get_singleton()->is_layered_allowed();
}

Vector<String> DisplayServerQnx::get_rendering_drivers_func() {
	Vector<String> drivers;

#ifdef VULKAN_ENABLED
	drivers.push_back("vulkan");
#endif
#ifdef GLES3_ENABLED
	drivers.push_back("opengl3");
#endif
	drivers.push_back("dummy");

	return drivers;
}

void DisplayServerQnx::_mouse_update_mode() {
	MouseMode wanted_mouse_mode = mouse_mode_override_enabled
			? mouse_mode_override
			: mouse_mode_base;

	if (mouse_mode == wanted_mouse_mode) {
		return;
	}

	bool show_cursor = (wanted_mouse_mode == MOUSE_MODE_VISIBLE || wanted_mouse_mode == MOUSE_MODE_CONFINED);
	int qnx_cursor_shape = show_cursor ? _godot_cursor_to_qnx_cursor(current_cursor) : SCREEN_CURSOR_SHAPE_NONE;

	for (const KeyValue<WindowID, WindowData> &E : windows) {
		WindowData &wd = windows[E.key];

		if (0 != screen_set_session_property_iv(wd.pointer_session, SCREEN_PROPERTY_CURSOR, &qnx_cursor_shape)) {
			ERR_PRINT("Failed to set cursor shape for window ID " + itos(E.key) + ".");
		}
	}

	mouse_mode = wanted_mouse_mode;
}

void DisplayServerQnx::mouse_set_mode(MouseMode p_mode) {
	ERR_FAIL_INDEX(p_mode, MouseMode::MOUSE_MODE_MAX);
	if (p_mode == mouse_mode_base) {
		return;
	}
	mouse_mode_base = p_mode;
	_mouse_update_mode();
}

DisplayServer::MouseMode DisplayServerQnx::mouse_get_mode() const {
	return mouse_mode;
}

void DisplayServerQnx::mouse_set_mode_override(MouseMode p_mode) {
	ERR_FAIL_INDEX(p_mode, MouseMode::MOUSE_MODE_MAX);
	if (p_mode == mouse_mode_override) {
		return;
	}
	mouse_mode_override = p_mode;
	_mouse_update_mode();
}

DisplayServer::MouseMode DisplayServerQnx::mouse_get_mode_override() const {
	return mouse_mode_override;
}

void DisplayServerQnx::mouse_set_mode_override_enabled(bool p_override_enabled) {
	mouse_mode_override_enabled = p_override_enabled;
	_mouse_update_mode();
}

bool DisplayServerQnx::mouse_is_mode_override_enabled() const {
	return mouse_mode_override_enabled;
}

Point2i DisplayServerQnx::mouse_get_position() const {
	return screen_thread.mouse_get_position();
}

BitField<MouseButtonMask> DisplayServerQnx::mouse_get_button_state() const {
	return screen_thread.mouse_get_button_state();
}

void DisplayServerQnx::cursor_set_shape(CursorShape p_shape) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_INDEX(p_shape, CURSOR_MAX);

	if (p_shape == current_cursor) {
		return;
	}

	int qnx_cursor_shape = _godot_cursor_to_qnx_cursor(p_shape);

	for (const KeyValue<WindowID, WindowData> &E : windows) {
		WindowData &wd = windows[E.key];

		// Currently only default cursors are supported.
		if (0 != screen_set_session_property_iv(wd.pointer_session, SCREEN_PROPERTY_CURSOR, &qnx_cursor_shape)) {
			ERR_PRINT("Failed to set cursor shape for window ID " + itos(E.key) + ".");
		}
	}
	current_cursor = p_shape;

	DEBUG_LOG_QNX(vformat("Cursor shape set to %d.", qnx_cursor_shape));
}

DisplayServerQnx::CursorShape DisplayServerQnx::cursor_get_shape() const {
	return current_cursor;
}

DisplayServer *DisplayServerQnx::create_func(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error) {
	DisplayServer *ds = memnew(DisplayServerQnx(p_rendering_driver, p_mode, p_vsync_mode, p_flags, p_position, p_resolution, p_screen, p_context, p_parent_window, r_error));
	if (r_error != OK) {
		ERR_PRINT("Can't create the QNX display server.");
		memdelete(ds);

		return nullptr;
	}
	return ds;
}

DisplayServerQnx::DisplayServerQnx() {
	// empty on purpose: only for gdextension
}

void DisplayServerQnx::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_qnx_screen_context"), &DisplayServerQnx::get_qnx_screen_context);
}

DisplayServerQnx::DisplayServerQnx(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error) {
	r_error = ERR_UNAVAILABLE;

	default_display = &DEFAULT_DISPLAY_ID;

	int res = screen_create_context(&m_screenContext, SCREEN_BUFFER_PROVIDER_CONTEXT);
	if (0 != res) {
		r_error = ERR_UNAVAILABLE;
		ERR_FAIL_MSG(vformat("QNX Screen Display screen_create_context() failed (%d)", res));
	}

	Error thread_err = screen_thread.init(m_screenContext);
	if (thread_err != OK) {
		r_error = thread_err;
		ERR_FAIL_MSG("Failed to initialize QNX screen thread.");
	}

	native_menu = memnew(NativeMenu);

	// Input.
	Input::get_singleton()->set_event_dispatch_function(_dispatch_input_events);

	rendering_driver = p_rendering_driver;

	print_line("Requested rendering driver: " + rendering_driver);

	bool driver_found = false;
	String executable_name = OS::get_singleton()->get_executable_path().get_file();

	if (rendering_driver == "dummy") {
		RasterizerDummy::make_current();
		driver_found = true;
	}

#ifdef RD_ENABLED
#ifdef VULKAN_ENABLED
	if (rendering_driver == "vulkan") {
		rendering_context = memnew(RenderingContextDriverVulkanScreen);
	}
#endif // VULKAN_ENABLED

	if (rendering_context) {
		if (rendering_context->initialize() != OK) {
			memdelete(rendering_context);
			rendering_context = nullptr;
#if defined(GLES3_ENABLED)
			bool fallback_to_opengl3 = GLOBAL_GET("rendering/rendering_device/fallback_to_opengl3");
			if (fallback_to_opengl3 && rendering_driver != "opengl3") {
				WARN_PRINT("Your video card drivers seem not to support the required Vulkan version, switching to OpenGL ES 3.");
				rendering_driver = "opengl3";
				OS::get_singleton()->set_current_rendering_method("gl_compatibility");
				OS::get_singleton()->set_current_rendering_driver_name(rendering_driver);
			} else
#endif // GLES3_ENABLED
			{
				r_error = ERR_CANT_CREATE;

				if (p_rendering_driver == "vulkan") {
					OS::get_singleton()->alert(
							vformat("Your video card drivers seem not to support the required Vulkan version.\n\n"
									"If possible, consider updating your video card drivers or using the OpenGL 3 driver.\n\n"
									"You can enable the OpenGL 3 driver by starting the engine from the\n"
									"command line with the command:\n\n    \"%s\" --rendering-driver opengl3\n\n"
									"If you recently updated your video card drivers, try rebooting.",
									executable_name),
							"Unable to initialize Vulkan video driver");
				}

				ERR_FAIL_MSG(vformat("Could not initialize %s", rendering_driver));
			}
		}

		driver_found = true;
	}
#endif // RD_ENABLED

#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3") {
		egl_manager = memnew(GLManagerEGL_Screen);

		if (egl_manager->initialize(default_display) != OK || egl_manager->open_display(default_display) != OK) {
			memdelete(egl_manager);
			egl_manager = nullptr;
			r_error = ERR_CANT_CREATE;

			OS::get_singleton()->alert(
					vformat("Your video card drivers seem not to support the required OpenGL ES 3.0 version.\n\n"
							"If possible, consider updating your video card drivers or using the Vulkan driver.\n\n"
							"You can enable the Vulkan driver by starting the engine from the\n"
							"command line with the command:\n\n    \"%s\" --rendering-driver vulkan\n\n"
							"If you recently updated your video card drivers, try rebooting.",
							executable_name),
					"Unable to initialize OpenGL ES video driver");

			ERR_FAIL_MSG("Could not initialize OpenGL ES.");
		}
		driver_found = true;
	}

#endif // GLES3_ENABLED

	if (!driver_found) {
		r_error = ERR_UNAVAILABLE;
		ERR_FAIL_MSG("Video driver not found.");
	}

	cursor_set_shape(CURSOR_BUSY);

	Point2i window_position;
	if (p_position != nullptr) {
		window_position = *p_position;
		print_verbose(vformat("Window position: %d, %d", window_position.x, window_position.y));
	} else {
		if (p_screen == SCREEN_OF_MAIN_WINDOW) {
			p_screen = SCREEN_PRIMARY;
		}

		Rect2i scr_rect = screen_get_usable_rect(p_screen);
		print_verbose(vformat("Screen rect: %d, %d, %d, %d", scr_rect.position.x, scr_rect.position.y, scr_rect.size.x, scr_rect.size.y));
		window_position = scr_rect.position + (scr_rect.size - p_resolution) / 2;
	}
	print_verbose(vformat("window resolution: %d, %d", p_resolution.x, p_resolution.y));
	WindowID main_window = _create_window(p_mode, p_vsync_mode, p_flags, Rect2i(window_position, p_resolution), (screen_window_t)p_parent_window);
	if (main_window == INVALID_WINDOW_ID) {
		r_error = ERR_CANT_CREATE;
		return;
	}
	for (int i = 0; i < WINDOW_FLAG_MAX; i++) {
		if (p_flags & (1 << i)) {
			window_set_flag(WindowFlags(i), true, main_window);
		}
	}

#if defined(GLES3_ENABLED)
	if (rendering_driver == "opengl3") {
		RasterizerGLES3::make_current(false);
	}
#endif

#if defined(RD_ENABLED)
	if (rendering_context) {
		rendering_device = memnew(RenderingDevice);
		if (rendering_device->initialize(rendering_context, MAIN_WINDOW_ID) != OK) {
			memdelete(rendering_device);
			rendering_device = nullptr;
			memdelete(rendering_context);
			rendering_context = nullptr;
			r_error = ERR_UNAVAILABLE;
			return;
		}
		rendering_device->screen_create(MAIN_WINDOW_ID);

		RendererCompositorRD::make_current();
	}
#endif // RD_ENABLED
	r_error = OK;
}

DisplayServerQnx::~DisplayServerQnx() {
	if (native_menu) {
		memdelete(native_menu);
		native_menu = nullptr;
	}

	for (KeyValue<WindowID, WindowData> &E : windows) {
#if defined(RD_ENABLED)
		if (rendering_device) {
			rendering_device->screen_free(E.key);
		}

		if (rendering_context) {
			rendering_context->window_destroy(E.key);
		}
#endif
#ifdef GLES3_ENABLED
		if (egl_manager) {
			egl_manager->window_destroy(E.key);
		}
#endif
		WindowData &wd = E.value;
		screen_destroy_window(wd.screen_window);
		screen_destroy_session(wd.pointer_session);
	}
	windows.clear();

	screen_thread.destroy();

	// Destroy all drivers.
#ifdef RD_ENABLED
	if (rendering_device) {
		memdelete(rendering_device);
	}

	if (rendering_context) {
		memdelete(rendering_context);
	}
#endif

#ifdef GLES3_ENABLED
	if (egl_manager) {
		memdelete(egl_manager);
		egl_manager = nullptr;
	}
#endif

	if (m_screenContext) {
		screen_destroy_context(m_screenContext);
		m_screenContext = nullptr;
	}
}

void DisplayServerQnx::register_qnx_driver() {
	register_create_function("qnx", create_func, get_rendering_drivers_func);
}

DisplayServerQnx::WindowID DisplayServerQnx::_create_window(WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect, screen_window_t p_parent_window) {
	//Create window

	WindowID id = window_id_counter++;
	WindowData &wd = windows[id];

	int rq_screen = get_screen_from_rect(p_rect);
	if (rq_screen < 0) {
		rq_screen = get_primary_screen(); // Requested window rect is outside any screen bounds.
	}
	wd.current_screen = rq_screen;

	WindowID parent_window_id = INVALID_WINDOW_ID;
	Rect2i win_rect = p_rect;
	Point2i parent_rel_pos;
	if (p_parent_window) {
		for (const KeyValue<WindowID, WindowData> &E : windows) {
			if (E.value.screen_window == p_parent_window) {
				parent_window_id = E.key;
				parent_rel_pos = E.value.rect_relative.position;
				break;
			}
		}
	} else {
		// No parent.
		if (p_mode == WINDOW_MODE_FULLSCREEN || p_mode == WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
			Rect2i screen_rect = Rect2i(screen_get_position(rq_screen), screen_get_size(rq_screen));

			win_rect = screen_rect;
		} else {
			Rect2i srect = screen_get_usable_rect(rq_screen);
			Point2i wpos = p_rect.position;
			wpos = wpos.clamp(srect.position, srect.position + srect.size - p_rect.size / 3);

			win_rect.position = wpos;
		}
	}
	wd.parent = parent_window_id;
	wd.rect = win_rect;

	int res = 0;

	const int window_type = (p_flags & WINDOW_FLAG_POPUP) ? SCREEN_CHILD_WINDOW : SCREEN_APPLICATION_WINDOW;
	res = screen_create_window_type(&wd.screen_window, m_screenContext, window_type);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "screen_create_window() failed");

	const int screen_count = get_screen_count();
	ERR_FAIL_INDEX_V_MSG(rq_screen, screen_count, INVALID_WINDOW_ID, "Requested screen index is out of bounds.");

	screen_display_t display = screen_thread.get_screen_handle(rq_screen);
	res = screen_set_window_property_pv(wd.screen_window, SCREEN_PROPERTY_DISPLAY, reinterpret_cast<void **>(&display));
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "Failed to set screen " + itos(rq_screen) + " for window " + itos(id) + ".");

	constexpr int screen_format = SCREEN_FORMAT_RGBA8888;
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_FORMAT, &screen_format);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_FORMAT " + itos(screen_format) + " failed");

	int screen_usage = 0;
#ifdef VULKAN_ENABLED
	if (rendering_driver == "vulkan") {
		screen_usage = SCREEN_USAGE_VULKAN;
	}
#endif // VULKAN_ENABLED
#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3") {
		screen_usage = SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_OPENGL_ES3;
	}
#endif // GLES3_ENABLED
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_USAGE, &screen_usage);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_USAGE " + itos(screen_usage) + " failed");

	const int transparency = SCREEN_TRANSPARENCY_NONE;
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_TRANSPARENCY, &transparency);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_TRANSPARENCY " + itos(transparency) + " failed");

	const int swap_interval = (p_vsync_mode != DisplayServer::VSYNC_DISABLED) ? 1 : 0;
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_SWAP_INTERVAL, &swap_interval);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_SWAP_INTERVAL " + itos(swap_interval) + " failed");

	int window_size[2] = { wd.rect.size.width, wd.rect.size.height };
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_SIZE, &window_size[0]);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_SIZE failed");

	Point2i screen_position = screen_get_position(rq_screen);
	Point2i screen_rel_pos = win_rect.position - screen_position;

	if (window_type == SCREEN_CHILD_WINDOW) {
		// child windows need to be positioned relative to their parent.
		wd.rect_relative = Rect2i(screen_rel_pos - parent_rel_pos, win_rect.size);

		// Join parent's window group.
		char window_group_name[64] = { '\0' };
		res = screen_get_window_property_cv(p_parent_window, SCREEN_PROPERTY_ID, sizeof(window_group_name), window_group_name);
		ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "get SCREEN_PROPERTY_ID failed");

		res = screen_join_window_group(wd.screen_window, window_group_name);
		ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "screen_join_window_group() failed");

		DEBUG_LOG_QNX(vformat("Joined window group: %s", window_group_name));
	} else {
		// Application windows need to be positioned relative to the display.
		wd.rect_relative = Rect2i(screen_rel_pos, win_rect.size);
	}
	int window_pos[2] = { wd.rect_relative.position.x, wd.rect_relative.position.y };
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_POSITION, &window_pos[0]);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_POSITION failed");

	DEBUG_LOG_QNX(vformat("Created window with rect: (%d, %d, %d, %d) on display %d, mode: %d, flags: %d, parent: %d",
			window_pos[0], window_pos[1], window_size[0], window_size[1], rq_screen, p_mode, p_flags, parent_window_id));

	if (p_flags & WINDOW_FLAG_NO_FOCUS) {
		const int sensitivity = SCREEN_SENSITIVITY_NO_FOCUS;
		res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_SENSITIVITY, &sensitivity);
		ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "setting SCREEN_PROPERTY_SENSITIVITY failed");
	}

	const int window_buffers = 2;
	res = screen_create_window_buffers(wd.screen_window, window_buffers);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, vformat("screen_create_window_buffers() failed: %s", strerror(errno)));

	// Create pointer session per window to allow setting pointer shape
	res = screen_create_session_type(&wd.pointer_session, m_screenContext, SCREEN_EVENT_POINTER);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "screen_create_session_type() for pointer failed");

	res = screen_set_session_property_pv(wd.pointer_session, SCREEN_PROPERTY_WINDOW, (void **)&wd.screen_window);
	ERR_FAIL_COND_V_MSG(0 != res, INVALID_WINDOW_ID, "screen_set_session_property_pv() for pointer failed");

#if defined(RD_ENABLED)
	if (rendering_context) {
		union {
#ifdef VULKAN_ENABLED
			RenderingContextDriverVulkanScreen::WindowPlatformData vulkan;
#endif
		} wpd;
#ifdef VULKAN_ENABLED
		if (rendering_driver == "vulkan") {
			wpd.vulkan.window = wd.screen_window;
			wpd.vulkan.context = m_screenContext;
		}
#endif
		Error err = rendering_context->window_create(id, &wpd);
		ERR_FAIL_COND_V_MSG(err != OK, INVALID_WINDOW_ID, vformat("Can't create a %s window", rendering_driver));

		rendering_context->window_set_size(id, win_rect.size.width, win_rect.size.height);
		rendering_context->window_set_vsync_mode(id, p_vsync_mode);
	}
#endif
#ifdef GLES3_ENABLED
	if (egl_manager) {
		Error err = egl_manager->window_create(id, default_display, &wd.screen_window, win_rect.size.width, win_rect.size.height);
		ERR_FAIL_COND_V_MSG(err != OK, INVALID_WINDOW_ID, "Can't create EGL window");
	}

	window_set_vsync_mode(p_vsync_mode, id);
#endif

	return id;
}

DisplayServer::WindowID DisplayServerQnx::get_window_at_screen_position(const Point2i &p_position) const {
	WindowID found_window = INVALID_WINDOW_ID;
	for (const KeyValue<WindowID, WindowData> &E : windows) {
		const WindowData &wd = E.value;

		if (wd.rect.has_point(p_position)) {
			found_window = E.key;
		}
	}
	return found_window;
}

void DisplayServerQnx::window_attach_instance_id(ObjectID p_instance, WindowID p_window_id) {
	ERR_FAIL_COND(!windows.has(p_window_id));
	WindowData &wd = windows[p_window_id];

	wd.instance_id = p_instance;
}

ObjectID DisplayServerQnx::window_get_attached_instance_id(DisplayServer::WindowID p_window_id) const {
	ERR_FAIL_COND_V(!windows.has(p_window_id), ObjectID());
	return windows[p_window_id].instance_id;
}

void DisplayServerQnx::window_set_rect_changed_callback(Callable const &p_callable, DisplayServer::WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));

	windows[p_window_id].rect_changed_callback = p_callable;
}

void DisplayServerQnx::window_set_window_event_callback(Callable const &p_callable, DisplayServer::WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));

	windows[p_window_id].window_event_callback = p_callable;
}

void DisplayServerQnx::window_set_input_event_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window));

	windows[p_window].input_event_callback = p_callable;
}

void DisplayServerQnx::window_set_input_text_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window));

	windows[p_window].input_text_callback = p_callable;
}

void DisplayServerQnx::window_set_drop_files_callback(Callable const &p_callable, DisplayServer::WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));

	windows[p_window_id].drop_files_callback = p_callable;
}

void DisplayServerQnx::window_set_title(String const &, DisplayServer::WindowID) {
	//
}

void DisplayServerQnx::window_set_mouse_passthrough(Vector<Vector2> const &, DisplayServer::WindowID) {
	DEBUG_LOG_QNX("window_set_mouse_passthrough is not supported on QNX.\n");
}

Point2i DisplayServerQnx::window_get_position_with_decorations(DisplayServer::WindowID p_window_id) const {
	return window_get_position(p_window_id); // No decorations in QNX.
}

void DisplayServerQnx::window_set_position(const Vector2i &p_position, WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));
	WindowData &wd = windows[p_window_id];

	wd.rect.position = p_position;

	if (wd.parent != INVALID_WINDOW_ID) {
		// For child windows, position is relative to parent window's position.
		ERR_FAIL_COND_MSG(!windows.has(wd.parent), "Parent window does not exist");
		WindowData &parent_wd = windows[wd.parent];
		wd.rect_relative.position = wd.rect.position - parent_wd.rect.position;
	} else {
		int rq_screen = get_screen_from_rect(Rect2i(p_position, wd.rect.size));
		if (rq_screen < 0) {
			rq_screen = get_primary_screen(); // Requested window rect is outside any screen bounds.
		}
		wd.rect_relative = Rect2i(p_position - screen_get_position(rq_screen), wd.rect.size);
	}

	int res = 0;
	int window_pos[2] = { wd.rect_relative.position.x, wd.rect_relative.position.y };
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_POSITION, &window_pos[0]);
	ERR_FAIL_COND_MSG(0 != res, "setting SCREEN_PROPERTY_POSITION failed");
}

void DisplayServerQnx::window_set_max_size(const Size2i p_size, WindowID p_window_id) {
	// Not supported
}

Size2i DisplayServerQnx::window_get_max_size(int) const {
	return Size2i();
}

void DisplayServerQnx::window_set_min_size(const Size2i p_size, WindowID p_window_id) {
	// Not supported
}

Size2i DisplayServerQnx::window_get_min_size(DisplayServer::WindowID p_window_id) const {
	return Size2i();
}

void DisplayServerQnx::window_set_size(const Size2i p_size, WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));
	WindowData &wd = windows[p_window_id];

	if (wd.rect.size.width == p_size.width && wd.rect.size.height == p_size.height) {
		return;
	}

	Size2i size = p_size;
	size = size.maxi(1);

	int res = 0;

	int window_size[2] = { size.width, size.height };
	res = screen_set_window_property_iv(wd.screen_window, SCREEN_PROPERTY_SIZE, &window_size[0]);
	ERR_FAIL_COND_MSG(0 != res, "setting SCREEN_PROPERTY_SIZE failed");

	// Keep rendering context window size in sync
#if defined(RD_ENABLED)
	if (rendering_context) {
		rendering_context->window_set_size(p_window_id, size.width, size.height);
	}
#endif
	DEBUG_LOG_QNX(vformat("Window ID %d size set to: %d, %d", p_window_id, size.width, size.height));
}

Size2i DisplayServerQnx::window_get_size(DisplayServer::WindowID p_window_id) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window_id), Size2i());
	return windows[p_window_id].rect.size;
}

Size2i DisplayServerQnx::window_get_size_with_decorations(DisplayServer::WindowID p_window_id) const {
	return window_get_size(p_window_id); // No decorations in QNX.
}

void DisplayServerQnx::window_set_mode(DisplayServer::WindowMode p_mode, DisplayServer::WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));
	WindowData &wd = windows[p_window_id];

	if (wd.mode == p_mode) {
		return; // do nothing
	}

	WARN_PRINT("Window mode change is not supported on QNX.");
}

DisplayServer::WindowMode DisplayServerQnx::window_get_mode(DisplayServer::WindowID p_window_id) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window_id), DisplayServer::WINDOW_MODE_WINDOWED);
	return windows[p_window_id].mode;
}

bool DisplayServerQnx::window_is_maximize_allowed(DisplayServer::WindowID p_window_id) const {
	return false;
}

void DisplayServerQnx::window_set_flag(DisplayServer::WindowFlags p_flag, bool p_enabled, DisplayServer::WindowID p_window_id) {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!windows.has(p_window_id));
	WindowData &wd = windows[p_window_id];

	switch (p_flag) {
		case WINDOW_FLAG_ALWAYS_ON_TOP: {
			wd.on_top = p_enabled;
		} break;
		case WINDOW_FLAG_NO_FOCUS: {
			wd.no_focus = p_enabled;
		} break;
		case WINDOW_FLAG_POPUP: {
			wd.is_popup = p_enabled;
		} break;
		default: {
			// Unsupported flag, do nothing.
		} break;
	}
}

bool DisplayServerQnx::window_get_flag(DisplayServer::WindowFlags p_flag, DisplayServer::WindowID p_window_id) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window_id), false);
	const WindowData &wd = windows[p_window_id];

	switch (p_flag) {
		case WINDOW_FLAG_ALWAYS_ON_TOP: {
			return wd.on_top;
		} break;
		case WINDOW_FLAG_NO_FOCUS: {
			return wd.no_focus;
		} break;
		case WINDOW_FLAG_POPUP: {
			return wd.is_popup;
		} break;
		default: {
		} break;
	}
	return false; // Unsupported flag, return false.
}

void DisplayServerQnx::window_request_attention(DisplayServer::WindowID) {
	//
}

void DisplayServerQnx::window_move_to_foreground(DisplayServer::WindowID) {
	//
}

bool DisplayServerQnx::window_is_focused(DisplayServer::WindowID p_window_id) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!windows.has(p_window_id), false);

	const WindowData &wd = windows[p_window_id];

	return wd.focused;
}

void DisplayServerQnx::_send_window_event(const WindowData &p_wd, DisplayServer::WindowEvent p_event) {
	if (p_wd.window_event_callback.is_valid()) {
		Variant event = int(p_event);
		p_wd.window_event_callback.call(event);
	}
}

void DisplayServerQnx::_dispatch_input_events(const Ref<InputEvent> &p_event) {
	static_cast<DisplayServerQnx *>(get_singleton())->_dispatch_input_event(p_event);
}

void DisplayServerQnx::_dispatch_input_event(const Ref<InputEvent> &p_event) {
	Ref<InputEventFromWindow> event_from_window = p_event;

	if (event_from_window.is_valid()) {
		WindowID window_id = event_from_window->get_window_id();

		if (windows.has(window_id)) {
			Callable callable = windows[window_id].input_event_callback;
			if (callable.is_valid()) {
				callable.call(p_event);
			}
		}
	} else {
		// Send to all windows. Copy all pending callbacks, since callback can erase window.
		Vector<Callable> cbs;
		for (KeyValue<WindowID, WindowData> &E : windows) {
			Callable callable = E.value.input_event_callback;
			if (callable.is_valid()) {
				cbs.push_back(callable);
			}
		}

		for (const Callable &cb : cbs) {
			cb.call(p_event);
		}
	}
}

void DisplayServerQnx::_set_input_focus(WindowID p_window_id) {
}

#endif //QNX_ENABLED
