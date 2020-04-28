#pragma once


#include "Models.h"
#include "InputHandler.h"


namespace MazeGame{

class GameObject {
protected:
	Cell* parent;
	bool transparent_ = false;
	bool expired = false;
public:	

	static int count;

	float x, y;

	Cell* getCell();
	Cell* getParent(){
		return parent;
	}

	explicit GameObject(Cell* par): x(par->x), y(par->y), parent(par){ parent->addNewObject(this); count++;};

	bool isExpired() const{
		return expired;
	}


	bool isTransparent() const{
		return transparent_;
	};

	virtual void update(float dt) = 0;

	virtual void printObjectInfo() const{
	};

	virtual ObjectInfo getInfo() const{
		return {ObjectType::UNKNOWN, 0};
	};

	virtual void interact(GameObject* another){

	};


	virtual ~GameObject(){
		count--;
		parent->removeObject(this);
	};


};

class GameCore: public ::triGraphic::Field{

	std::list<GameObject*> objects;
protected:

	bool quit = false;
	InputHandler inputHandler;

public:
	GameCore(int f_w = 50, int f_h = 50): ::triGraphic::Field(f_w, f_h){};

	GameCore& operator=(GameCore&& another){
		if(&another != this){
			::triGraphic::Field::operator=(static_cast<::triGraphic::Field&&>(another));
			objects = another.objects;
			another.objects.clear();
		}
		return *this;
	}

	virtual void update(float dt) {
		for(auto& object: objects)
			if(object != nullptr)
				object->update(dt);


		for(auto& object: objects)
			for(auto& nei: object->getParent()->objects)
				if(nei != object && object->getParent() == nei->getParent()) 
					object->interact(nei);

		for(auto& object: objects)
			if(object->isExpired()){
				delete object;
				object = nullptr;
			}
			
		objects.remove_if([](GameObject* const& obj) -> bool { return obj == nullptr; });
	}

	virtual void initialize() = 0;


	GameObject* addNewGameObject(GameObject* object){
		objects.push_back(object);

		return objects.back();
	}

	void freeGameObjects(){
		for(auto& object: objects){
			if(object != nullptr){
				delete object;
			}
		}
		objects.resize(0);	
	}

	InputHandler& getInputHandler(){
		return inputHandler;
	}

	virtual ~GameCore(){
		freeGameObjects();
	}

	bool shouldQuit(){
		return quit;
	}

	static bool isThereObjectsInCell(Cell const* cell, std::function<bool(const GameObject*)> rule = [](const GameObject* obj) -> bool{ return true; }) {
		for(auto object: cell->objects)
			if(rule(object))
				return true;

		return false;
	}

};




};