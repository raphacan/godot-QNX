/**************************************************************************/
/*  egl_manager.cpp                                                       */
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

#include "gl_manager_qnx_egl.h"

#include "drivers/gles3/rasterizer_gles3.h"

// #ifdef EGL_ENABLED // this seems to pull some dependencies to GLAD
#ifdef QNX_ENABLED //


// Creates and caches a GLDisplay. Returns -1 on error.
int GLManagerEGL_Screen::_get_gldisplay_id() {

	// Assuming single display for QNX
	if (displays.size() > 0)
	{
		return 0;
	}

	// We didn't find any, so we'll have to create one, along with its own
	// EGLDisplay and EGLContext.
	GLDisplay new_gldisplay;
	// new_gldisplay.display = p_display;

    int res = screen_create_context(&m_screenContext, SCREEN_BUFFER_PROVIDER_CONTEXT);
    if (0 != res)
    {
        ERR_PRINT("GL EGL screen_create_context() FAILED");
        return -1;
    }

    // screen attributes
    const int screenFormat  = SCREEN_FORMAT_RGBA8888;
    const int screenUsage   = SCREEN_USAGE_READ | SCREEN_USAGE_WRITE | SCREEN_USAGE_OPENGL_ES3;
    const int screenSize[2] = {1920, 1080};// {640, 480};
    const int windowBuffers = 2;

    res = screen_create_window(&m_screenWindow, m_screenContext);
    if (0 != res)
    {
        ERR_PRINT("screen_create_window() FAILED");
        return -1;
    }

    res = screen_set_window_property_iv(m_screenWindow, SCREEN_PROPERTY_FORMAT, &screenFormat);
    if (0 != res)
    {
        ERR_PRINT("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT) FAILED");
        return -1;
    }

    res = screen_set_window_property_iv(m_screenWindow, SCREEN_PROPERTY_USAGE, &screenUsage);
    if (0 != res)
    {
        ERR_PRINT("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) FAILED");
        return -1;
    }

    res = screen_set_window_property_iv(m_screenWindow, SCREEN_PROPERTY_SIZE, screenSize);
    if (0 != res)
    {
        ERR_PRINT("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) FAILED");
        return -1;
    }

    res = screen_create_window_buffers(m_screenWindow, windowBuffers);
    if (0 != res)
    {
        ERR_PRINT("screen_create_window_buffers() FAILED");
        return -1;
    }

	/// Further EGL
    new_gldisplay.egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == new_gldisplay.egl_display)
    {
        ERR_PRINT("eglGetDisplay() FAILED");
        return -1;
    }

	ERR_FAIL_COND_V(eglGetError() != EGL_SUCCESS, -1);

	ERR_FAIL_COND_V_MSG(new_gldisplay.egl_display == EGL_NO_DISPLAY, -1, "Can't create an EGL display.");

	if (!eglInitialize(new_gldisplay.egl_display, nullptr, nullptr)) {
		ERR_FAIL_V_MSG(-1, "Can't initialize an EGL display.");
	}


	if (!eglBindAPI(_get_platform_api_enum())) {
		ERR_FAIL_V_MSG(-1, "OpenGL not supported.");
	}

	Error err = _gldisplay_create_context(new_gldisplay);

	if (err != OK) {
		eglTerminate(new_gldisplay.egl_display);
		ERR_FAIL_V(-1);
	}

	displays.push_back(new_gldisplay);

	// Return the new GLDisplay's ID.
	return displays.size() - 1;
}


Error GLManagerEGL_Screen::_gldisplay_create_context(GLDisplay &p_gldisplay) {
	EGLint attribs[] = {
		EGL_RED_SIZE,
		1,
		EGL_BLUE_SIZE,
		1,
		EGL_GREEN_SIZE,
		1,
		EGL_DEPTH_SIZE,
		24,
		EGL_NONE,
	};

	EGLint attribs_layered[] = {
		EGL_RED_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_ALPHA_SIZE,
		8,
		EGL_DEPTH_SIZE,
		24,
        EGL_STENCIL_SIZE,
       	8,
        // attributes.push_back(EGL_SAMPLE_BUFFERS);
        // attributes.push_back(_traits->sampleBuffers);
        EGL_SAMPLES,
        0,
		EGL_RENDERABLE_TYPE,	// QNX
        EGL_OPENGL_ES3_BIT,		// QNX
		EGL_NONE,
	};

	EGLint config_count = 0;

	if (true) {// (OS::get_singleton()->is_layered_allowed()) {
		eglChooseConfig(p_gldisplay.egl_display, attribs_layered, &p_gldisplay.egl_config, 1, &config_count);
	} else {
		eglChooseConfig(p_gldisplay.egl_display, attribs, &p_gldisplay.egl_config, 1, &config_count);
	}

	ERR_FAIL_COND_V(eglGetError() != EGL_SUCCESS, ERR_BUG);
	ERR_FAIL_COND_V(config_count == 0, ERR_UNCONFIGURED);

	Vector<EGLint> context_attribs = _get_platform_context_attribs();
	p_gldisplay.egl_context = eglCreateContext(p_gldisplay.egl_display, p_gldisplay.egl_config, EGL_NO_CONTEXT, (context_attribs.size() > 0) ? context_attribs.ptr() : nullptr);
	ERR_FAIL_COND_V_MSG(p_gldisplay.egl_context == EGL_NO_CONTEXT, ERR_CANT_CREATE, vformat("Can't create an EGL context. Error code: %d", eglGetError()));

	return OK;
}

Error GLManagerEGL_Screen::open_display() {
	int gldisplay_id = _get_gldisplay_id();
	if (gldisplay_id < 0) {
		return ERR_CANT_CREATE;
	} else {
		return OK;
	}
}

int GLManagerEGL_Screen::display_get_native_visual_id(void *p_display) {
	int gldisplay_id = _get_gldisplay_id();
	ERR_FAIL_COND_V(gldisplay_id < 0, ERR_CANT_CREATE);

	GLDisplay gldisplay = displays[gldisplay_id];

	EGLint native_visual_id = -1;

	if (!eglGetConfigAttrib(gldisplay.egl_display, gldisplay.egl_config, EGL_NATIVE_VISUAL_ID, &native_visual_id)) {
		ERR_FAIL_V(-1);
	}

	return native_visual_id;
}

Error GLManagerEGL_Screen::window_create(DisplayServer::WindowID p_window_id, void *p_display, void *p_native_window, int p_width, int p_height) {
	int gldisplay_id = _get_gldisplay_id();
	ERR_FAIL_COND_V(gldisplay_id < 0, ERR_CANT_CREATE);

	GLDisplay &gldisplay = displays[gldisplay_id];

	// In order to ensure a fast lookup, make sure we got enough elements in the
	// windows local vector to use the window id as an index.
	if (p_window_id >= (int)windows.size()) {
		windows.resize(p_window_id + 1);
	}

	GLWindow &glwindow = windows[p_window_id];
	glwindow.gldisplay_id = gldisplay_id;

	// Vector<EGLAttrib> egl_attribs;
	// egl_attribs.push_back(EGL_RENDER_BUFFER);
	// egl_attribs.push_back(EGL_BACK_BUFFER);
	// egl_attribs.push_back(EGL_NONE);
	const EGLint surfaceAttributes[] = {EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE};

	glwindow.egl_surface = eglCreateWindowSurface(gldisplay.egl_display, gldisplay.egl_config, m_screenWindow, surfaceAttributes);

	if (glwindow.egl_surface == EGL_NO_SURFACE) {
		return ERR_CANT_CREATE;
	}

	glwindow.initialized = true;

	window_make_current(p_window_id);

	return OK;
}

void GLManagerEGL_Screen::window_destroy(DisplayServer::WindowID p_window_id) {
	ERR_FAIL_INDEX(p_window_id, (int)windows.size());

	GLWindow &glwindow = windows[p_window_id];

	if (!glwindow.initialized) {
		return;
	} 

	glwindow.initialized = false;

	ERR_FAIL_INDEX(glwindow.gldisplay_id, (int)displays.size());
	GLDisplay &gldisplay = displays[glwindow.gldisplay_id];

	if (glwindow.egl_surface != EGL_NO_SURFACE) {
		eglDestroySurface(gldisplay.egl_display, glwindow.egl_surface);
		glwindow.egl_surface = nullptr;
	}
}

void GLManagerEGL_Screen::release_current() {
	if (!current_window) {
		return;
	}

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglMakeCurrent(current_display.egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void GLManagerEGL_Screen::swap_buffers() {		
	if (!current_window) {
		return;
	}

	if (!current_window->initialized) {
		WARN_PRINT("Current OpenGL window is uninitialized!");
		return;
	}

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglSwapBuffers(current_display.egl_display, current_window->egl_surface);
}

void GLManagerEGL_Screen::window_make_current(DisplayServer::WindowID p_window_id) {
	if (p_window_id == DisplayServer::INVALID_WINDOW_ID) {
		return;
	}

	GLWindow &glwindow = windows[p_window_id];

	if (&glwindow == current_window || !glwindow.initialized) {
		return;
	}

	current_window = &glwindow;

	GLDisplay &current_display = displays[current_window->gldisplay_id];

	eglMakeCurrent(current_display.egl_display, current_window->egl_surface, current_window->egl_surface, current_display.egl_context);
}

void GLManagerEGL_Screen::set_use_vsync(bool p_use) {
	// We need an active window to get a display to set the vsync.
	if (!current_window) {
		return;
	}

	GLDisplay &disp = displays[current_window->gldisplay_id];

	int swap_interval = p_use ? 1 : 0;

	if (!eglSwapInterval(disp.egl_display, swap_interval)) {
		WARN_PRINT("Could not set V-Sync mode.");
	}

	use_vsync = p_use;
}

bool GLManagerEGL_Screen::is_using_vsync() const {
	return use_vsync;
}

EGLContext GLManagerEGL_Screen::get_context(DisplayServer::WindowID p_window_id) {
	GLWindow &glwindow = windows[p_window_id];

	if (!glwindow.initialized) {
		return EGL_NO_CONTEXT;
	}

	GLDisplay &display = displays[glwindow.gldisplay_id];

	return display.egl_context;
}

EGLDisplay GLManagerEGL_Screen::get_display(DisplayServer::WindowID p_window_id) {
	GLWindow &glwindow = windows[p_window_id];

	if (!glwindow.initialized) {
		return EGL_NO_CONTEXT;
	}

	GLDisplay &display = displays[glwindow.gldisplay_id];

	return display.egl_display;
}

EGLConfig GLManagerEGL_Screen::get_config(DisplayServer::WindowID p_window_id) {
	GLWindow &glwindow = windows[p_window_id];

	if (!glwindow.initialized) {
		return nullptr;
	}

	GLDisplay &display = displays[glwindow.gldisplay_id];

	return display.egl_config;
}

Error GLManagerEGL_Screen::initialize(void *p_native_display) {
	String client_extensions_string = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

	// If the above method fails, we don't support client extensions, so there's nothing to check.
	if (eglGetError() == EGL_SUCCESS) {
		print_verbose(vformat("Extensions: %s", client_extensions_string));
	}

	return OK;
}


GLManagerEGL_Screen::GLManagerEGL_Screen() {
}

GLManagerEGL_Screen::~GLManagerEGL_Screen() {
	for (unsigned int i = 0; i < displays.size(); i++) {
		eglTerminate(displays[i].egl_display);
	}
}

#endif // EGL_ENABLED
