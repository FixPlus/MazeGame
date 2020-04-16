#include "drawer.h"
#include "GameField.h"
#include "Models.h"
#include "Objects.h"
#include <unistd.h>
#include <ctime>
#include <list>
#include <cstdlib>
#include "MazeStandartHeader.h"


/*
	Maze Game 3D

	Simple 3D game coded just for fun.

	Bushev Dmitry 2019 - 2020

*/

using namespace triGraphic;

Drawer* drawer;
PlayerObject<SimpleArrow>* player;
int* triGraphic::counter = &CoinObject::count;



DTManager MazeGame::triManager;   // Global triangle manager used by model to recieve and return triangles
CellField MazeGame::gameField;    // Global game field - core object of the game, used by all in game objects
FieldModel* MazeGame::fieldModel; // Global field model, that used to draw a game field and other enviropment
int CoinObject::count = 0;




class CameraKeeper final {
	
	Drawer* drawer;
	glm::vec3 disposal;
	Model* objectToFixAt;

public:
	CameraKeeper(Drawer* idrawer = NULL,Model* objTofixAt = NULL, glm::vec3 idisposal = {-15.0f, -15.0f, -15.0f}): drawer(idrawer), objectToFixAt(objTofixAt), disposal(idisposal){
	};

	void setDisposal(glm::vec3 newDisposal){
		disposal = newDisposal;
	};

	void holdCamera(){
		if(drawer)
			drawer->moveCamera(-objectToFixAt->getPosition() - disposal);

	};

	void scaleDisposal(float mult){
		disposal *= mult;
	};

	void rotateDisposal(glm::vec3 axis, float angle){
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians(angle), axis);

		disposal = glm::vec3(rotateMat * glm::vec4(disposal, 0.0f));

		drawer->rotateCamera(axis.x * angle, axis.y * angle, axis.z * angle);
	}

};



CameraKeeper camKeep;


void gameHandleEvents(const xcb_generic_event_t *event){
//	std::cout << "Event id:" << (event->response_type & 0x7f) <<  std::endl;
	switch(event->response_type & 0x7f){
		case XCB_KEY_PRESS: //Keyboard input
		{
			const xcb_key_release_event_t *keyEvent = reinterpret_cast<const xcb_key_release_event_t*>(event);
		switch (keyEvent->detail)
			{
				case KEY_W:{
					player->moveInDirection();
					break;
				}
				case KEY_S:
					player->changeDirection(player->getDir() + 2);
					break;
				case KEY_A:
					player->changeDirection(player->getDir() + 3);
					break;
				case KEY_D:
					player->changeDirection(player->getDir() + 1);
					break;
				case KEY_R:
					break;
				case KEY_P:
					break;
				case KEY_F1:
					break;				
			}
		}
		case XCB_KEY_RELEASE:{
//			std::cout << "Released" << std::endl;
		}
		case XCB_BUTTON_PRESS:
		{
			xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
//			std::cout << "Button press details: " << static_cast<int>(press->detail) <<std::endl;

			switch(press->detail){
				case 4:{
					//camKeep.scaleDisposal(1.1f);
					camKeep.rotateDisposal(glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);
	//				std::cout << "Mouse wheel: " << press->event_x << std::endl;
					break;
				}
				case 5:{
					//camKeep.scaleDisposal(0.9f);
					camKeep.rotateDisposal(glm::vec3(0.0f, 1.0f, 0.0f), -10.0f);
	//				std::cout << "Mouse wheel: " << press->event_x << std::endl;
					break;
				}
				default:{

				}
			}
/*			if (press->detail == XCB_BUTTON_INDEX_1)
				mouseButtons.left = true;
			if (press->detail == XCB_BUTTON_INDEX_2)
				mouseButtons.middle = true;
			if (press->detail == XCB_BUTTON_INDEX_3)
				mouseButtons.right = true;
*/
		}
		break;
		case XCB_BUTTON_RELEASE:
		{
			xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
//			std::cout << "Button release details: " << static_cast<int>(press->detail) <<std::endl;
/*			if (press->detail == XCB_BUTTON_INDEX_1)
				mouseButtons.left = false;
			if (press->detail == XCB_BUTTON_INDEX_2)
				mouseButtons.middle = false;
			if (press->detail == XCB_BUTTON_INDEX_3)
				mouseButtons.right = false;
*/
		}

		break;
	}	

}





int main(int argc, char** argv){
	srand(time(NULL));

	float deltaTime = 0.0f;
	float overallTime = 0.0f;


	int fieldSize = 50;
	enum WindowStyle style = WS_WINDOWED;

	int number_of_creatures = 5;
	for(int i = 0; i < argc; i++){
		std::string arg = argv[i];
		if(arg == FULLSCREEN_MSG)
			style = WS_FULLSCREEN;
		if(arg == FIELD_SIZE_MSG){
			if(i + 1 == argc){
				std::cout << "You should input NUMBER after '"<<  arg <<"' token!" << std::endl;
				return 0;
			}
			fieldSize = atoi(argv[i + 1]);
		}
		if(arg == DEBUG_UNIFORM_MSG_1){
			if(i + 1 == argc){
				std::cout << "You should input NUMBER after '"<<  arg <<"' token!" << std::endl;
				continue;
			}
			number_of_creatures = atoi(argv[i + 1]);
		}

	}



// INITIALIZATION PROCESS

// STEP 1 : Starting a graphics core


	drawer = new Drawer(style, gameHandleEvents, "MazeGame");
//	drawer->uboVS.lodBias = 6.0f;


// STEP 2 : Creating and generating a gameField

	MazeGame::gameField = CellField{fieldSize, fieldSize};
	MazeGame::gameField.generateRandomMaze();

// STEP 3 : Initializing triangle manager

	MazeGame::triManager = DTManager{drawer, polysRequested(), 20000};


// STEP 4: Setting up models

	MazeGame::fieldModel = new FieldModel{};

	

	Cell init = *MazeGame::gameField.getRandomCell(CellType::PATH);


	player = dynamic_cast<PlayerObject<SimpleArrow>*>(MazeGame::gameField.addNewGameObject(new PlayerObject<SimpleArrow>{static_cast<float>(init.x), static_cast<float>(init.y), 5.0f,  glm::vec3{1.0f, 0.0f, 0.0f}, 5.0f}));

	for(int i = 0; i < number_of_creatures; i++){
		do{
			init = *MazeGame::gameField.getRandomCell(CellType::PATH);
		}while(MazeGame::gameField.isThereObjectsInCell(init.x, init.y));

		MazeGame::gameField.addNewGameObject(new Seeker<SimpleOctagon>{init.x, init.y, 5.0f, 5.0f, {1.0f, 1.0f, 1.0f}, player});

		do{
			init = *MazeGame::gameField.getRandomCell(CellType::PATH);
		}while(MazeGame::gameField.isThereObjectsInCell(init.x, init.y));

		MazeGame::gameField.addNewGameObject(new CoinObject{static_cast<float>(init.x), static_cast<float>(init.y), 5.0f});
	}


// STEP 5: finishing initialization, updaating static vertices and setting up camera

	drawer->updateStaticVertices();


	camKeep = CameraKeeper{drawer, player, {-70.0f,-60.0f,-70.0f}};
	camKeep.scaleDisposal(0.5);

	drawer->updateOverlay();

// GAME LOOP STARTS HERE

	while(!drawer->shouldQuit() && CoinObject::count > 0){

		auto tStart = std::chrono::high_resolution_clock::now();

		MazeGame::gameField.update(deltaTime); //ALL IN-GAME EVENTS HAPPEN HERE

		drawer->handleEvents(); // LISTENING TO USER INPUT
	//	player->moveInDirection();
		camKeep.holdCamera();
		drawer->draw(); //RENDERING THE FRAME



		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();


		int timeToSleepMicroSecs = 1000000u/60 - tDiff * 1000;
		if(timeToSleepMicroSecs < 0)
			timeToSleepMicroSecs = 0;

		usleep((unsigned int)timeToSleepMicroSecs);

		tEnd = std::chrono::high_resolution_clock::now();
		tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		
		deltaTime = tDiff / 1000.0f; // time of current cycle turn in seconds

		overallTime += deltaTime;


	} //GAME LOOP ENDS HERE



// FREEING THE ALLOCATED DATA

	MazeGame::gameField.freeGameObjects();
	delete MazeGame::fieldModel;



	delete drawer;


} // triangle manager and game field are destroyed here