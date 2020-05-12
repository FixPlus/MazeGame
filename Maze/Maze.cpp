#include "drawer.h"
#include "Models.h"
#include "GameManager.h"
#include "MazeUI.h"

#if defined(VK_USE_PLATFORM_XCB_KHR)

#include <unistd.h>

#endif

#include <ctime>
#include <list>
#include <cstdlib>
#include "MazeStandartHeader.h"
#include "InputHandler.h"
#include <thread>

/*
	Maze Game 3D

	Simple 3D game coded 

	for university project.

	Bushev Dmitry

	2019 - 2020

*/

using namespace triGraphic;

Drawer* drawer;



MazeGame::GameCore* MazeGame::gameCore;
MazeUI::Manager MazeUI::manager;  // Global UI manager, handles all UI windows and input
bool MazeGame::should_update_static_vertices = false; 

int MazeGame::CoinObject::count = 0;
int MazeGame::GameObject::count = 0;
template<typename AnyDynamicModel>
int MazeGame::Cannon<AnyDynamicModel>::next_id = 1;

float MAZE_FPS = 0.0f;






class FpsCounter{
	float timer = 0.0f;
	int frame_count = 0;
public:
	float fps = 60.0f;

	void addFrame(float dt){
		frame_count++;
		timer += dt;
		if(timer >= 1.0f){
			fps = static_cast<float>(frame_count) / timer;
			MAZE_FPS = fps;
			timer = 0.0f;
			frame_count = 0;
		}
	}

};



FpsCounter fpsCounter;






void renderLoop(){
	float deltaTime = 0.0f;
	while(!drawer->shouldQuit() && !MazeGame::gameCore->shouldQuit() ){
	
		auto tStart = std::chrono::high_resolution_clock::now();


		drawer->draw(); //RENDERING THE FRAME



		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();


		int timeToSleepMicroSecs = 1000000u/60 - tDiff * 1000;
		if(timeToSleepMicroSecs < 0)
			timeToSleepMicroSecs = 0;

#if defined(VK_USE_PLATFORM_XCB_KHR)
		usleep((unsigned int)timeToSleepMicroSecs);
#elif defined(_WIN32)
		Sleep((unsigned int)timeToSleepMicroSecs / 1000);
#endif

		tEnd = std::chrono::high_resolution_clock::now();
		tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		

		deltaTime = tDiff / 1000.0f; // time of current cycle turn in seconds
		fpsCounter.addFrame(deltaTime);	

	}

}

void gameLoop(){
	float deltaTime = 0.0f;
	while(!drawer->shouldQuit() && !MazeGame::gameCore->shouldQuit() ){
	
		auto tStart = std::chrono::high_resolution_clock::now();

		MazeGame::gameCore->update(deltaTime); //ALL IN-GAME EVENTS HAPPEN HERE

#if defined(VK_USE_PLATFORM_XCB_KHR)
		drawer->handleEvents(); // LISTENING TO USER INPUT
#endif




		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();


		int timeToSleepMicroSecs = 1000000u/20 - tDiff * 1000;
		if(timeToSleepMicroSecs < 0)
			timeToSleepMicroSecs = 0;

#if defined(VK_USE_PLATFORM_XCB_KHR)
		usleep((unsigned int)timeToSleepMicroSecs);
#elif defined(_WIN32)
		Sleep((unsigned int)timeToSleepMicroSecs / 1000);
#endif


		tEnd = std::chrono::high_resolution_clock::now();
		tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		
		deltaTime = tDiff / 1000.0f; // time of current cycle turn in seconds
	}

}


void singleLoop(){
	float deltaTime = 0.0f;
	while(!drawer->shouldQuit() && !MazeGame::gameCore->shouldQuit() ){
	
		auto tStart = std::chrono::high_resolution_clock::now();

		MazeGame::gameCore->update(deltaTime); //ALL IN-GAME EVENTS HAPPEN HERE

#if defined(VK_USE_PLATFORM_XCB_KHR)
		drawer->handleEvents(); // LISTENING TO USER INPUT
#endif


		drawer->draw(); //RENDERING THE FRAME


		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();


		int timeToSleepMicroSecs = 1000000u/60u - tDiff * 1000;
		if(timeToSleepMicroSecs < 0)
			timeToSleepMicroSecs = 0;

#if defined(VK_USE_PLATFORM_XCB_KHR)
		usleep((unsigned int)timeToSleepMicroSecs);
#endif


		tEnd = std::chrono::high_resolution_clock::now();
		tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		
		deltaTime = tDiff / 1000.0f; // time of current cycle turn in seconds
		fpsCounter.addFrame(deltaTime);
	}

}




#if defined(_WIN32)

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						
{																									
	if (drawer != NULL)																		
	{																								
		drawer->handleMessages(hWnd, uMsg, wParam, lParam);									
	}																								
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												
}	


#endif


#if defined(VK_USE_PLATFORM_XCB_KHR)

int main(int argc, char** argv)

#elif defined(_WIN32)

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)

#endif

{
	srand(time(NULL));

	float deltaTime = 0.0f;
	float overallTime = 0.0f;


	int fieldSize = 75;
	enum WindowStyle style = WS_WINDOWED;

	int number_of_creatures = 250;
	int my_argc;
	char** my_argv;

#if defined(VK_USE_PLATFORM_XCB_KHR)
	
	my_argc = argc;
	my_argv = argv;

#elif defined(_WIN32)
	my_argc = __argc;
	my_argv = __argv;
#endif
	//TODO: move args reading and proccessing out of main() to some function/class

	for(int i = 0; i < my_argc; i++){
		std::string arg = my_argv[i];
		if(arg == FULLSCREEN_MSG)
			style = WS_FULLSCREEN;
		if(arg == FIELD_SIZE_MSG){
			if(i + 1 == my_argc){
				std::cout << "You should input NUMBER after '"<<  arg <<"' token!" << std::endl;
				return 0;
			}
			fieldSize = atoi(my_argv[i + 1]);
		}
		if(arg == DEBUG_UNIFORM_MSG_1){
			if(i + 1 == my_argc){
				std::cout << "You should input NUMBER after '"<<  arg <<"' token!" << std::endl;
				continue;
			}
			number_of_creatures = atoi(my_argv[i + 1]);
		}

	}

#if defined(_WIN32)

	for (int32_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };  			

#endif







// INITIALIZATION PROCESS

// STEP 1 : Starting a graphics core
	MazeGame::GameCore*& gc_ref = MazeGame::gameCore;

#if defined(VK_USE_PLATFORM_XCB_KHR)
		std::cout << "Constructing drawer" << std::endl;
	drawer = new Drawer(style, nullptr,  [&gc_ref](UserInputMessage message){ gc_ref->getInputHandler().handler(message);}, "MazeGame");

#elif defined(_WIN32)

	drawer = new Drawer(hInstance, WndProc, style,[&gc_ref](UserInputMessage message){ gc_ref->getInputHandler().handler(message);}, "MazeGame");

#endif

//	drawer->uboVS.lodBias = 6.0f;

// STEP 2: Initializing GameManager




	MazeGame::gameCore = new MazeGame::GameManager{fieldSize, fieldSize};
	
	MazeGame::gameCore->initialize();

	MazeUI::Window* fpsWindow = new MazeUI::Window("", 0.9f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	fpsWindow->addNewItem(new MazeUI::StatText<float>(MAZE_FPS, "fps"));
	MazeUI::manager.addNewElement(fpsWindow);

	drawer->updateOverlay();

// GAME LOOP STARTS HERE

#ifdef MULTITHREADING
	std::thread gameEvents{gameLoop};
	renderLoop();
	gameEvents.join();
#else
	singleLoop();
#endif

// FREEING THE ALLOCATED DATA

	delete MazeGame::gameCore;



	delete drawer;

	return 0; 
}


