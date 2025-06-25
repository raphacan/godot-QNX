/**************************************************************************/
/*  api.cpp                                                               */
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

#include "api.h"

#if defined(QNX_ENABLED)

#include "display_server_qnx.h"

#else // QNX_ENABLED

#include "servers/display_server.h"

// Dummy implementation for non-qnx platforms
class DisplayServerQnx : public DisplayServer {
	GDCLASS(DisplayServerQnx, DisplayServer)
public:
	uint64_t getScreenContext(){ return 0U;} 

	DisplayServerQnx(){}  // needed for gdextension
	~DisplayServerQnx(){}  

protected:
static void _bind_methods(){
	ClassDB::bind_method(D_METHOD("getScreenContext"), &DisplayServerQnx::getScreenContext);
}
};

#endif // QNX_ENABLED

void register_qnx_api() {
	GDREGISTER_ABSTRACT_CLASS(DisplayServerQnx);
#if defined(QNX_ENABLED)
	Engine::get_singleton()->add_singleton(Engine::Singleton("DisplayServerQnx", DisplayServerQnx::get_singleton(), "DisplayServerQnx"));
#endif
}

void unregister_qnx_api() {
}




