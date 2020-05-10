#pragma once
#include "InstanceData.h"
#include "drawer.h"
#include "Rotatible.h"
#include "GameField.h"
#include "ModelList.h"

/*
	MazeGame/Maze/Models.h

	Here lie all model classes (oh, really? ^_^)
	They all are inheritated from a virtual base class Model(declared below)

	Model uses DrawingTriangle(s) (Declared and defined in DrawableTriangle.h)
	to make a 3D graphical objects that would be then drawn on screen.
	To recieve the triangles it uses DTManager (Declared and defined in DTManager.h)
	

*/

namespace MazeGame {

};



extern triGraphic::Drawer* drawer;

namespace triGraphic{



struct Model : public Rotatible{

	Model(){

	}

	Model(Model const &another) = delete;




//	Model& virtual operator=(Model&& another) = 0;


	virtual void set(glm::vec3 const &position) = 0;

	virtual void move(glm::vec3 const &shift) = 0;

	virtual void setColor(glm::vec3 newColor) = 0;

	virtual glm::vec3 getPosition() = 0;

	virtual void scale(float mult) = 0;


	virtual void rotate(float dt) = 0;

	virtual void rotate(glm::vec3 rotAxis, float angle) = 0;

	virtual void faceOnAxis(glm::vec3 axis) = 0;

//	virtual void initialize(vertIterator initVertIt) = 0;
	virtual ~Model(){

	};
};


class SingleInstanceModel: public virtual Model{
	InstanceView const* instance_;
public:
	SingleInstanceModel(enum ::MazeGame::ModelName modName, float scale): instance_(drawer->addInstance(static_cast<int>(modName))){ instance_->instance()->scale = scale;};
	void set(glm::vec3 const &position) override{
		instance_->instance()->pos = position;
	};

	void move(glm::vec3 const &shift) override{
		instance_->instance()->pos += shift;	
	};

	void setColor(glm::vec3 newColor) override{};

	glm::vec3 getPosition() override{
		return instance_->instance()->pos;
	};

	void scale(float mult) override{
		instance_->instance()->scale = mult;
	};

	void rotate(glm::vec3 rotAxis, float angle) override{
		glm::fquat instRot{instance_->instance()->rot};
		instRot = glm::rotate(instRot, glm::radians(angle), rotAxis);
		instRot = glm::normalize(instRot);
		instance_->instance()->rot = glm::eulerAngles(instRot);
	};



	void rotate(float dt) override{
		//instance_->instance()->rot += glm::vec3(0.0f, 1.0f * dt, 1.0f * dt);
		for(auto& rot: rotations){
			glm::fquat instRot{instance_->instance()->rot};
			instance_->instance()->rot = glm::vec3{0.0f, 0.0f, 0.0f}; 
			rotate(rot.first, rot.second * dt);
			rotate(glm::axis(instRot), glm::degrees(glm::angle(instRot)));
		}
	};


	void faceOnAxis(glm::vec3 axis) override{
		axis = glm::normalize(axis);
/*		glm::vec3 xzProj = glm::vec3{axis.x, 0.0f, axis.z};
		if(glm::length(xzProj) > __FLT_EPSILON__){
			xzProj = glm::normalize(xzProj);
		}

		float xangle =  acos(glm::dot(xzProj, glm::vec3{1.0f, 0.0f, 0.0f}));
		if(glm::dot(xzProj, glm::vec3{0.0f, 0.0f, 1.0f}) < 0.0f)
			xangle = -xangle;
		


		float yangle = asin(glm::dot(axis, glm::vec3{0.0f, 1.0f, 0.0f}));
		instance_->instance()->rot = glm::vec3{xangle, yangle, 0.0f};
*/
		glm::fquat instRot{glm::vec3{0.0f, 0.0f, 0.0f}};
		instRot = glm::rotate(instRot, acos(glm::dot(axis, glm::vec3{1.0f, 0.0f, 0.0f})), glm::cross(axis, glm::vec3{1.0f, 0.0f, 0.0f}));
		instRot = glm::normalize(instRot);
		instance_->instance()->rot = glm::eulerAngles(instRot);

	};

	~SingleInstanceModel(){
		drawer->returnInstance(instance_);
	}
};

const glm::vec3 VERTICAL_NORM = {0.0f, -1.0f, 0.0f};
const glm::vec3 EAST_NORM = {-1.0f, 0.0f, 0.0f};
const glm::vec3 WEST_NORM = {1.0f, 0.0f, 0.0f};
const glm::vec3 NORTH_NORM = {0.0f, 0.0f, -1.0f};
const glm::vec3 SOUTH_NORM = {0.0f, 0.0f, 1.0f};

const float zeroLevel = 10.0f;

const glm::vec3 dirNormal(int dir){
	switch(dir){
		case 0: return SOUTH_NORM;
		case 1: return EAST_NORM;
		case 2: return NORTH_NORM;
		case 3:	return WEST_NORM;
	}
	return VERTICAL_NORM;
}


class Field: public virtual Model, public MazeGame::CellField{

	std::vector<InstanceView const*> walls;
	std::vector<InstanceView const*> paths;


	float cellSize = 10.0;//, wallHeight = 8.0;




public:
	
	

	void set(glm::vec3 const &position) {};

	void move(glm::vec3 const &shift) {};

	void setColor(glm::vec3 newColor) {};

	glm::vec3 getPosition() { return glm::vec3{0.0f, 0.0f, 0.0f};};

	void scale(float mult) {};


	void rotate(float dt){};

	void rotate(glm::vec3 rotAxis, float angle) {};

	void faceOnAxis(glm::vec3 axis) {};

	void recreate(){
		for(auto& wall: walls)
			drawer->returnInstance(wall);
		for(auto& path: paths)
			drawer->returnInstance(path);

		for(int i = 0; i < getWidth(); i++)
			for(int j = 0; j < getHeight(); j++)
				if(getCell(i, j)->type == MazeGame::CellType::WALL){
					walls.emplace_back(drawer->addInstance(MazeGame::M_WALL));
					InstanceData* instance = (*(--walls.end()))->instance();
					instance->pos = glm::vec3{i * cellSize, getZeroLevel() - cellSize / 2.0f, j * cellSize};
					instance->scale = cellSize;
				}
				else{
					paths.emplace_back(drawer->addInstance(MazeGame::M_PATH));
					InstanceData* instance = (*(--paths.end()))->instance();
					instance->pos = glm::vec3{i * cellSize, getZeroLevel() + cellSize / 2.0f, j * cellSize};
					instance->scale = cellSize;
				}
				std::cout << "Field made with " << walls.size() << " walls and " << paths.size() << " paths" << std::endl;

	}

	Field const& operator=(Field const& another) = delete;

	Field& operator=(Field&& another){
		CellField::operator=(static_cast<CellField&&>(another));
		if(&another != this){
			for(auto& wall: walls){
				drawer->returnInstance(wall);
			}
			for(auto& path: paths)
				drawer->returnInstance(path);
			paths.clear();
			walls.clear();
			walls = another.walls;
			paths = another.paths;
			another.walls.clear();
			another.paths.clear();
			cellSize = another.cellSize;
		}
		return *this;
	}

	float getCellSize() const{
		return cellSize;
	}


	float getZeroLevel() const {
		return zeroLevel;
	}

	Field(int w = 0, int h = 0) : Model(), CellField(w, h){

	};

	~Field(){
		for(auto& wall: walls)
			drawer->returnInstance(wall);
		for(auto& path: paths)
			drawer->returnInstance(path);
	}
};



};
