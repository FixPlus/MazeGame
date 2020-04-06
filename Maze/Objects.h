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
	explicit DirectedObject(int idir = 2): dir(idir) {};


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

};

