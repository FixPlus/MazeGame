#pragma once

#include "UserInputMessage.h"





struct InputHandler{

	void handler(UserInputMessage message){
		switch (message.type){
			case UserInputMessage::Type::UIM_KEYDOWN: //Keyboard input
			{
				onKeyDown(message.detail);
				break;
			}
			case UserInputMessage::Type::UIM_KEYUP:{
				onKeyUp(message.detail);
				break;
			}
			case UserInputMessage::Type::UIM_MOUSEWHEEL_MOVE:{
				onMouseWheelMove(message.s_detail);
				break;			
			}
			case UserInputMessage::Type::UIM_MOUSE_BTN_DOWN:
			{
				onMsBtnDown(message.detail);
			}
			break;
			case UserInputMessage::Type:: UIM_MOUSE_BTN_UP:
			{
				onMsBtnUp(message.detail);
			}

			break;
			default:
			break;
		}	

	}

	std::function<void(unsigned char)> onKeyDown = [](unsigned char){}, onKeyUp= [](unsigned char){}, 
									   onMsBtnDown= [](unsigned char){}, onMsBtnUp= [](unsigned char){};
	std::function<void(char)> onMouseWheelMove= [](char){};

	void reset(){
		onKeyUp = onKeyDown = onMsBtnDown = onMsBtnUp = [](unsigned char){};
		onMouseWheelMove= [](char){};
	}
};