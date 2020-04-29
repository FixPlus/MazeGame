#pragma once
#include "GameField.h"
#include "Models.h"
#include "GameCore.h"

/*

	/MazeGame/Maze/Objects.h

	Here lie declarations of all in-game objects classes

	This classes are usualy are inheritated from
	two virtual bases: GameObject (declared in "GameField.h")
	and Model ( delclared in "Models.h"). It is mean that they
	as are part of a gameField as they are 3D graphical models

	In order to set it as a rule a ModeledObject class was made (declared below)
	All in-game non-abstract objects classes must be inheritated from it.

	===========================================================================


*/

namespace MazeGame{

extern GameCore* gameCore;
extern bool should_update_static_vertices; 

using namespace triGraphic;



Cell* GameObject::getCell(){
	return gameCore->getCell(static_cast<int>(round(x)), static_cast<int>(round(y)));
}


class HealthObject: public virtual GameObject {
	float max_hp_;
	float hp_;
public:
	explicit HealthObject(float max_hp = 100.0f): max_hp_(max_hp), hp_(max_hp){};

	void modifyHP(float count){
		hp_ += count;
		if(hp_ > max_hp_)
			hp_ = max_hp_;

		if(hp_ <= 0.0f + __FLT_EPSILON__){
			hp_ = 0.0f;
			expired = true;
		}
	}

	float const& hp() const{
		return hp_;
	}
	float max_hp() const{
		return max_hp_;
	}
};


class DynamicObject: public virtual GameObject {
	bool moving = false;
	int xFrom, yFrom, xDest, yDest;
protected:
	Cell* destination = NULL;
	float progression = 0.0f;
public:
	float speed = 1.0f;

	explicit DynamicObject(int ispeed = 1.0f): speed(ispeed){};


	virtual bool canMove(Cell const* from, Cell const* into) = 0;

	void moveObj(int dir){
		if(moving)
			return;
		Cell* dest = gameCore->getNeiCell(getCell(), static_cast<enum Dirs>(dir));
		if(dest == nullptr)
			return;
	

		if(canMove(getCell(), dest)){
			destination = dest;
			destination->addNewObject(this);

			xFrom = getCell()->x;
			yFrom = getCell()->y;

			xDest = destination->x;
			yDest = destination->y;


			progression = 0.0f;
			moving = true;
		}
	};
	void moveObj(int x, int y){
		if(moving)
			return;

		Cell* dest = gameCore->getCell(x, y);

		if(dest == nullptr)
			return;

		if(canMove(getCell(), dest)){
			destination = dest;
			destination->addNewObject(this);

			xFrom = getCell()->x;
			yFrom = getCell()->y;

			xDest = destination->x;
			yDest = destination->y;

			progression = 0.0f;
			moving = true;
		}

	};

	void update(float dt) override{
		if(moving){
			progression += dt * speed;
			
			x = static_cast<float>(xFrom) * (1.0f - progression) + static_cast<float>(xDest) * progression;
			y = static_cast<float>(yFrom) * (1.0f - progression) + static_cast<float>(yDest) * progression;
			if(progression >= 1.0f){
				parent->removeObject(this);
				parent = destination;

				x = destination->x;
				y = destination->y;
				moving = false;
				progression -= 1.0f;
			}
		}
	}
	
	bool isMoving(){
		return moving;
	}

	~DynamicObject(){
		if(destination != nullptr && destination != parent)
			destination->removeObject(this);
	}
};



class ModeledObject: public virtual GameObject, public virtual Model {
protected:
	void setInPosition(){
		set({(x + 0.5f) * gameCore->getCellSize(), gameCore->getZeroLevel() - 5.0f,(y + 0.5f) * gameCore->getCellSize()});		
	}
public:
	ModeledObject() {
	};

	void update(float dt) override{
		rotate(dt);
	}

};



class DynamicModeledObject: public virtual ModeledObject, public DynamicObject {
public:
	explicit DynamicModeledObject(float ispeed = 1.0f):
	DynamicObject(ispeed){
		speed = ispeed;

	};

	bool canMove(Cell const* from, Cell const* into) override{
		return (into && into->type == CellType::PATH && !GameCore::isThereObjectsInCell(into)) ? true : false;
	}

	void update(float dt) override{
		ModeledObject::update(dt);
		DynamicObject::update(dt);
		setInPosition();
	}

	DynamicModeledObject& operator=(DynamicModeledObject&& rhs){
		if(&rhs != this){
			DynamicObject::operator=(rhs);
			Model::operator=(rhs);
			speed = rhs.speed;

		}
		return *this;
	}

};

class DirectedObject: public virtual ModeledObject {
	float progress = 0.0f;
	bool clockwise = true;
	float rotSpeed = 500.0f;
	int nDir = 2;
	int delta = 0;
protected:
	bool onChangingDirection = false;
	int dir = 2;
public:
	explicit DirectedObject(int idir = 2): dir(idir) { };

	void changeDirection(int newDir){
		if(onChangingDirection)
			return;

		int div = newDir % 4 - dir;
		if(div == 0)
			return;
		//rotate({0.0f, 1.0f, 0.0f}, -div * 90.0f);
		clockwise = div > 0 && div < 2 || div == -3;
		delta = (abs(div) == 2) ? 2 : 1;
		nDir = newDir % 4;
		onChangingDirection = true;
	};

	void update(float dt) override{
		ModeledObject::update(dt);
		if(onChangingDirection){
			progress += dt * rotSpeed;
			rotate({0.0f, 1.0f, 0.0f}, - dt * rotSpeed * (clockwise ? 1.0f : -1.0f));
			if(progress > 90.0f * (delta)){
				progress - 90.0f * delta;
				rotate({0.0f, 1.0f, 0.0f}, (progress - 90.0f * delta) * (clockwise ? 1.0f : -1.0f));
				onChangingDirection = false;
				dir = nDir;
			//	faceOnAxis(dirNormal(dir));
				progress = 0.0f;
			}

		}
	}

	bool isChangingDirection() const{
		return onChangingDirection;
	}

	int const& getDir() const{
		return dir;
	}

	void printObjectInfo() const override{
		std::cout << "DirectedObject" << std::endl;
	}

	DirectedObject& operator=(DirectedObject&& rhs){
		if(&rhs != this){
			ModeledObject::operator=(rhs);
			dir = rhs.dir;

		}
		return *this;
	}

};

class DynamicDirectedObject: public DirectedObject, public DynamicModeledObject {
	
public:
	explicit DynamicDirectedObject(int idir = 2, float ispeed = 1.0f):
	DirectedObject(idir), DynamicModeledObject(ispeed) {

	}

	void update(float dt) override{
		DirectedObject::update(dt);
		DynamicModeledObject::update(dt);
	}
	bool canMove(Cell const* from, Cell const* into) override{
		return (into && into->type == CellType::PATH && !GameCore::isThereObjectsInCell(into, [](GameObject const * obj) -> bool{ return !obj->isTransparent();})) ? true : false;
	}

	void moveInDirection(){
		if(!onChangingDirection)
		moveObj(dir * 2);
	};

};








// INSTANCED OBJECTS DOWN THERE





class LoadableDynamicModeledObject: public DynamicModeledObject, public DynamicLodableModel{
public:
	explicit LoadableDynamicModeledObject(std::string filename,  Cell* par, float ispeed = 1.0f, float scale = 1.0f):
	Model(), GameObject(par), DynamicModeledObject(ispeed), DynamicLodableModel(filename, scale){ setInPosition();};
};

class CoinObject: public ModeledObject, public CoinModel {
	int nominal = 10;
public:
	static int count;
	explicit CoinObject(Cell* par, float size = 5.0f): 
	Model(), GameObject(par), CoinModel(size), ModeledObject(){ transparent_ = true; count++; setInPosition();};

	void printObjectInfo() const override{
		std::cout << "Coin" << std::endl;
	}


	ObjectInfo getInfo() const override{
		return {ObjectType::COIN, nominal};
	};

	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::PLAYER: {
				expired = true;
				break;
			}
		}
	};

	~CoinObject(){
		count--;
	}
};









template <typename AnyDynamicModel>
class Seeker: public DynamicModeledObject, public AnyDynamicModel{
	GameObject* aim;
	std::list<Cell> path;
	int counter = 0;
public:

	explicit Seeker(Cell* par, float size = 5.0f, float ispeed = 1.0f, glm::vec3 color = {1.0f, 1.0f, 1.0f}, GameObject* iaim = NULL): 
	GameObject(par), Model(), AnyDynamicModel(size, color), DynamicModeledObject(ispeed), aim(iaim){
		speed = ispeed;
	};

	bool canMove(Cell const* from, Cell const* into) override{
		return (into && into->type == CellType::PATH && !GameCore::isThereObjectsInCell(into, [this](GameObject const * obj) -> bool{ return !obj->isTransparent() && !(obj == aim);}));
	}

	void setAim(GameObject* newAim){
		aim = newAim;
	}

	void update(float dt) override{
		counter++;
		DynamicModeledObject::update(dt);
		if(!isMoving()){
			if(!aim){
				return;
			}
			if(counter > 20){
				path = gameCore->findPath(x, y, aim->x, aim->y); /*, [this](Cell const* from, Cell const* into) -> bool{
					return (into && into->type == CellType::PATH && !MazeGame::gameField.isThereObjectsInCell(into->x, into->y, 
						[this](GameObject const * obj) -> bool{ return !obj->isTransparent() && !(obj == aim);})) ? true : false;
				});
				*/

				if(!path.empty())
					path.pop_front();
				counter = 0;
			}

			if(!path.empty()){
				Cell next = path.front();
				path.pop_front();
				moveObj(next.x, next.y);
				if(!isMoving()){
					path.push_front(next);
				}
				if(path.empty())
					counter += 20;
			}
		}
	};

	void printObjectInfo() const override {
		std::cout << "Seeker" << std::endl;
	}

	ObjectInfo getInfo() const override{
		return {ObjectType::NPC, 0};
	};

	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::PLAYER: {
				expired = true;
				break;
			}
		}
	};

};


template <typename AnyDynamicModel>
class Bullet: public DynamicDirectedObject, public AnyDynamicModel{
	int id;
public:
	explicit Bullet(Cell* par, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f}, float ispeed = 1.0f, int idir = 2, int iid = 0):
	GameObject(par), Model(), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(size, color), id(iid){ transparent_ = true; setInPosition(); };

	ObjectInfo getInfo() const override{
		return {ObjectType::POWERUP, id};
	};
	bool canMove(Cell const* from, Cell const* into) override{
		return (into);
	}

	void update(float dt) override{
		if(parent->type == CellType::WALL){
		//	if(id == 0)
		//		MazeGame::gameField.setType(parent->x, parent->y, CellType::PATH);
			expired = true;
		}
		if(!isMoving())
			moveInDirection();

		DynamicDirectedObject::update(dt);
	}


	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::PLAYER: {
				//std::cout << "SHOT" << std::endl;
				if(id == 0)
					return;
				break;
			}
			case ObjectType::NPC: {
				if(info.data == id)
					return;
/*			speed -= 0.2f;
				if(speed < 0.2f)
					speed = 0.2f;
*/
				break;
			}
			case ObjectType::POWERUP:{
				return;
			}
			default: return;
		}
		expired = true;
	}

};

template <typename AnyDynamicModel>
class Cannon: public DynamicDirectedObject, public AnyDynamicModel {
	
	float launch_timer = 0.0;
	float fire_rate;
	float state_timer = 0.0;
	float next_state_time = 1.0;
	int id;
	static int next_id;
	std::list<std::function<void(void)>> actions;
	enum CannonState {CS_FIRING, CS_GATHERING, CS_IDLE} state = CS_FIRING;

	static glm::vec3 constexpr stateColors[3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
public:
	explicit Cannon(Cell* par, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f},float ispeed = 5.0, int idir = 2, float fr = 2.0):
	GameObject(par), Model(), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(size, color), fire_rate(fr) { setInPosition(); id = next_id++;};

	ObjectInfo getInfo() const override{
		return {ObjectType::NPC, id};
	};

	void update(float dt) override{
		DynamicDirectedObject::update(dt);
		if(!isMoving() && !isChangingDirection() && !actions.empty()){
			(*actions.begin())();
			actions.pop_front();
		}

		switch(state){
			case CS_FIRING:{
				launch_timer += dt;
				if(launch_timer > 1.0 / fire_rate){
					actions.push_back([this](){gameCore->addNewGameObject(new Bullet<SimpleOctagon>{parent, 1.0f, {0.0f, 0.0f, 0.0f}, 10.0f, dir, id});});
					launch_timer = 0.0;
				}
				break;
			}
			case CS_GATHERING:{
				if(isMoving() || isChangingDirection())
					break;
				bool prob_dirs[4] = {false};
				int count_dirs = 0;
				bool back_is_possible = false;
				for(int i = 0; i < 4; ++i){

					Cell* dest = gameCore->getNeiCell(getCell(), static_cast<enum Dirs>(i * 2));
					if(dest && canMove(parent, dest)){
						if(i != ((dir + 2) % 4)){
							count_dirs++;
							prob_dirs[i] = true;
						}
						else
							back_is_possible =  true;
					}
				}

				if(count_dirs == 0)
					if(back_is_possible){
						prob_dirs[((dir + 2) % 4)] = true;
						count_dirs = 1;
					}
					else{
						state_timer = next_state_time;
						break;
					}

				int next_dir = rand() % count_dirs + 1;
				int i;
				for(i = 0; next_dir != 0; i++){
					if(prob_dirs[i])
						next_dir--;
				}
				i--;
				actions.push_back([this, i](){changeDirection(i);});
				actions.push_back([this](){moveInDirection();});
				break;
			}
			case CS_IDLE:{
				break;
			}
		}
		state_timer += dt;
		if(state_timer >= next_state_time){
			state_timer = 0.0;
			next_state_time = static_cast<float>(rand() % 5 + 5) / 5.0f;
			state = static_cast<enum CannonState>((static_cast<int>(state) + rand() % 2) % 3);
			setColor(stateColors[static_cast<int>(state)]);
		}
	}

	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::PLAYER: {
			//	expired = true;
				break;
			}
			case ObjectType::POWERUP: {
				if(info.data != id)
					expired = true;
/*			speed -= 0.2f;
				if(speed < 0.2f)
					speed = 0.2f;
*/
				break;
			}
			default:{}
		}
	};

};

};

