#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
constexpr int MAX_XBOX_CONTROLLERS = 4;
constexpr int MAX_KEY_CODES = 256;


//-----------------------------------------------------------------------------------------------
// Key Codes
//
extern const unsigned char KEY_ESC;
extern const unsigned char KEY_ENTER;
extern const unsigned char KEY_SHIFT;
extern const unsigned char KEY_CTRL;
extern const unsigned char KEY_ALT;
extern const unsigned char KEY_SPACEBAR;
extern const unsigned char KEY_BACKSPACE;
extern const unsigned char KEY_DELETE;
extern const unsigned char KEY_UPARROW;
extern const unsigned char KEY_LEFTARROW;
extern const unsigned char KEY_DOWNARROW;
extern const unsigned char KEY_RIGHTARROW;
extern const unsigned char KEY_HOME;
extern const unsigned char KEY_END;
extern const unsigned char KEY_F1;
extern const unsigned char KEY_F2;
extern const unsigned char KEY_F3;
extern const unsigned char KEY_F4;
extern const unsigned char KEY_F5;
extern const unsigned char KEY_F6;
extern const unsigned char KEY_F7;
extern const unsigned char KEY_F8;
extern const unsigned char KEY_F9;
extern const unsigned char KEY_F10;
extern const unsigned char KEY_F11;
extern const unsigned char KEY_F12;
extern const unsigned char KEY_SEMICOLON;
extern const unsigned char KEY_TILDE;
extern const unsigned char KEY_LEFT_BRACKET;
extern const unsigned char KEY_RIGHT_BRACKET;
extern const unsigned char KEY_QUOTE;
extern const unsigned char KEY_COMMA;
extern const unsigned char KEY_PERIOD;
extern const unsigned char KEY_PLUS;
extern const unsigned char KEY_MINUS;

extern const unsigned char MOUSE_LBUTTON;
extern const unsigned char MOUSE_RBUTTON;
extern const unsigned char MOUSE_MBUTTON;

extern const unsigned char CMD_PASTE;


//-----------------------------------------------------------------------------------------------
enum eCursorMode : uint
{
	CURSOR_ABSOLUTE,
	CURSOR_RELATIVE
};


//-----------------------------------------------------------------------------------------------
struct MouseOptions
{
public:
	eCursorMode m_cursorMode = CURSOR_ABSOLUTE;
	bool m_isVisible = true;
	bool m_isClipped = false;
};
