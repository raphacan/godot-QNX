/**************************************************************************/
/*  key_mapping_qnx_screen.h                                              */
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

#include "core/os/keyboard.h"

#include <sys/keycodes.h>

#define KEYCODE_MAX 0xFFFF

struct QnxGodotCodePair {
	int qnx_code = 0;
	Key godot_code = Key::NONE;
};

static QnxGodotCodePair qnx_godot_code_pairs[] = {
	{ KEYCODE_PAUSE, Key::PAUSE },
	{ KEYCODE_SCROLL_LOCK, Key::SCROLLLOCK },
	{ KEYCODE_PRINT, Key::PRINT },
	{ KEYCODE_SYSREQ, Key::PRINT },
	{ KEYCODE_BREAK, Key::PAUSE },
	{ KEYCODE_ESCAPE, Key::ESCAPE },
	{ KEYCODE_BACKSPACE, Key::BACKSPACE },
	{ KEYCODE_RETURN, Key::ENTER },
	{ KEYCODE_TAB, Key::TAB },
	{ KEYCODE_BACK_TAB, Key::BACKTAB },
	{ KEYCODE_CAPS_LOCK, Key::CAPSLOCK },
	{ KEYCODE_LEFT_SHIFT, Key::SHIFT },
	{ KEYCODE_RIGHT_SHIFT, Key::SHIFT },
	{ KEYCODE_LEFT_CTRL, Key::CTRL },
	{ KEYCODE_RIGHT_CTRL, Key::CTRL },
	{ KEYCODE_LEFT_ALT, Key::ALT },
	{ KEYCODE_RIGHT_ALT, Key::ALT },
	{ KEYCODE_MENU, Key::MENU },
	{ KEYCODE_LEFT_HYPER, Key::HYPER },
	{ KEYCODE_RIGHT_HYPER, Key::HYPER },
	{ KEYCODE_INSERT, Key::INSERT },
	{ KEYCODE_HOME, Key::HOME },
	{ KEYCODE_PG_UP, Key::PAGEUP },
	{ KEYCODE_DELETE, Key::KEY_DELETE },
	{ KEYCODE_END, Key::END },
	{ KEYCODE_PG_DOWN, Key::PAGEDOWN },
	{ KEYCODE_LEFT, Key::LEFT },
	{ KEYCODE_RIGHT, Key::RIGHT },
	{ KEYCODE_UP, Key::UP },
	{ KEYCODE_DOWN, Key::DOWN },
	{ KEYCODE_NUM_LOCK, Key::NUMLOCK },
	{ KEYCODE_KP_PLUS, Key::KP_ADD },
	{ KEYCODE_KP_MINUS, Key::KP_SUBTRACT },
	{ KEYCODE_KP_MULTIPLY, Key::KP_MULTIPLY },
	{ KEYCODE_KP_DIVIDE, Key::KP_DIVIDE },
	{ KEYCODE_KP_ENTER, Key::KP_ENTER },
	{ KEYCODE_KP_HOME, Key::HOME },
	{ KEYCODE_KP_UP, Key::UP },
	{ KEYCODE_KP_PG_UP, Key::PAGEUP },
	{ KEYCODE_KP_LEFT, Key::LEFT },
	{ KEYCODE_KP_FIVE, Key::KP_5 },
	{ KEYCODE_KP_RIGHT, Key::RIGHT },
	{ KEYCODE_KP_END, Key::END },
	{ KEYCODE_KP_DOWN, Key::DOWN },
	{ KEYCODE_KP_PG_DOWN, Key::PAGEDOWN },
	{ KEYCODE_KP_INSERT, Key::INSERT },
	{ KEYCODE_KP_DELETE, Key::KEY_DELETE },
	{ KEYCODE_F1, Key::F1 },
	{ KEYCODE_F2, Key::F2 },
	{ KEYCODE_F3, Key::F3 },
	{ KEYCODE_F4, Key::F4 },
	{ KEYCODE_F5, Key::F5 },
	{ KEYCODE_F6, Key::F6 },
	{ KEYCODE_F7, Key::F7 },
	{ KEYCODE_F8, Key::F8 },
	{ KEYCODE_F9, Key::F9 },
	{ KEYCODE_F10, Key::F10 },
	{ KEYCODE_F11, Key::F11 },
	{ KEYCODE_F12, Key::F12 },
	{ KEYCODE_PLAY, Key::MEDIAPLAY },
	{ KEYCODE_RECORD, Key::MEDIARECORD },
	{ KEYCODE_SCAN_NEXT, Key::MEDIANEXT },
	{ KEYCODE_SCAN_PREVIOUS, Key::MEDIAPREVIOUS },
	{ KEYCODE_SLEEP, Key::STANDBY },
	{ KEYCODE_STOP, Key::MEDIASTOP },
	{ KEYCODE_MUTE, Key::VOLUMEMUTE },
	{ KEYCODE_VOLUME_UP, Key::VOLUMEUP },
	{ KEYCODE_VOLUME_DOWN, Key::VOLUMEDOWN },
	{ KEYCODE_SPACE, Key::SPACE },
	{ KEYCODE_EXCLAM, Key::EXCLAM },
	{ KEYCODE_NUMBER, Key::NUMBERSIGN },
	{ KEYCODE_DOLLAR, Key::DOLLAR },
	{ KEYCODE_APOSTROPHE, Key::APOSTROPHE },
	{ KEYCODE_LEFT_PAREN, Key::BRACELEFT },
	{ KEYCODE_RIGHT_PAREN, Key::BRACERIGHT },
	{ KEYCODE_ASTERISK, Key::ASTERISK },
	{ KEYCODE_PLUS, Key::PLUS },
	{ KEYCODE_COMMA, Key::COMMA },
	{ KEYCODE_MINUS, Key::MINUS },
	{ KEYCODE_PERIOD, Key::PERIOD },
	{ KEYCODE_SLASH, Key::SLASH },
	{ KEYCODE_ZERO, Key::KEY_0 },
	{ KEYCODE_ONE, Key::KEY_1 },
	{ KEYCODE_TWO, Key::KEY_2 },
	{ KEYCODE_THREE, Key::KEY_3 },
	{ KEYCODE_FOUR, Key::KEY_4 },
	{ KEYCODE_FIVE, Key::KEY_5 },
	{ KEYCODE_SIX, Key::KEY_6 },
	{ KEYCODE_SEVEN, Key::KEY_7 },
	{ KEYCODE_EIGHT, Key::KEY_8 },
	{ KEYCODE_NINE, Key::KEY_9 },
	{ KEYCODE_COLON, Key::COLON },
	{ KEYCODE_SEMICOLON, Key::SEMICOLON },
	{ KEYCODE_LESS_THAN, Key::LESS },
	{ KEYCODE_EQUAL, Key::EQUAL },
	{ KEYCODE_GREATER_THAN, Key::GREATER },
	{ KEYCODE_QUESTION, Key::QUESTION },
	{ KEYCODE_AT, Key::AT },
	{ KEYCODE_CAPITAL_A, Key::A },
	{ KEYCODE_CAPITAL_B, Key::B },
	{ KEYCODE_CAPITAL_C, Key::C },
	{ KEYCODE_CAPITAL_D, Key::D },
	{ KEYCODE_CAPITAL_E, Key::E },
	{ KEYCODE_CAPITAL_F, Key::F },
	{ KEYCODE_CAPITAL_G, Key::G },
	{ KEYCODE_CAPITAL_H, Key::H },
	{ KEYCODE_CAPITAL_I, Key::I },
	{ KEYCODE_CAPITAL_J, Key::J },
	{ KEYCODE_CAPITAL_K, Key::K },
	{ KEYCODE_CAPITAL_L, Key::L },
	{ KEYCODE_CAPITAL_M, Key::M },
	{ KEYCODE_CAPITAL_N, Key::N },
	{ KEYCODE_CAPITAL_O, Key::O },
	{ KEYCODE_CAPITAL_P, Key::P },
	{ KEYCODE_CAPITAL_Q, Key::Q },
	{ KEYCODE_CAPITAL_R, Key::R },
	{ KEYCODE_CAPITAL_S, Key::S },
	{ KEYCODE_CAPITAL_T, Key::T },
	{ KEYCODE_CAPITAL_U, Key::U },
	{ KEYCODE_CAPITAL_V, Key::V },
	{ KEYCODE_CAPITAL_W, Key::W },
	{ KEYCODE_CAPITAL_X, Key::X },
	{ KEYCODE_CAPITAL_Y, Key::Y },
	{ KEYCODE_CAPITAL_Z, Key::Z },
	{ KEYCODE_A, Key::A },
	{ KEYCODE_B, Key::B },
	{ KEYCODE_C, Key::C },
	{ KEYCODE_D, Key::D },
	{ KEYCODE_E, Key::E },
	{ KEYCODE_F, Key::F },
	{ KEYCODE_G, Key::G },
	{ KEYCODE_H, Key::H },
	{ KEYCODE_I, Key::I },
	{ KEYCODE_J, Key::J },
	{ KEYCODE_K, Key::K },
	{ KEYCODE_L, Key::L },
	{ KEYCODE_M, Key::M },
	{ KEYCODE_N, Key::N },
	{ KEYCODE_O, Key::O },
	{ KEYCODE_P, Key::P },
	{ KEYCODE_Q, Key::Q },
	{ KEYCODE_R, Key::R },
	{ KEYCODE_S, Key::S },
	{ KEYCODE_T, Key::T },
	{ KEYCODE_U, Key::U },
	{ KEYCODE_V, Key::V },
	{ KEYCODE_W, Key::W },
	{ KEYCODE_X, Key::X },
	{ KEYCODE_Y, Key::Y },
	{ KEYCODE_Z, Key::Z },
	{ KEYCODE_LEFT_BRACKET, Key::BRACKETLEFT },
	{ KEYCODE_BACK_SLASH, Key::BACKSLASH },
	{ KEYCODE_RIGHT_BRACKET, Key::BRACKETRIGHT },
	{ KEYCODE_CIRCUMFLEX, Key::ASCIICIRCUM },
	{ KEYCODE_UNDERSCORE, Key::UNDERSCORE },
	{ KEYCODE_GRAVE, Key::QUOTELEFT },
	{ KEYCODE_LEFT_BRACE, Key::BRACELEFT },
	{ KEYCODE_BAR, Key::BAR },
	{ KEYCODE_RIGHT_BRACE, Key::BRACERIGHT },
	{ KEYCODE_TILDE, Key::ASCIITILDE },
	{ KEYCODE_MAX, Key::UNKNOWN }
};

Key godot_code_from_qnx_code(int p_code);

KeyLocation godot_location_from_qnx_code(int p_code);
