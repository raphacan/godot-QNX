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

#ifdef VULKAN_ENABLED
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif

#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#include <EGL/egl.h>
#endif


DisplayServerQnx *DisplayServerQnx::get_singleton() {
	return static_cast<DisplayServerQnx *>(DisplayServer::get_singleton());
}

screen_context_t DisplayServerQnx::getScreenContext()
{
	screen_context_t l_context = nullptr;
	RenderingContextDriverVulkanScreen* l_vulkanContext = dynamic_cast<RenderingContextDriverVulkanScreen*>(rendering_context);
	if (egl_manager)
	{
		l_context = egl_manager->getScreenContext();
	}
	else if (l_vulkanContext)
	{
		l_context = l_vulkanContext->getScreenContext();
	}
	else
	{
		WARN_PRINT(vformat("DisplayServerQnx::getScreenContext() Could not get a screen context"));	
	}
	return l_context;
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
		case FEATURE_CLIPBOARD:
		case FEATURE_CURSOR_SHAPE:
		case FEATURE_CUSTOM_CURSOR_SHAPE:
		case FEATURE_WINDOW_TRANSPARENCY:
		case FEATURE_HIDPI:
		case FEATURE_TOUCHSCREEN:
		case FEATURE_SWAP_BUFFERS:
		case FEATURE_KEEP_SCREEN_ON:
		case FEATURE_IME:
		case FEATURE_CLIPBOARD_PRIMARY: {
			return true;
		} break;

		//case FEATURE_NATIVE_DIALOG:
		//case FEATURE_NATIVE_DIALOG_INPUT:
#ifdef DBUS_ENABLED
		case FEATURE_NATIVE_DIALOG_FILE:
		case FEATURE_NATIVE_DIALOG_FILE_EXTRA: {
			return true;
		} break;
#endif

#ifdef SPEECHD_ENABLED
		case FEATURE_TEXT_TO_SPEECH: {
			return true;
		} break;
#endif

		default: {
			return false;
		}
	}
}

String DisplayServerQnx::get_name() const {
	return "qnx";
}


int DisplayServerQnx::get_screen_count() const {
	return 1;
}

int DisplayServerQnx::get_primary_screen() const {
	return 0;
}

Point2i DisplayServerQnx::screen_get_position(int p_screen) const {
	return Point2i(0, 0);
}

Size2i DisplayServerQnx::screen_get_size(int p_screen) const {
	// return OS_QNX::get_singleton()->get_display_size();
	return Size2i(1920, 1080);// Size2i(640,480); // Rect2i(0, 0, display_size.width, display_size.height);
}


Rect2i DisplayServerQnx::screen_get_usable_rect(int p_screen) const {
	// Size2i display_size = OS_QNX::get_singleton()->get_display_size();
	return Rect2i(0,0,1920,1080);// Rect2i(0,0,640,480); // Rect2i(0, 0, display_size.width, display_size.height);
}


int DisplayServerQnx::screen_get_dpi(int p_screen) const {

	// SCREEN_PROPERTY_DPI


	return 96;
}

float DisplayServerQnx::screen_get_scale(int p_screen) const {

	return 1.0;
}

float DisplayServerQnx::screen_get_refresh_rate(int p_screen) const {

	return 1.0;
}

bool DisplayServerQnx::is_touchscreen_available() const {
	return true;
}

void DisplayServerQnx::screen_set_keep_on(bool p_enable) {

}

bool DisplayServerQnx::screen_is_kept_on() const {
	return true;
}

Vector<DisplayServer::WindowID> DisplayServerQnx::get_window_list() const {
	Vector<int> ret;
	ret.push_back(0);

	return ret;
}

int64_t DisplayServerQnx::window_get_native_handle(HandleType p_handle_type, WindowID p_window) const {	
	switch (p_handle_type) {
		case DISPLAY_HANDLE: {
			return 0;
		} break;

		case WINDOW_HANDLE: {
			return 0;
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

int DisplayServerQnx::window_get_current_screen(DisplayServer::WindowID p_window_id) const {
	// Standard Wayland APIs don't support getting the screen of a window.
	return 0;
}

void DisplayServerQnx::window_set_current_screen(int p_screen, DisplayServer::WindowID p_window_id) {
	// Standard Wayland APIs don't support setting the screen of a window.
}

Point2i DisplayServerQnx::window_get_position(DisplayServer::WindowID p_window_id) const {

	// We can't know the position of toplevels with the standard protocol.
	return Point2i();
}

void DisplayServerQnx::gl_window_make_current(DisplayServer::WindowID p_window_id) {
#ifdef GLES3_ENABLED
	if (egl_manager) {
		egl_manager->window_make_current(MAIN_WINDOW_ID);
	}
#endif
}

void DisplayServerQnx::window_set_transient(DisplayServer::WindowID p_window_id, WindowID p_parent) {
	// Currently unsupported.
}


bool DisplayServerQnx::window_can_draw(DisplayServer::WindowID p_window_id) const {
	return true;
}

bool DisplayServerQnx::can_any_window_draw() const {
	return true;
}

void DisplayServerQnx::window_set_ime_active(const bool p_active, DisplayServer::WindowID p_window_id) {

}

void DisplayServerQnx::window_set_ime_position(const Point2i &p_pos, DisplayServer::WindowID p_window_id) {

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

bool isTouchEvent(std::int32_t f_propertyType)
{
    return (
        (SCREEN_EVENT_MTOUCH_PRETOUCH == f_propertyType) || (SCREEN_EVENT_MTOUCH_TOUCH == f_propertyType) ||
        (SCREEN_EVENT_MTOUCH_MOVE == f_propertyType) || (SCREEN_EVENT_MTOUCH_RELEASE == f_propertyType));
}

void DisplayServerQnx::process_events() {

    if (nullptr == m_event)
    {
        if (0 != screen_create_event(&m_event))
        {
            return;
        }
    }

	screen_context_t l_context = nullptr;
	RenderingContextDriverVulkanScreen* l_vulkanContext = dynamic_cast<RenderingContextDriverVulkanScreen*>(rendering_context);
	if (egl_manager)
	{
		l_context = egl_manager->getScreenContext();
	}
	else if (l_vulkanContext)
	{
		l_context = l_vulkanContext->getScreenContext();
	}
	else
	{
		WARN_PRINT(vformat("Could not get a screen context"));	
		return;
	}

    while (0 == screen_get_event(l_context, m_event, 0))
    {
        std::int32_t l_propertyType = 0;
        const auto   l_result       = screen_get_event_property_iv(m_event, SCREEN_PROPERTY_TYPE, &l_propertyType);
        if ((0 != l_result) || (SCREEN_EVENT_NONE == l_propertyType))
        {
            break;
        }

        std::int32_t touch_position[2] = {0, 0};
        std::int32_t touch_id{0};
        if (isTouchEvent(l_propertyType))
        {
            screen_get_event_property_iv(m_event, SCREEN_PROPERTY_POSITION, touch_position);
            screen_get_event_property_iv(m_event, SCREEN_PROPERTY_TOUCH_ID, &touch_id);
        }

        switch (l_propertyType)
        {
			case SCREEN_EVENT_MTOUCH_TOUCH:
			{
				TouchPos tp;
				tp.id  = touch_id;
				tp.pos = Vector2(touch_position[0], touch_position[1]);
				// tp.pressure = 0;
				// tp.tilt;

				touch.push_back(tp);

				Ref<InputEventScreenTouch> ev;
				ev.instantiate();
				ev->set_index(tp.id);
				ev->set_pressed(true);
				ev->set_position(tp.pos);
				Input::get_singleton()->parse_input_event(ev);							
				break;
			}
			case SCREEN_EVENT_MTOUCH_PRETOUCH:
			{	
				break;
			}
			case SCREEN_EVENT_MTOUCH_MOVE:
			{
				for (int i = 0; i < touch.size(); i++) {

					if (touch[i].id != touch_id) {
						continue;
					}

					if (touch[i].pos == Vector2(touch_position[0], touch_position[1])) {
						continue; // Don't move unnecessarily.
					}

					Ref<InputEventScreenDrag> ev;
					ev.instantiate();
					ev->set_index(touch_id);
					ev->set_position(Vector2(touch_position[0], touch_position[1]));
					ev->set_relative(Vector2(touch_position[0], touch_position[1]) - touch[i].pos);
					ev->set_relative_screen_position(ev->get_relative());
					// ev->set_pressure(p_points[idx].pressure);
					// ev->set_tilt(p_points[idx].tilt);
					Input::get_singleton()->parse_input_event(ev);
					touch.write[i].pos = Vector2(touch_position[0], touch_position[1]);
				}

				break;
			}
			case SCREEN_EVENT_MTOUCH_RELEASE:
			{
				for (int i = 0; i < touch.size(); i++) {
					if (touch[i].id == touch_id) {
						Ref<InputEventScreenTouch> ev;
						ev.instantiate();
						ev->set_index(touch[i].id);
						ev->set_pressed(false);
						ev->set_position(touch[i].pos);
						Input::get_singleton()->parse_input_event(ev);
						touch.remove_at(i);
						break;
					}
				}

				break;
			}
			default:
			{
				WARN_PRINT(vformat("DisplayServerQnx::process_events() received unhandled event of type %d", l_propertyType));	
				break;
			}
		}
    } // while

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
	drivers.push_back("opengl3_es");
#endif

	return drivers;
}


void DisplayServerQnx::mouse_set_mode(MouseMode p_mode) {
	//WARN_PRINT("Mouse is not supported by this display server.");
}

DisplayServer::MouseMode DisplayServerQnx::mouse_get_mode() const {
	return MOUSE_MODE_VISIBLE;
}

void DisplayServerQnx::mouse_set_mode_override(MouseMode p_mode) {
	//WARN_PRINT("Mouse is not supported by this display server.");
}

DisplayServer::MouseMode DisplayServerQnx::mouse_get_mode_override() const {
	return MOUSE_MODE_VISIBLE;
}

void DisplayServerQnx::mouse_set_mode_override_enabled(bool p_override_enabled) {
	//ARN_PRINT("Mouse is not supported by this display server.");
}

bool DisplayServerQnx::mouse_is_mode_override_enabled() const {
	return false;
}


Point2i DisplayServerQnx::mouse_get_position() const {
	return Point2i(); //ERR_FAIL_V_MSG(Point2i(), "Mouse is not supported by this display server.");
}

BitField<MouseButtonMask> DisplayServerQnx::mouse_get_button_state() const {
	return 0; //ERR_FAIL_V_MSG(0, "Mouse is not supported by this display server.");
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

DisplayServerQnx::DisplayServerQnx(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error) {	
	r_error = ERR_UNAVAILABLE;

	// Input.
	Input::get_singleton()->set_event_dispatch_function(_dispatch_input_events);

	native_menu = memnew(NativeMenu);

	rendering_driver = p_rendering_driver;

	bool driver_found = false;
	String executable_name = OS::get_singleton()->get_executable_path().get_file();

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
#if 0 // defined(GLES3_ENABLED)
			bool fallback_to_opengl3 = GLOBAL_GET("rendering/rendering_device/fallback_to_opengl3");
			if (fallback_to_opengl3 && rendering_driver != "opengl3") {
				WARN_PRINT("Your video card drivers seem not to support the required Vulkan version, switching to OpenGL 3.");
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
	if (rendering_driver == "opengl3" || rendering_driver == "opengl3_es") {

		if (rendering_driver == "opengl3") {
			egl_manager = memnew(GLManagerEGL_Screen);

			if (egl_manager->initialize() != OK || egl_manager->open_display() != OK) {
				memdelete(egl_manager);
				egl_manager = nullptr;

				bool fallback = GLOBAL_GET("rendering/gl_compatibility/fallback_to_gles");
				if (fallback) {
					WARN_PRINT("Your video card drivers seem not to support the required OpenGL version, switching to OpenGLES.");
					rendering_driver = "opengl3_es";
					OS::get_singleton()->set_current_rendering_driver_name(rendering_driver);
				} else {
					r_error = ERR_UNAVAILABLE;

					OS::get_singleton()->alert(
							vformat("Your video card drivers seem not to support the required OpenGL 3.3 version.\n\n"
									"If possible, consider updating your video card drivers or using the Vulkan driver.\n\n"
									"You can enable the Vulkan driver by starting the engine from the\n"
									"command line with the command:\n\n    \"%s\" --rendering-driver vulkan\n\n"
									"If you recently updated your video card drivers, try rebooting.",
									executable_name),
							"Unable to initialize OpenGL video driver");

					ERR_FAIL_MSG("Could not initialize OpenGL.");
				}
			} else {
				RasterizerGLES3::make_current(true);
				driver_found = true;
			}
		}

		if (rendering_driver == "opengl3_es") {
			egl_manager = memnew(GLManagerEGL_Screen);

			if (egl_manager->initialize() != OK || egl_manager->open_display() != OK) {
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

			RasterizerGLES3::make_current(false);
			driver_found = true;
		}
	}
#endif // GLES3_ENABLED

	if (!driver_found) {
		r_error = ERR_UNAVAILABLE;
		ERR_FAIL_MSG("Video driver not found.");
	}

#if defined(RD_ENABLED)
	if (rendering_context) {
			union {
#ifdef VULKAN_ENABLED
				RenderingContextDriverVulkanScreen::WindowPlatformData vulkan;
#endif
			} wpd;
#ifdef VULKAN_ENABLED
			if (rendering_driver == "vulkan") {
				// vulkan.m_screen_context =  TODO ;
				// vulkan.m_screen_window =  TODO;
			}
#endif

		Error err = rendering_context->window_create(MAIN_WINDOW_ID, &wpd);
		// ERR_FAIL_COND_V_MSG(err != OK, INVALID_WINDOW_ID, vformat("Can't create a %s window", rendering_driver));
		ERR_FAIL_COND_MSG(err != OK, vformat("Can't create a %s window", rendering_driver));
	}
#endif // RD_ENABLED

	if (egl_manager) {
		egl_manager->window_create(MAIN_WINDOW_ID, nullptr, nullptr, 0, 0);
	}
	// _show_window();
	screen_set_keep_on(GLOBAL_GET("display/window/energy_saving/keep_screen_on"));


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
	// TODO: Multiwindow support.

	if (native_menu) {
		memdelete(native_menu);
	 	native_menu = nullptr;
	}

//  	if (main_window.visible) {
//  #ifdef VULKAN_ENABLED
//  		if (rendering_device) {
//  			rendering_device->screen_free(MAIN_WINDOW_ID);
//  		}
//  
//  		if (rendering_context) {
//  			rendering_context->window_destroy(MAIN_WINDOW_ID);
//  		}
//  #endif
//  
//  #ifdef GLES3_ENABLED
//  		if (egl_manager) {
//  			egl_manager->window_destroy(MAIN_WINDOW_ID);
//  		}
//  #endif
//  	}
//  


	// Destroy all drivers.
#ifdef RD_ENABLED
	if (rendering_device) {
		memdelete(rendering_device);
	}

	if (rendering_context) {
		memdelete(rendering_context);
	}
#endif

}

void DisplayServerQnx::register_qnx_driver() {
	register_create_function("qnx", create_func, get_rendering_drivers_func);
}



DisplayServer::WindowID  DisplayServerQnx::get_window_at_screen_position(const Point2i &p_position) const {
	return MAIN_WINDOW_ID;
}

void DisplayServerQnx::window_attach_instance_id(ObjectID p_instance, WindowID p_window_id) {
	//
}

ObjectID DisplayServerQnx::window_get_attached_instance_id(DisplayServer::WindowID p_window_id) const {
	return ObjectID();
}

void DisplayServerQnx::window_set_rect_changed_callback(Callable const&, int) {
	//
}
void DisplayServerQnx::window_set_window_event_callback(Callable const&, int) {
	//
}
void DisplayServerQnx::window_set_input_event_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	input_event_callback = p_callable;
}

void DisplayServerQnx::window_set_input_text_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	//
}
void DisplayServerQnx::window_set_drop_files_callback(Callable const&, int) {
	//
}
void DisplayServerQnx::window_set_title(String const&, int) {
	//
}
void DisplayServerQnx::window_set_mouse_passthrough(Vector<Vector2> const&, int) {
	//
}

Point2i DisplayServerQnx::window_get_position_with_decorations(DisplayServer::WindowID p_window_id) const {
	return Point2i();
}

void DisplayServerQnx::window_set_position(Vector2i const&, int) {
	// 
}

void DisplayServerQnx::window_set_max_size(const Size2i p_size, WindowID p_window_id)
{
	//
}

Size2i DisplayServerQnx::window_get_max_size(int) const {
	return Size2i();
}

void DisplayServerQnx::window_set_min_size(const Size2i p_size, WindowID p_window_id) {
	//
}

Size2i DisplayServerQnx::window_get_min_size(DisplayServer::WindowID p_window_id ) const  {
	return Size2i();
}

void DisplayServerQnx::window_set_size(const Size2i p_size, WindowID p_window_id) {
	//
}

Size2i DisplayServerQnx::window_get_size(DisplayServer::WindowID p_window_id ) const {
	//return OS_QNX::get_singleton()->get_display_size();
	return Size2i(1920, 1080);// Size2i(640,480);
}

Size2i DisplayServerQnx::window_get_size_with_decorations(DisplayServer::WindowID p_window_id) const {
	// return OS_QNX::get_singleton()->get_display_size();
	return Size2i(1920, 1080); // Size2i(640,480);
}

void DisplayServerQnx::window_set_mode(DisplayServer::WindowMode, int) {
	//
}

DisplayServer::WindowMode DisplayServerQnx::window_get_mode(DisplayServer::WindowID p_window_id ) const {
	return DisplayServer::WindowMode::WINDOW_MODE_FULLSCREEN;
}

bool DisplayServerQnx::window_is_maximize_allowed(DisplayServer::WindowID p_window_id ) const {
	return false;
}

void DisplayServerQnx::window_set_flag(DisplayServer::WindowFlags, bool, int) {
	//
}

bool DisplayServerQnx::window_get_flag(DisplayServer::WindowFlags, int) const {
	return false;
}

void DisplayServerQnx::window_request_attention(int) {
	//
}

void DisplayServerQnx::window_move_to_foreground(int) {
	//
}

bool DisplayServerQnx::window_is_focused(int) const {
	return true;
}

void DisplayServerQnx::_dispatch_input_events(const Ref<InputEvent> &p_event) {
	DisplayServerQnx::get_singleton()->send_input_event(p_event);
}

void DisplayServerQnx::send_input_event(const Ref<InputEvent> &p_event) const {
	_window_callback(input_event_callback, p_event);
}

void DisplayServerQnx::_window_callback(const Callable &p_callable, const Variant &p_arg, bool p_deferred) const {
	if (p_callable.is_valid()) {
		if (p_deferred) {
			p_callable.call_deferred(p_arg);
		} else {
			p_callable.call(p_arg);
		}
	}
}



#endif //QNX_ENABLED
