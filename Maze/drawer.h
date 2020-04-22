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
		std::string windowName = "Window", uint32_t width = 1240, uint32_t height = 780, int32_t init_camera_rot_x = 90, int32_t init_camera_rot_y = -540);

	Drawer(Drawer const &rhs) = delete;


	void draw()	{   //copies the vertices info to GPU memory and renders frame
		updateUniformBuffers();
		renderLoop();
	};

	template<typename It> //any random access iterator
	void connect(It beginStatic, It endStatic, It beginDynamic, It endDynamic) {
		prepared = false;
		prepare((endStatic - beginStatic) * 3, (endDynamic - beginDynamic) * 3);

		auto vertStatIt = localStaticVertices.begin();
		auto vertDynIt = localDynamicVertices.begin();

		for(; beginStatic < endStatic; beginStatic++, vertStatIt += 3)
			beginStatic[0].setIt(vertStatIt);

		for(; beginDynamic < endDynamic; beginDynamic++, vertDynIt += 3)
			beginDynamic[0].setIt(vertDynIt);


	} //connects the triangles iterators to localVertices buffer

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
	
	void OnUpdateUIOverlay(vks::UIOverlay *overlay)
	{
	}




};

};
