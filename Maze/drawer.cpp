#include "drawer.h"

namespace triGraphic{

	Drawer::Drawer( 
#if defined(_WIN32)
		HINSTANCE hInstance, LRESULT CALLBACK (*WndProc)(HWND, UINT, WPARAM, LPARAM),
#endif
		enum WindowStyle style, 
#if defined(VK_USE_PLATFORM_XCB_KHR)		
		CHEFR che, 
#endif
		UIFunc func, std::string windowName, uint32_t width, uint32_t height, int32_t init_camera_rot_x, int32_t init_camera_rot_y):
 VulkanExample(windowName)
#if defined(VK_USE_PLATFORM_XCB_KHR)
 , customHandleEvent(che)
#endif

 {
 	user_input = func;
	if(style == WS_FULLSCREEN)
		settings.fullscreen = true;
	else{
		width = width;
		height = height;
	}

	rotation.x = init_camera_rot_x;
	rotation.y = init_camera_rot_y;

	initVulkan();
#if defined(VK_USE_PLATFORM_XCB_KHR)
	setupWindow();
#elif defined(_WIN32)																	
	setupWindow(hInstance, WndProc);													
#endif

}

#if defined(VK_USE_PLATFORM_XCB_KHR)

void Drawer::handleEvents(){
	xcb_generic_event_t *event;
	while ((event = xcb_poll_for_event(connection)))
	{
		handleEvent(event);
		localHandleEvent(event);
		free(event);
	}


}


void Drawer::localHandleEvent(const xcb_generic_event_t *event) //handles the xcb window events
{
		handleEvent(event); //handling events by the APIManager
	

	if(customHandleEvent != nullptr)
		customHandleEvent(event); //handling events by given customHandleEvent function

	switch(event->response_type & 0x7f){
		case XCB_KEY_PRESS: //Keyboard input
		{
			const xcb_key_release_event_t *keyEvent = reinterpret_cast<const xcb_key_release_event_t *>(event);

		switch (keyEvent->detail)
			{
				case KEY_W:
					break;
				case KEY_S:
					break;
				case KEY_A:
					break;
				case KEY_D:{
					break;
				}
				case KEY_P:
					break;
				case KEY_F1:
					break;				
			}
		}
		break;
	}	

}

#endif

};