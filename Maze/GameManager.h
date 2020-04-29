#pragma once

#include "GameCore.h"
#include "Objects.h"
#include "drawer.h"
#include "MazeUI.h"

extern triGraphic::Drawer* drawer;
extern float MAZE_FPS;
namespace MazeGame{

class CameraKeeper final {
	
	glm::vec3 disposal;
	Model* objectToFixAt;

public:
	explicit CameraKeeper(Model* objTofixAt = NULL, glm::vec3 idisposal = {-15.0f, -15.0f, -15.0f}): objectToFixAt(objTofixAt), disposal(idisposal){
	};

	void setDisposal(glm::vec3 newDisposal){
		disposal = newDisposal;
	};

	void holdCamera(){
		if(drawer){
			drawer->moveCamera(-objectToFixAt->getPosition() - disposal);
			drawer->setCameraAxis(disposal);
		}

	};

	void scaleDisposal(float mult){
		disposal *= mult;
	};

	float const& getDisposalX() const{
		return disposal.x;
	}
	float const& getDisposalY() const{
		return disposal.y;
	}
	float const& getDisposalZ() const{
		return disposal.z;
	}

	void rotateDisposal(glm::vec3 axis, float angle){
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians(angle), axis);
		disposal = glm::vec3(rotateMat * glm::vec4(disposal, 0.0f));
		holdCamera();
	}

};


template <typename AnyDynamicModel>
class PlayerObject: public DynamicDirectedObject, public HealthObject, public AnyDynamicModel {
public:
	bool onMove = false;
	bool onRotate = false;
	bool onFiring = false;

	int nextDir = 0;
	std::function<void(void)> onDeath = [](){};
	explicit PlayerObject(Cell* par, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f}, float ispeed = 1.0f, int idir = 2):
	GameObject(par), Model(), HealthObject(1000.0f), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(size, color){ rotSpeed = 400.0f; };

	ObjectInfo getInfo() const override{
		return {ObjectType::PLAYER, 0};
	};

	void update(float dt) override{
		if(onMove)
			moveInDirection();
		if(onRotate)
			changeDirection(dir + nextDir);

		if(!isChangingDirection() && onFiring)
			gameCore->addNewGameObject(new MazeGame::Bullet<SimpleOctagon>(getCell(), 1.0f, {1.0f, 1.0f, 1.0f}, 10.0f, getDir(), 0));

		DynamicDirectedObject::update(dt);
	}

	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::COIN: {
				speed += 0.2f;
				break;
			}
			case ObjectType::POWERUP: {
				if(info.data != 0)
					modifyHP(-5.0f);
				break;
			}
		}
	};

	~PlayerObject(){
		onDeath();
	}

};



class GameManager: public GameCore{
	CameraKeeper camKeep;
	PlayerObject<SimpleArrow>* player; // this is NOT owning pointer
public:
	int setup = 0;
	bool paused = false;

	GameManager(int f_w = 50, int f_h = 50): GameCore(f_w, f_h){

	};

	void setupMenuScene();

	void setupLevelScene();

	void initialize() override {

		generateRandomMaze();

		recreate();

		setupMenuScene();

	}

	void update(float dt) override{
		switch(setup){
			case 1:
				setupMenuScene();
				break;
			case 2:
				setupLevelScene();
				break;	
		}
		setup = 0;
		if(!paused){
			GameCore::update(dt);
			camKeep.holdCamera();
		}
	}
	GameObject* getPlayer() {
		return player;
	}

};



void GameManager::setupMenuScene(){
	freeGameObjects();
	recreate();
	camKeep = CameraKeeper{dynamic_cast<Model*>(this), {-70.0f,-60.0f,-70.0f}};
	MazeUI::manager.clear();
	MazeUI::Window* menuWindow = new MazeUI::Window("Maze game", 0.4f, 0.4f, 0.2f, 0.2f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	menuWindow->addNewItem(new MazeUI::Button("Play!", [this](){
		setup = 2;
	}, 0.19f, 0.05f));		
	menuWindow->addNewItem(new MazeUI::Button("Exit", [this](){
		quit = true;
	}, 0.19f, 0.05f));		
	MazeUI::manager.addNewElement(menuWindow);

	inputHandler.reset();		
}

void GameManager::setupLevelScene(){

	freeGameObjects();
	generateRandomMaze();
	recreate();

	Cell* init = getRandomCell(MazeGame::CellType::PATH);

	player = dynamic_cast<PlayerObject<SimpleArrow>*>(addNewGameObject(new PlayerObject<SimpleArrow>{init, 5.0f,  glm::vec3{1.0f, 0.0f, 0.0f}, 5.0f}));

	player->onDeath = [this](){
		player = nullptr;
		setup = 1;
	};

	camKeep = CameraKeeper{dynamic_cast<Model*>(player), {-70.0f,-60.0f,-70.0f}};
	camKeep.scaleDisposal(0.5);






	for(int i = 0; i < 250; i++){
		do{
			init = getRandomCell(MazeGame::CellType::PATH);
		}while(isThereObjectsInCell(init));

		addNewGameObject(new MazeGame::CoinObject{init, 5.0f});

		do{
			init = getRandomCell(MazeGame::CellType::PATH);
		}while(isThereObjectsInCell(init));
		
		addNewGameObject(new MazeGame::Cannon<SimpleArrow>{init, 5.0f, {1.0f, 0.0f, 0.0f}, 5.0f, 2, 2.0});
	}



	MazeUI::manager.clear();
	MazeUI::Window* statWindow = new MazeUI::Window("Stats", 0.0f, 0.7f, 0.2f, 0.3f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	statWindow->addNewItem(new MazeUI::StatText<float>(player->hp(), "HP"));


	MazeUI::Window* debugWindow = new MazeUI::Window("Debug", 0.0f, 0.2f, 0.2f, 0.3f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	debugWindow->addNewItem(new MazeUI::StatText<float>(drawer->rotation.x, "rotX"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(drawer->rotation.y, "rotY"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(camKeep.getDisposalX(), "disX"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(camKeep.getDisposalY(), "disY"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(camKeep.getDisposalZ(), "disZ"));
	debugWindow->addNewItem(new MazeUI::StatText<int>(player->getDir(), "dir"));
	debugWindow->addNewItem(new MazeUI::StatText<bool>(player->onRotate, "onRot"));

	debugWindow->addNewItem(new MazeUI::StatText<int>(MazeGame::GameObject::count, "Objects"));
	
	debugWindow->visible = false;

	statWindow->addNewItem(new MazeUI::Button("Menu", [this](){
		setup = 1;
	}, 0.19f, 0.05f));

	MazeUI::Window* fpsWindow = new MazeUI::Window("", 0.9f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	fpsWindow->addNewItem(new MazeUI::StatText<float>(MAZE_FPS, "fps"));
	MazeUI::Window* hintWindow = new MazeUI::Window("Hint", 0.0f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	hintWindow->addNewItem(new MazeUI::Text("Press 'R' to fire"));
	//hintWindow->visible = false;

	MazeUI::manager.addNewElement(statWindow);
	MazeUI::manager.addNewElement(fpsWindow);
	MazeUI::manager.addNewElement(debugWindow);
	MazeUI::manager.addNewElement(hintWindow);

	inputHandler.reset();		

	inputHandler.onKeyDown = [this, debugWindow](unsigned char key){ 		
		switch (key)
		{
			case KEY_W:{
				player->onMove = true;
				break;
			}
			case KEY_S:
				player->onRotate = true;
				player->nextDir = 2;
				break;
			case KEY_A:
				player->onRotate = true;
				player->nextDir = 3;
				break;
			case KEY_D:
				player->onRotate = true;
				player->nextDir = 1;
				break;
			case KEY_R:
				player->onFiring = true;
				break;
			case KEY_P:
				debugWindow->visible = !debugWindow->visible;
//				paused = !paused;
				break;
			case KEY_F1:
				break;				
		}
	};
	inputHandler.onKeyUp = [this, hintWindow](unsigned char key){ 		
		switch (key)
		{
			case KEY_W:{
				player->onMove = false;
				//player->moveInDirection();
				break;
			}
			case KEY_S:
				player->onRotate = false;
				//player->changeDirection(player->getDir() + 2);
				break;
			case KEY_A:
				player->onRotate = false;
				//player->changeDirection(player->getDir() + 3);
				break;
			case KEY_D:
				player->onRotate = false;
				//player->changeDirection(player->getDir() + 1);
				break;
			case KEY_R:
				player->onFiring = false;
				//addNewGameObject(new MazeGame::Bullet<SimpleOctagon>(player->getCell(), 1.0f, {1.0f, 1.0f, 1.0f}, 10.0f, player->getDir(), 0));
				break;
			case KEY_P:
				//hintWindow->visible = false;
				break;
			case KEY_F1:
				break;				
		}
	};
	
	inputHandler.onMouseWheelMove = [this](char d){
		float dir = d > 0 ? 1.0f : -1.0f;
		camKeep.rotateDisposal(glm::vec3(0.0f, 1.0f, 0.0f), 10.0f * dir);
	};	

}

};