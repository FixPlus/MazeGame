#pragma once
#include "VulkanExample.h"
#include "DrawableTriangle.h"

// Drawer is used to operate with VulkanExample class by creating, initializing, preparing, rendering, handling events and deleting

namespace triGraphic{


enum WindowStyle{WS_WINDOWED, WS_FULLSCREEN};


class Drawer final: public VulkanExample
{
#if defined(VK_USE_PLATFORM_XCB_KHR)
	typedef void (*CHEFR)(const xcb_generic_event_t *event); //typedefed pointer to handleEvent function 
	CHEFR customHandleEvent;                //used to have a possibility to handle events out of Drawer class
#endif

public:

	explicit Drawer(
#if defined(_WIN32)
		HINSTANCE hInstance, LRESULT CALLBACK (*WndProc)(HWND, UINT, WPARAM, LPARAM),
#endif
		enum WindowStyle style = WS_WINDOWED, 
#if defined(VK_USE_PLATFORM_XCB_KHR)		
		CHEFR che = nullptr, 
#endif
		UIFunc func = nullptr, std::string windowName = "Window", uint32_t width = 1240, uint32_t height = 780, int32_t init_camera_rot_x = 90, int32_t init_camera_rot_y = -540);

	Drawer(Drawer const &rhs) = delete;


	void draw()	{   //copies the vertices info to GPU memory and renders frame
		updateUniformBuffers();
		renderLoop();
	};


	bool shouldQuit() const { return quit;}; //return the APIManager state quit

#if defined(VK_USE_PLATFORM_XCB_KHR)
	void handleEvents(); //handles the xcb window events
	void localHandleEvent(const xcb_generic_event_t *event); //handles current xcb event
#endif

	Drawer& operator=(Drawer const &rhs) = delete;

	glm::vec3 getCameraPos(){
		return cameraPos;
	}

	void moveCamera(glm::vec3 newPos){
		cameraPos = newPos;
	}

	void setCameraAxis(glm::vec3 axis){
		axis = glm::normalize(axis);
		rotation.x = (acos(glm::dot(axis, glm::vec3{0.0f,-1.0f, 0.0f})) * 720.0f / 3.141592f) - 120.0f;
		glm::vec3 axis_proj = {axis.x, 0.0f, axis.z};
		axis_proj = glm::normalize(axis_proj);
		rotation.y = (acos(glm::dot(axis_proj, glm::vec3{0.0f, 0.0f, 1.0f}))* 720.0f / 3.141592f * (glm::dot(axis_proj, glm::vec3{1.0f, 0.0f, 0.0f}) > 0.0f ? 1.0f : -1.0f)  );
	}
	
	void OnUpdateUIOverlay(vks::UIOverlay *overlay)
	{
	}




};

};
