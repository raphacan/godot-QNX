/**************************************************************************/
/*  display_server_qnx.h                                              */
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

#ifdef QNX_ENABLED

#include "screen/screen_thread.h"

#ifdef RD_ENABLED
#include "servers/rendering/rendering_device.h"

#ifdef VULKAN_ENABLED
#include "rendering_context_driver_vulkan_screen.h" // TODO
#endif

#endif //RD_ENABLED

#ifdef GLES3_ENABLED
#include "egl_manager_qnx_screen.h"
#endif

#if defined(SPEECHD_ENABLED)
#include "tts_linux.h"
#endif

#ifdef DBUS_ENABLED
#include "freedesktop_portal_desktop.h"
#include "freedesktop_screensaver.h"
#endif

#include "core/config/project_settings.h"
#include "core/input/input.h"
#include "servers/display/display_server.h"

#include <screen/screen.h>

#include <limits.h>
#include <stdio.h>

#undef CursorShape

class DisplayServerQnx : public DisplayServer {
	GDCLASS(DisplayServerQnx, DisplayServer)

	_THREAD_SAFE_CLASS_

	struct WindowData {
		screen_window_t screen_window = nullptr;
		screen_session_t pointer_session = nullptr;

		ObjectID instance_id;

		Size2i min_size;
		Size2i max_size;
		Rect2i rect;
		Rect2i rect_relative; // relative to display

		Rect2i safe_rect;

		Callable rect_changed_callback;
		Callable window_event_callback;
		Callable input_event_callback;
		Callable input_text_callback;
		Callable drop_files_callback;

		int current_screen = INVALID_SCREEN;
		WindowID parent = INVALID_WINDOW_ID;
		WindowMode mode = WINDOW_MODE_WINDOWED;

		bool focused = true;

		bool on_top = false;
		bool no_focus = false;
		bool is_popup = false;
	};

	HashMap<WindowID, WindowData> windows;
	WindowID window_id_counter = MAIN_WINDOW_ID;

	List<WindowID> popup_list;

	WindowID window_mouseover_id = INVALID_WINDOW_ID;

	CursorShape current_cursor = CURSOR_ARROW;
	MouseMode mouse_mode = MouseMode::MOUSE_MODE_VISIBLE;
	MouseMode mouse_mode_base = MouseMode::MOUSE_MODE_VISIBLE;
	MouseMode mouse_mode_override = MouseMode::MOUSE_MODE_VISIBLE;
	bool mouse_mode_override_enabled = false;
	void _mouse_update_mode();

	QnxScreenThread screen_thread;

	String rendering_driver;

	void *default_display = nullptr;

	NativeMenu *native_menu = nullptr;

	screen_context_t m_screenContext = nullptr;

	Rect2i _screen_get_rect(int p_screen) const;

	WindowID _create_window(WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect, screen_window_t p_parent_window);

	int64_t _get_native_display_handle(WindowID p_window) const;

	static int _godot_cursor_to_qnx_cursor(CursorShape p_shape);

public:
	uint64_t get_qnx_screen_context(); // return type needed for gdextension

	virtual bool has_feature(Feature p_feature) const override;

	virtual String get_name() const override;

#if defined(RD_ENABLED)
	RenderingContextDriver *rendering_context = nullptr;
	RenderingDevice *rendering_device = nullptr;
#endif

#if defined(GLES3_ENABLED)
	// GLManager_Screen *gl_manager = nullptr;
	GLManagerEGL_Screen *egl_manager = nullptr;
#endif

	virtual int get_screen_count() const override;
	virtual int get_primary_screen() const override;
	virtual Point2i screen_get_position(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual Size2i screen_get_size(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual Rect2i screen_get_usable_rect(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual int screen_get_dpi(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual float screen_get_scale(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual float screen_get_refresh_rate(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual bool is_touchscreen_available() const override;

	virtual void screen_set_orientation(ScreenOrientation p_orientation, int p_screen = SCREEN_OF_MAIN_WINDOW);
	virtual ScreenOrientation screen_get_orientation(int p_screen = SCREEN_OF_MAIN_WINDOW) const;

	virtual void screen_set_keep_on(bool p_enable) override;
	virtual bool screen_is_kept_on() const override;

	virtual Vector<DisplayServer::WindowID> get_window_list() const override;

	virtual WindowID create_sub_window(WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect = Rect2i(), bool p_exclusive = false, WindowID p_transient_parent = INVALID_WINDOW_ID) override;
	virtual void show_window(WindowID p_id) override;
	virtual void delete_sub_window(WindowID p_id) override;

	virtual WindowID window_get_active_popup() const override;
	virtual void window_set_popup_safe_rect(WindowID p_window, const Rect2i &p_rect) override;
	virtual Rect2i window_get_popup_safe_rect(WindowID p_window) const override;

	virtual int64_t window_get_native_handle(HandleType p_handle_type, WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual WindowID get_window_at_screen_position(const Point2i &p_position) const override;

	virtual void window_attach_instance_id(ObjectID p_instance, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual ObjectID window_get_attached_instance_id(WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual void window_set_title(const String &p_title, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual void window_set_mouse_passthrough(const Vector<Vector2> &p_region, WindowID p_window_id = MAIN_WINDOW_ID) override;

	virtual void window_set_rect_changed_callback(const Callable &p_callable, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual void window_set_window_event_callback(const Callable &p_callable, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual void window_set_input_event_callback(const Callable &p_callable, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual void window_set_input_text_callback(const Callable &p_callable, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual void window_set_drop_files_callback(const Callable &p_callable, WindowID p_window_id = MAIN_WINDOW_ID) override;

	virtual int window_get_current_screen(WindowID p_window_id = MAIN_WINDOW_ID) const override;
	virtual void window_set_current_screen(int p_screen, WindowID p_window_id = MAIN_WINDOW_ID) override;

	virtual Point2i window_get_position(WindowID p_window_id = MAIN_WINDOW_ID) const override;
	virtual Point2i window_get_position_with_decorations(WindowID p_window_id = MAIN_WINDOW_ID) const override;
	virtual void window_set_position(const Point2i &p_position, WindowID p_window_id = MAIN_WINDOW_ID) override;

	virtual void window_set_max_size(const Size2i p_size, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual Size2i window_get_max_size(WindowID p_window_id = MAIN_WINDOW_ID) const override;
	virtual void gl_window_make_current(DisplayServer::WindowID p_window_id) override;

	virtual void window_set_transient(WindowID p_window_id, WindowID p_parent) override;

	virtual void window_set_min_size(const Size2i p_size, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual Size2i window_get_min_size(WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual void window_set_size(const Size2i p_size, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual Size2i window_get_size(WindowID p_window_id = MAIN_WINDOW_ID) const override;
	virtual Size2i window_get_size_with_decorations(WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual void window_set_mode(WindowMode p_mode, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual WindowMode window_get_mode(WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual bool window_is_maximize_allowed(WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual void window_set_flag(WindowFlags p_flag, bool p_enabled, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual bool window_get_flag(WindowFlags p_flag, WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual void window_request_attention(WindowID p_window_id = MAIN_WINDOW_ID) override;

	virtual void window_move_to_foreground(WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual bool window_is_focused(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual bool window_can_draw(WindowID p_window_id = MAIN_WINDOW_ID) const override;

	virtual bool can_any_window_draw() const override;

	// virtual Point2i ime_get_selection() const override;
	// virtual String ime_get_text() const override;

	virtual void window_set_vsync_mode(DisplayServer::VSyncMode p_vsync_mode, WindowID p_window_id = MAIN_WINDOW_ID) override;
	virtual DisplayServer::VSyncMode window_get_vsync_mode(WindowID p_window_id) const override;

	virtual void process_events() override;

	virtual void release_rendering_thread() override;
	virtual void swap_buffers() override;

	virtual void set_context(Context p_context) override;

	virtual bool is_window_transparency_available() const override;

	virtual void mouse_set_mode(MouseMode p_mode) override;
	virtual MouseMode mouse_get_mode() const override;
	virtual void mouse_set_mode_override(MouseMode p_mode) override;
	virtual MouseMode mouse_get_mode_override() const override;
	virtual void mouse_set_mode_override_enabled(bool p_override_enabled) override;
	virtual bool mouse_is_mode_override_enabled() const override;

	virtual Point2i mouse_get_position() const override;
	virtual BitField<MouseButtonMask> mouse_get_button_state() const override;

	virtual void cursor_set_shape(CursorShape p_shape) override;
	virtual CursorShape cursor_get_shape() const override;

	static DisplayServer *create_func(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error);
	static Vector<String> get_rendering_drivers_func();

	static void register_qnx_driver();

	DisplayServerQnx(); // needed for gdextension
	DisplayServerQnx(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error);
	~DisplayServerQnx();

	static void _send_window_event(const WindowData &p_wd, DisplayServer::WindowEvent p_event);
	static void _dispatch_input_events(const Ref<InputEvent> &p_event);
	void _dispatch_input_event(const Ref<InputEvent> &p_event);
	void _set_input_focus(WindowID p_window_id);

protected:
	static void _bind_methods();
};

#endif // QNX_ENABLED
