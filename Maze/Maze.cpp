#include "drawer.h"
#include "GameField.h"
#include "Models.h"
#include "Objects.h"
#include "MazeUI.h"
#include <unistd.h>
#include <ctime>
#include <list>
#include <cstdlib>
#include "MazeStandartHeader.h"


/*
	Maze Game 3D

	Simple 3D game coded 

	for university project.

	Bushev Dmitry, Evgeniya Korelskaya 

	2019 - 2020

*/




/* Primary TODO list for the project:

	TODO: make UI  (in process)
		description:
			Windows and buttons, that display in-game info
			and collect user input

			1) Make a wrapping above the ImGui (almost done)
			2) Make a class that will manage the preparation of UI (not started)

			3) Fix some bugs with windows appering due to screen size change


	TODO: make MECH_MANGER (not started yet)
		decription:
			class or bunch of classes, that organize game proccess
			like level preparation, object spawning and destruction (though objs are owned by gameField, 
			but not managed by it)

			1) Make sure we know what we want be in the game, how it will be look like (in proccess)
			2) Create MECH_MANAGER, imply all our ideas about game process (not started yet)


	TODO: Texture and model loading arrangement (in proccess)
		description:
			As it is poorly arranged on current stage of the project
			it will be hard to expand the list of 3D models in the next stages
			So it definitely needs to be organized in the way to avoid much 
			hardcoded chuncks (for example it could be managed to load models from files) 
			
			1) Arrange satisfactory system of texture and model loading(in process)
			2) Make Models for the game (not started yet)


	**********************************EXTRA TODO LIST**********************************

	TODO: Animation implementation

	TODO: Sound effects implementation


*/



using namespace triGraphic;

Drawer* drawer;
MazeGame::PlayerObject<SimpleArrow>* player; // TODO: Move player object in MECH_MANAGER class



DTManager MazeGame::triManager;   // Global triangle manager used by model to recieve and return triangles
MazeGame::CellField MazeGame::gameField;    // Global game field - core object of the game, used by all in game objects
FieldModel* MazeGame::fieldModel; // Global field model, that used to draw a game field and other enviropment
MazeUI::Manager MazeUI::manager;  // Global UI manager, handles all UI windows and input
bool MazeGame::should_update_static_vertices = false; 

int MazeGame::CoinObject::count = 0;
int MazeGame::GameObject::count = 0;
template<typename AnyDynamicModel>
int MazeGame::Cannon<AnyDynamicModel>::next_id = 1;
bool force_quit = false;





// TODO:: move CameraKeeper out of Maze.cpp, rearrange camera control

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
			timer = 0.0f;
			frame_count = 0;
		}
	}

};



CameraKeeper camKeep;
FpsCounter fpsCounter;


// TODO: move gameHandleEvents out of Maze.cpp, create special class that will handle it

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
					MazeGame::gameField.addNewGameObject(new MazeGame::Bullet<SimpleOctagon>(static_cast<float>(player->getCell()->x), static_cast<float>(player->getCell()->y), 1.0f, {1.0f, 1.0f, 1.0f}, 10.0f, player->getDir(), 0));
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



void spawnCannon(){
	MazeGame::Cell cell;
	do{
		cell = *MazeGame::gameField.getRandomCell(MazeGame::CellType::PATH);
	}while(MazeGame::gameField.isThereObjectsInCell(cell.x, cell.y));

	MazeGame::gameField.addNewGameObject(new MazeGame::Cannon<SimpleArrow>{static_cast<float>(cell.x), static_cast<float>(cell.y), 5.0f, {1.0f, 0.0f, 0.0f}, 5.0f, 2, 2.0});

}




int main(int argc, char** argv){
	srand(time(NULL));

	float deltaTime = 0.0f;
	float overallTime = 0.0f;


	int fieldSize = 50;
	enum WindowStyle style = WS_FULLSCREEN;

	int number_of_creatures = 5;


	//TODO: move args reading and proccessing out of main() to some function/class

	for(int i = 0; i < argc; i++){
		std::string arg = argv[i];
//		if(arg == FULLSCREEN_MSG)
//			style = WS_FULLSCREEN;
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


	// TODO: rearrange the gameField creation and recreation inside game mechanics class(aka MECH_MANAGER) and make it free out of triangle manager   

	MazeGame::gameField = MazeGame::CellField{fieldSize, fieldSize};
	MazeGame::gameField.generateRandomMaze();

// STEP 3 : Initializing triangle manager

	MazeGame::triManager = DTManager{drawer, 25000, 20000};


// STEP 4: Setting up models and constructing game objects



	//TODO: organize GameObjects creation in some manager class (aka MECH_MANAGER), move there all game mechanics realization

	MazeGame::fieldModel = new FieldModel{};

	


	MazeGame::Cell init = *MazeGame::gameField.getRandomCell(MazeGame::CellType::PATH);


	player = dynamic_cast<MazeGame::PlayerObject<SimpleArrow>*>(MazeGame::gameField.addNewGameObject(new MazeGame::PlayerObject<SimpleArrow>{static_cast<float>(init.x), static_cast<float>(init.y), 5.0f,  glm::vec3{1.0f, 0.0f, 0.0f}, 5.0f}));

	for(int i = 0; i < number_of_creatures; i++){
		do{
			init = *MazeGame::gameField.getRandomCell(MazeGame::CellType::PATH);
		}while(MazeGame::gameField.isThereObjectsInCell(init.x, init.y));

		//MazeGame::gameField.addNewGameObject(new Seeker<SimpleOctagon>{init.x, init.y, 5.0f, 5.0f, {1.0f, 1.0f, 1.0f}, player});

		do{
			init = *MazeGame::gameField.getRandomCell(MazeGame::CellType::PATH);
		}while(MazeGame::gameField.isThereObjectsInCell(init.x, init.y));

		MazeGame::gameField.addNewGameObject(new MazeGame::CoinObject{static_cast<float>(init.x), static_cast<float>(init.y), 5.0f});
		spawnCannon();
	}


// STEP 5: finishing initialization, updating static vertices and setting up camera

	drawer->updateStaticVertices();


	camKeep = CameraKeeper{drawer, player, {-70.0f,-60.0f,-70.0f}};
	camKeep.scaleDisposal(0.5);

//STEP 5.1 initializing user interface 

	//TODO: make a special class managing intialization and re-initialization of UI 

	MazeUI::Window* testWindow = new MazeUI::Window("Maze game", 0.0f, 0.7f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	testWindow->addNewItem(new MazeUI::StatText<float>(fpsCounter.fps, "fps"));
	testWindow->addNewItem(new MazeUI::StatText<int>(MazeGame::CoinObject::count, "Coins left"));
	testWindow->addNewItem(new MazeUI::StatText<int>(MazeGame::GameObject::count, "Objects"));
	testWindow->addNewItem(new MazeUI::StatText<float>(player->x, "x"));
	testWindow->addNewItem(new MazeUI::StatText<float>(player->y, "y"));
	testWindow->addNewItem(new MazeUI::Button("More coins!", [](){
		MazeGame::Cell cell;
		do{
			cell = *MazeGame::gameField.getRandomCell(MazeGame::CellType::PATH);
		}while(MazeGame::gameField.isThereObjectsInCell(cell.x, cell.y));

		MazeGame::gameField.addNewGameObject(new MazeGame::CoinObject{static_cast<float>(cell.x), static_cast<float>(cell.y), 5.0f});
		spawnCannon();
	
	}));


	MazeUI::Window* exitWindow = new MazeUI::Window("", 0.9f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	exitWindow->addNewItem(new MazeUI::Button("Exit", [](){
		force_quit = true;
	}));


	int straightness = 5;
	int cycles = 3;

	MazeUI::Window* debugWindow = new MazeUI::Window("Maze params", 0.7f, 0.7f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	debugWindow->addNewItem(new MazeUI::StatText<int>(MazeGame::gameField.getHeight(), "Height"));
	debugWindow->addNewItem(new MazeUI::StatText<int>(MazeGame::gameField.getWidth(), "Width"));
	debugWindow->addNewItem(new MazeUI::InputBox("Straightness", straightness, 0, 20));
	debugWindow->addNewItem(new MazeUI::InputBox("Cycleness", cycles, 0, 10));
	
	debugWindow->addNewItem(new MazeUI::Button("Recreate Maze", [&straightness, &cycles](){
		MazeGame::gameField.generateRandomMaze(straightness, static_cast<float>(cycles));
		delete MazeGame::fieldModel;
		MazeGame::fieldModel = new FieldModel{};
		drawer->updateStaticVertices();
	}));

	MazeUI::Window* hintWindow = new MazeUI::Window("Hint", 0.0f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	hintWindow->addNewItem(new MazeUI::Text("Press 'R' to fire"));


	MazeUI::manager.addNewElement(testWindow);
	MazeUI::manager.addNewElement(exitWindow);
	MazeUI::manager.addNewElement(debugWindow);
	MazeUI::manager.addNewElement(hintWindow);

	drawer->updateOverlay();

// GAME LOOP STARTS HERE

	while(!drawer->shouldQuit() && MazeGame::CoinObject::count > 0 && !force_quit){

		auto tStart = std::chrono::high_resolution_clock::now();

		MazeGame::gameField.update(deltaTime); //ALL IN-GAME EVENTS HAPPEN HERE   TODO: Move game events to MECH_MANAGER class

		drawer->handleEvents(); // LISTENING TO USER INPUT
	//	player->moveInDirection();
		camKeep.holdCamera();
		if(MazeGame::should_update_static_vertices){
			MazeGame::fieldModel->recreate();
			drawer->updateStaticVertices();
			MazeGame::should_update_static_vertices = false;
		}
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
		//std::cout << deltaTime << std::endl;

		fpsCounter.addFrame(deltaTime);
		//std::cout << "end frame" << std::endl;
		overallTime += deltaTime;


	} //GAME LOOP ENDS HERE



// FREEING THE ALLOCATED DATA

	MazeGame::gameField.freeGameObjects();
	delete MazeGame::fieldModel;



	delete drawer;


} // triangle manager and game field are destroyed here



