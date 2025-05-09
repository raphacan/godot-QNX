/**************************************************************************/
/*  rendering_context_driver_vulkan_x11.cpp                               */
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

#ifdef VULKAN_ENABLED

#include "rendering_context_driver_vulkan_screen.h"

#include "drivers/vulkan/godot_vulkan.h"



const char *RenderingContextDriverVulkanScreen::_get_platform_surface_extension() const {
	return VK_QNX_SCREEN_SURFACE_EXTENSION_NAME;
}

RenderingContextDriver::SurfaceID RenderingContextDriverVulkanScreen::surface_create(const void *p_platform_data) {
	VkScreenSurfaceCreateInfoQNX create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.context = m_screen_context;
	create_info.window  = m_screen_window;

	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	VkResult err = vkCreateScreenSurfaceQNX(instance_get(), &create_info, NULL, &vk_surface);
	ERR_FAIL_COND_V(err != VK_SUCCESS, SurfaceID());

	int size[2];
    int rc = screen_get_window_property_iv(m_screen_window, SCREEN_PROPERTY_SIZE, size);

	Surface *surface = memnew(Surface);
	surface->vk_surface = vk_surface;
	surface->width = size[0];
	surface->height = size[1];
	return SurfaceID(surface);

	// const WindowPlatformData *wpd = (const WindowPlatformData *)(p_platform_data);
 
	// VkXlibSurfaceCreateInfoKHR create_info = {};
	// create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	// create_info.dpy = wpd->display;
	// create_info.window = wpd->window;
 
	// VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	// VkResult err = vkCreateXlibSurfaceKHR(instance_get(), &create_info, get_allocation_callbacks(VK_OBJECT_TYPE_SURFACE_KHR), &vk_surface);
	// ERR_FAIL_COND_V(err != VK_SUCCESS, SurfaceID());
 
	// Surface *surface = memnew(Surface);
	// surface->vk_surface = vk_surface;
	// return SurfaceID(surface);
}

RenderingContextDriverVulkanScreen::RenderingContextDriverVulkanScreen() {
}

RenderingContextDriverVulkanScreen::~RenderingContextDriverVulkanScreen() {
}

Error RenderingContextDriverVulkanScreen::initialize() {
	Error err = OK;

	// base class initialization
	err = RenderingContextDriverVulkan::initialize();
	ERR_FAIL_COND_V(err != OK, err);

	// Screen specific initialization
	int rc = screen_create_context(&m_screen_context, 0);
	if (0 != rc)
	{
		// cannot create screen context
		err = FAILED;
		ERR_PRINT("screen_create_context() FAILED");
	}

	ERR_FAIL_COND_V(err != OK, err);

	rc = screen_create_window(&m_screen_window, m_screen_context);
	if (0 != rc)
	{
		// cannot create screen window
		err = FAILED;
		ERR_PRINT("screen_create_window() FAILED");
	}
    
	// screen attributes
    const int screenUsage   = SCREEN_USAGE_VULKAN;
    const int screenSize[2] = {1920, 1080};// {640, 480};

    rc = screen_set_window_property_iv(m_screen_window, SCREEN_PROPERTY_USAGE, &screenUsage);
    if (0 != rc)
    {
        ERR_PRINT("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) FAILED");
        err = FAILED;
    }

	int initialScreenSize[2];
	rc = screen_get_window_property_iv(m_screen_window, SCREEN_PROPERTY_SIZE, initialScreenSize);
    if (0 != rc)
    {
        ERR_PRINT("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE) FAILED");
        err = FAILED;
    }

    rc = screen_set_window_property_iv(m_screen_window, SCREEN_PROPERTY_SIZE, screenSize);
    if (0 != rc)
    {
        ERR_PRINT("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) FAILED");
        err = FAILED;
    }

	ERR_FAIL_COND_V(err != OK, err);

	return err;
}

#endif // VULKAN_ENABLED
