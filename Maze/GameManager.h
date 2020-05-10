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

class ObjectSpawner{
public:
	class SpawnInfo{
		float period, timer = 0.0f;
		int genCount = 0;
		int amount;
		std::function<bool(Cell*)> cellRule;
		std::function<GameObject*(Cell*)> objConstruct;

		float lifeTime = -1.0f;
		bool expired = false;
	public:
		SpawnInfo(float per = 1.0f, int amnt = 1,
				std::function<GameObject*(Cell*)> objCons = [](Cell*)->GameObject*{ return nullptr;}, 
				std::function<bool(Cell*)> cRule = [](Cell*){ return true;}, float lt = -1.0f): lifeTime(lt), period(per), cellRule(cRule), objConstruct(objCons), amount(amnt) { };
		void execute(float dt){
			timer += dt;
			if(timer > period * (genCount + 1)){
				++genCount;
				for(int i = 0; i < amount; ++i){
					
					Cell* parent = gameCore->getRandomCell(cellRule);
					GameObject* obj = nullptr;
					if(parent != nullptr)
						obj = objConstruct(parent);
					
					// may return nullptr if GameObject cant be spawned under some circumstances written inside lambda 
					// This is the legal way to limit the spawn of objects to avoid exceptional situation 


					if(obj != nullptr)
						gameCore->addNewGameObject(obj);
				}
			}
			if(lifeTime > 0.0f && timer > lifeTime)
				expired = true;
		}

		bool isExpired() const{
			return expired;
		}
	};
private:
	std::list<SpawnInfo> spawnTasks;
public:
	void update(float dt){
		for(auto it = spawnTasks.begin(); it != spawnTasks.end();){
			(*it).execute(dt);
			if((*it).isExpired())
				it = spawnTasks.erase(it);
			else
				++it;
		}
	};

	void addNewSpawnTask(SpawnInfo&& spnInfo){
		spawnTasks.emplace_back(spnInfo);
	}

	void clear(){
		spawnTasks.clear();
	}
};

template <typename AnyDynamicModel>
class PlayerObject: public DynamicDirectedObject, public HealthObject, public AnyDynamicModel { //TODO: move class declaration to Objects.h, leave only methods realisation here
public:
	bool onMove = false;
	bool onRotate = false;
	bool onFiring = false;

	int nextDir = 0;
	std::function<void(void)> onDeath = [](){};
	explicit PlayerObject(Cell* par, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f}, float ispeed = 1.0f, int idir = 2):
	GameObject(par), Model(), HealthObject(100.0f), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(M_CANNON, size){ rotSpeed = 400.0f; addNewRotationBack(std::make_pair(glm::vec3{1.0f, 0.0f, 0.0f}, 90.0f)); };

	ObjectInfo getInfo() const override{
		return {ObjectType::PLAYER, 0};
	};

	void update(float dt) override{
		if(onMove)
			moveInDirection();
		if(onRotate)
			changeDirection(dir + nextDir);

		if(!isChangingDirection() && onFiring)
			gameCore->addNewGameObject(new MazeGame::Bullet<SingleInstanceModel>(getCell(), 1.0f, {1.0f, 1.0f, 1.0f}, 10.0f, getDir(), 0));

		DynamicDirectedObject::update(dt);
	}

	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::COIN: {
				//speed += 0.2f;
				break;
			}
			case ObjectType::BULLET: {
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
	PlayerObject<SingleInstanceModel>* player; // this is NOT owning pointer
	ObjectSpawner spawner;
public:
	int setup = 0;
	bool paused = false;
	struct {
		int width = 75;
		int height = 75;
	} options;

	GameManager(int f_w = 50, int f_h = 50): GameCore(f_w, f_h){

	};

	void setupMenuScene();

	void setupLevelScene();

	void initialize() override {

		//generateRandomMaze();

		//recreate();

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
			spawner.update(dt);
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
	spawner.clear();
	Field::clear();
	//recreate();
	camKeep = CameraKeeper{dynamic_cast<Model*>(this), {-70.0f,-60.0f,-70.0f}};
	MazeUI::manager.clear();
	MazeUI::Window* menuWindow = new MazeUI::Window("Maze game", 0.4f, 0.4f, 0.2f, 0.25f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	menuWindow->addNewItem(new MazeUI::Button("Play!", [this](){
		setup = 2;
	}, 0.19f, 0.05f));		
	menuWindow->addNewItem(new MazeUI::Button("Options", [this, menuWindow](){
		MazeUI::Window* optionsWindow  = new MazeUI::Window("Options", 0.4f, 0.4f, 0.2f, 0.3f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		optionsWindow->addNewItem(new MazeUI::Text("Field options"));
		optionsWindow->addNewItem(new MazeUI::InputBox("width", [this](int nWidth){ this->options.width = nWidth;}, this->options.width, 30.0f, 150.0f));
		optionsWindow->addNewItem(new MazeUI::InputBox("height", [this](int nHeight){ this->options.height = nHeight;}, this->options.height, 30.0f, 150.0f));
		optionsWindow->addNewItem(new MazeUI::Button("Back", [optionsWindow, menuWindow](){ optionsWindow->expired = true; menuWindow->visible = true;}));
		menuWindow->visible = false;
		MazeUI::manager.addNewElement(optionsWindow);
	}, 0.19f, 0.05f));	
	menuWindow->addNewItem(new MazeUI::Button("Exit", [this](){
		quit = true;
	}, 0.19f, 0.05f));

	MazeUI::manager.addNewElement(menuWindow);

	inputHandler.reset();		
}

void GameManager::setupLevelScene(){

	freeGameObjects();
	changeSize(options.width, options.height);
	generateRandomMaze();
	recreate();
	paused = false;

	Cell* init = getRandomCell([](Cell* c){ return c->type == CellType::PATH;});

	player = dynamic_cast<PlayerObject<SingleInstanceModel>*>(addNewGameObject(new PlayerObject<SingleInstanceModel>{init, 5.0f,  glm::vec3{1.0f, 0.0f, 0.0f}, 5.0f}));

	player->onDeath = [this](){
		player = nullptr;
		setup = 1;
	};

	camKeep = CameraKeeper{dynamic_cast<Model*>(player), {-70.0f,-60.0f,-70.0f}};
	camKeep.scaleDisposal(0.5);





/*
	for(int i = 0; i < 250; i++){
		auto freePathRule = [this](Cell* c){ return c->type == CellType::PATH && !isThereObjectsInCell(c);};
		init = getRandomCell(freePathRule);

		addNewGameObject(new CoinObject{init, 5.0f});

		init = getRandomCell(freePathRule);
		
		addNewGameObject(new Cannon<SimpleArrow>{init, 5.0f, {1.0f, 0.0f, 0.0f}, 5.0f, 2, 2.0});
	}
*/


	//This task will spawn 5 Cannon objects on free path cells every second during 20 seconds lifetime 
	spawner.addNewSpawnTask(ObjectSpawner::SpawnInfo{1.0f, 20, [](Cell* par)->GameObject*{ return new Cannon<SingleInstanceModel>(par, 5.0f, {1.0f, 0.0f, 0.0f}, 5.0f, 2, 2.0);},
										       [this](Cell* c){ return c->type == CellType::PATH && !isThereObjectsInCell(c);}, 20.0f});


	//This task will spawn 5 Coin objects on free path cells every second during 60 seconds lifetime 	
	spawner.addNewSpawnTask(ObjectSpawner::SpawnInfo{1.0f, 5, [](Cell* par)->GameObject*{ return new CoinObject(par);},
										       [this](Cell* c){ return c->type == CellType::PATH && !isThereObjectsInCell(c);}, 60.0f});

	int spike_dir = 2;

	spawner.addNewSpawnTask(ObjectSpawner::SpawnInfo{0.5f, 10, [spike_dir](Cell* par)->GameObject*{ return new Spike(par, spike_dir);},
										       [this, spike_dir](Cell* c){ 

										       	std::function<int(Cell*, int)> lenChecker = [this, &lenChecker](Cell* c, int dir)->int{ 
										       		if(c->type != CellType::PATH) 
										       			return 1; 
										       		else 
										       			if(isThereObjectsInCell(c, [](const GameObject* obj){ return obj->getInfo().type == ObjectType::NPC && obj->getInfo().data == -1;})) 
										       				return -getWidth() - getHeight();
										       			else
										       				return 1 + lenChecker(getNeiCell(c, static_cast<Dirs>(dir)), dir);
										       		};

										       	return c->type == CellType::PATH && !isThereObjectsInCell(c) && (lenChecker(c, spike_dir * 2) + lenChecker(c, ((spike_dir - 2) * 2) % 8) - 3 > 5);
										       },5.0f});

	spike_dir = 1;

	spawner.addNewSpawnTask(ObjectSpawner::SpawnInfo{0.5f, 10, [spike_dir](Cell* par)->GameObject*{ return new Spike(par, spike_dir);},
										       [this, spike_dir](Cell* c){ 

										       	std::function<int(Cell*, int)> lenChecker = [this, &lenChecker](Cell* c, int dir)->int{ 
										       		if(c->type != CellType::PATH) 
										       			return 1; 
										       		else 
										       			if(isThereObjectsInCell(c, [](const GameObject* obj){ return obj->getInfo().type == ObjectType::NPC && obj->getInfo().data == -1;})) 
										       				return -getWidth() - getHeight();
										       			else
										       				return 1 + lenChecker(getNeiCell(c, static_cast<Dirs>(dir)), dir);
										       		};

										       	return c->type == CellType::PATH && !isThereObjectsInCell(c) && (lenChecker(c, spike_dir * 2) + lenChecker(c, ((spike_dir + 2) * 2) % 8) - 3 > 5);
										       }, 5.0f});


//	spawner.clear();
	MazeUI::manager.clear();
	MazeUI::Window* statWindow = new MazeUI::Window("Stats", 0.0f, 0.85f, 0.2f, 0.15f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
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

	MazeUI::Window* pauseWindow = new MazeUI::Window("Pause window", 0.4f, 0.45f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse| ImGuiWindowFlags_NoTitleBar);
	pauseWindow->addNewItem(new MazeUI::Text("Game Paused"));
	pauseWindow->visible = false;

	MazeUI::manager.addNewElement(statWindow);
	MazeUI::manager.addNewElement(fpsWindow);
	MazeUI::manager.addNewElement(debugWindow);
	MazeUI::manager.addNewElement(hintWindow);
	MazeUI::manager.addNewElement(pauseWindow);

	inputHandler.reset();		

	inputHandler.onKeyDown = [this, debugWindow, pauseWindow](unsigned char key){ 		
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
			case KEY_ESCAPE:
				paused = !paused;
				pauseWindow->visible = !pauseWindow->visible;
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