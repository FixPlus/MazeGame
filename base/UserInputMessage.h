#pragma once

struct UserInputMessage{
	enum Type {UIM_KEYDOWN, UIM_KEYUP, UIM_MOUSE_BTN_DOWN, UIM_MOUSE_BTN_UP, UIM_MOUSEWHEEL_MOVE, UIM_DEFAULT} type;
	unsigned char detail;
	char s_detail;
};
