#pragma once
#include "GameField.h"
#include "Models.h"


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

extern ::triGraphic::FieldModel* fieldModel;


};
namespace triGraphic{

class ModeledObject: public virtual GameObject, public virtual Model {
protected:
	void setInPosition(){
		set({(x + 0.5f) * MazeGame::fieldModel->getCellSize(), MazeGame::fieldModel->getZeroLevel() - 5.0f,(y + 0.5f) * MazeGame::fieldModel->getCellSize()});		
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
		return (into && into->type == CellType::PATH && !MazeGame::gameField.isThereObjectsInCell(into->x, into->y)) ? true : false;
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
protected:
	int dir = 2;
public:
	explicit DirectedObject(int idir = 2): dir(idir) { };

	void changeDirection(int newDir){
		int div = newDir % 4 - dir;
		rotate({0.0f, 1.0f, 0.0f}, -div * 90.0f);
		dir = newDir % 4;
	};




	int getDir(){
		return dir;
	}

	void printObjectInfo() const override{
		std::cout << "DirectedObject" << std::endl;
	}

	DirectedObject& operator=(DirectedObject&& rhs){
		if(&rhs != this){
			ModeledObject::operator=(rhs);

		}
		return *this;
	}

};

class DynamicDirectedObject: public DirectedObject, public DynamicModeledObject {
	
public:
	explicit DynamicDirectedObject(int idir = 2, float ispeed = 1.0f):
	DirectedObject(idir), DynamicModeledObject(ispeed) {

	}

	bool canMove(Cell const* from, Cell const* into) override{
		return (into && into->type == CellType::PATH && !MazeGame::gameField.isThereObjectsInCell(into->x, into->y, [](GameObject const * obj) -> bool{ return !obj->isTransparent();})) ? true : false;
	}

	void moveInDirection(){
		moveObj(dir * 2);
	};

};








// INSTANCED OBJECTS DOWN THERE





class LoadableDynamicModeledObject: public DynamicModeledObject, public DynamicLodableModel{
public:
	explicit LoadableDynamicModeledObject(std::string filename,  float ix = 0, float iy = 0, float ispeed = 1.0f, float scale = 1.0f):
	Model(), GameObject(ix, iy), DynamicModeledObject(ispeed), DynamicLodableModel(filename, scale){ setInPosition();};
};

class CoinObject: public ModeledObject, public CoinModel {
	int nominal = 10;
public:
	static int count;
	explicit CoinObject(float ix = 0, float iy = 0, float size = 5.0f): 
	Model(), GameObject(ix, iy), CoinModel(size), ModeledObject(){ transparent_ = true; count++; setInPosition();};

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
class PlayerObject: public DynamicDirectedObject, public AnyDynamicModel {
public:
	explicit PlayerObject(float ix = 0, float iy = 0, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f}, float ispeed = 1.0f, int idir = 2):
	GameObject(ix, iy), Model(), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(size, color){ };

	ObjectInfo getInfo() const override{
		return {ObjectType::PLAYER, 0};
	};

	void interact(GameObject* another) override{
		ObjectInfo info = another->getInfo();
		switch(info.type){
			case ObjectType::COIN: {
				speed += 0.2f;
				break;
			}
			case ObjectType::NPC: {
/*			speed -= 0.2f;
				if(speed < 0.2f)
					speed = 0.2f;
*/
				break;
			}
		}
	};

};



template <typename AnyDynamicModel>
class Seeker: public DynamicModeledObject, public AnyDynamicModel{
	GameObject* aim;
	std::list<Cell> path;
	int counter = 0;
public:

	explicit Seeker(int ix = 0, int iy = 0, float size = 5.0f, float ispeed = 1.0f, glm::vec3 color = {1.0f, 1.0f, 1.0f}, GameObject* iaim = NULL): 
	GameObject(ix, iy), Model(), AnyDynamicModel(size, color), DynamicModeledObject(ispeed), aim(iaim){
		speed = ispeed;
	};

	bool canMove(Cell const* from, Cell const* into) override{
		return (into && into->type == CellType::PATH && !MazeGame::gameField.isThereObjectsInCell(into->x, into->y, [this](GameObject const * obj) -> bool{ return !obj->isTransparent() && !(obj == aim);}));
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
				path = MazeGame::gameField.findPath(x, y, aim->x, aim->y); /*, [this](Cell const* from, Cell const* into) -> bool{
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
	explicit Bullet(float ix = 0, float iy = 0, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f}, float ispeed = 1.0f, int idir = 2, int iid = 0):
	GameObject(ix, iy), Model(), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(size, color), id(iid){ transparent_ = true; setInPosition(); };

	ObjectInfo getInfo() const override{
		return {ObjectType::POWERUP, id};
	};
	bool canMove(Cell const* from, Cell const* into) override{
		return (into);
	}

	void update(float dt) override{
		if(parent->type == CellType::WALL){
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
				if(info.data == id || info.data == 2)
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
	enum CannonState {CS_FIRING, CS_GATHERING, CS_IDLE} state = CS_FIRING;

	static glm::vec3 constexpr stateColors[3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
public:
	explicit Cannon(float ix = 0, float iy = 0, float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f},float ispeed = 5.0, int idir = 2, float fr = 2.0):
	GameObject(ix, iy), Model(), DynamicDirectedObject(idir, ispeed), AnyDynamicModel(size, color), fire_rate(fr) { setInPosition(); id = next_id++;};

	ObjectInfo getInfo() const override{
		return {ObjectType::NPC, id};
	};

	void update(float dt) override{
		DynamicDirectedObject::update(dt);
		switch(state){
			case CS_FIRING:{
				launch_timer += dt;
				if(launch_timer > 1.0 / fire_rate){
					MazeGame::gameField.addNewGameObject(new Bullet<SimpleOctagon>{static_cast<float>(parent->x), static_cast<float>(parent->y), 1.0f, {0.0f, 0.0f, 0.0f}, 10.0f, dir, id});
					launch_timer = 0.0;
				}
				break;
			}
			case CS_GATHERING:{
				if(isMoving())
					break;
				bool prob_dirs[4] = {false};
				int count_dirs = 0;
				bool back_is_possible = false;
				for(int i = 0; i < 4; ++i){

					Cell* dest = MazeGame::gameField.getNeiCell(getCell(), static_cast<enum Dirs>(i * 2));
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
				changeDirection(i);
				moveInDirection();
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

