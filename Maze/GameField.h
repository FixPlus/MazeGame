#pragma once
#include <vector>
#include <list>
#include <cstdlib>
#include <unordered_map>
#include <iostream>



/*
	MazeGame/Maze/GameField.h


	The game happens here! All in-game procceses are controled
	by classes declared below (such as GameField - core of a game)



*/


enum class CellType {PATH, WALL, ERR, ANY};

enum class Dirs {UP, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN, DOWN_LEFT, LEFT, UP_LEFT};

enum class ObjectType{PLAYER, NPC, COIN, POWERUP, UNKNOWN};

struct ObjectInfo{
	enum ObjectType type;
	int data;
};
										          //	UP     UP_R    RIGHT    D_R     DOWN    D_L      LEFT     UP_L
const std::vector<std::pair<int, int>>  nei_dirs = {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};

class GameObject;

struct Cell {
	int x, y;
	CellType type;
	
	std::list<GameObject*> objects;

	explicit Cell(int ix = 0, int iy = 0, CellType itype = CellType::PATH): x(ix), y(iy), type(itype) {};

	void addNewObject(GameObject* obj){
		objects.push_back(obj);
	};

	void removeObject(GameObject* obj){
		auto it = objects.begin();
		for(int i = 0; i < objects.size(); i++, it++){
			if(obj == *it){
				objects.erase(it);
			}
		}
	}

	virtual ~Cell(){};
};

class CellField;


class GameObject {
protected:
	Cell* parent;
	bool transparent_ = false;
	bool expired = false;
public:	
	float x, y;

	Cell* getCell();
	Cell* getParent(){
		return parent;
	}

	explicit GameObject(float ix = 0, float iy = 0):
	 x(ix), y(iy){ parent = getCell(); getCell()->addNewObject(this);};


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
		parent->removeObject(this);
	};
};



class CellField {
	
	std::vector<Cell> cells;
	std::list<GameObject*> objects;
	int width, height;
	bool isOutOfbounds(int x, int y) const{
		return x < 0 || x >= width || y < 0 || y >= height;
	};


	std::vector<bool> getProbDirections(Cell* cell){
		std::vector<bool> ret = {false};
		if(!cell)
			return ret;

		ret.resize(4);
		
		for(int i = 0; i < 4; i++){
			Cell* nei = getNeiCell(cell, static_cast<enum Dirs>(i * 2));
			
			if(!nei || nei->type == CellType::PATH){
				ret[i] = false;
				continue;
			}

			int count = 0;
			int dir = 0;
			for(int j = 0; j < 4; j++) {
				Cell* neinei = getNeiCell(nei, static_cast<enum Dirs>(j * 2));
				if(!neinei){
					count = 4;
					break;
				}
				if(neinei->type == CellType::PATH){
					dir = j * 2;
					count++;
				}
			}
			
			if(count > 1){
				ret[i] = false;
				continue;
			}

			bool flag = true;
			for(int j = 0; j < 4; j++) {
				int cur_index = j * 2 + 1;
				Cell* neinei = getNeiCell(nei, static_cast<enum Dirs>(cur_index));
				if(neinei->type == CellType::PATH && (cur_index + 1) % 8 != dir && (cur_index - 1) % 8 != dir){
					flag = false;
					break;
				}
			}
			ret[i] = flag;
		}

		return ret;
	}

	Cell* getRandomNewNodeCell();

public:
	explicit CellField(int w = 0, int h = 0): width(w), height(h){
		cells.resize(width * height);
		for(int i = 0; i < height; i++)
			for(int j = 0; j < width; j++){
				cells[i*width + j].x = j;
				cells[i*width + j].y = i;
				cells[i*width + j].type = CellType::PATH;
			}
	};

	void update(float dt) {
		for(auto& object: objects){
			if(object != nullptr){
				object->update(dt);

			for(auto& nei: object->getCell()->objects)
				if(nei != object && object->getParent() == nei->getParent()) 
					object->interact(nei);
			

			}
		}

		for(auto& object: objects)
			if(object->isExpired()){
				delete object;
				object = nullptr;
			}
			
		objects.remove_if([](GameObject* const& obj) -> bool { return obj == nullptr; });
	}

	Cell* getCell(int x, int y){
		return &cells[y * width + x];
	}


	Cell const * getCell(int x, int y) const{
		return &cells[y * width + x];
	}

	GameObject* addNewGameObject(GameObject* object){
		objects.push_back(object);

		return objects.back();
	}

	void setType(int x, int y, CellType type){
		if(isOutOfbounds(x, y))
			return;

		int index = y * width + x;

		cells[index].type = type;
	};
	void clear(){
		for(auto& cell: cells)
			cell.type = CellType::WALL;
	}

	Cell* getNeiCell(Cell* cell, enum Dirs dir) {
		if(!cell)
			return nullptr;
		int neiX = cell->x + nei_dirs[static_cast<int>(dir)].first;
		int neiY = cell->y + nei_dirs[static_cast<int>(dir)].second;
		
		if(isOutOfbounds(neiX, neiY))
			return nullptr;

		int index = neiY * width + neiX;
		return &cells[index];
	}

	Cell const * getNeiCell(Cell const * cell, enum Dirs dir) const {
		if(!cell)
			return nullptr;
		int neiX = cell->x + nei_dirs[static_cast<int>(dir)].first;
		int neiY = cell->y + nei_dirs[static_cast<int>(dir)].second;
		
		if(isOutOfbounds(neiX, neiY))
			return nullptr;

		int index = neiY * width + neiX;
		return &cells[index];

	}

	bool isThereObjectsInCell(int x, int y, std::function<bool(const GameObject*)> rule = [](const GameObject* obj) -> bool{ return true; }) const{
		Cell const* cell = getCell(x, y);
		for(auto object: cell->objects)
			if(rule(object))
				return true;

		return false;
	}

	// up right down left
	std::vector<bool> openSideFaces(int x, int y){
		std::vector<bool> ret;
		if(isOutOfbounds(x, y))
			return ret;

		ret.resize(4, false);
		
		Cell* cur = &cells[y * width + x];
		if(cur->type == CellType::PATH)
			return ret;

		for(int i = 0; i < 8; i += 2){
			Cell* nei = getNeiCell(cur, static_cast<enum Dirs>(i));
			if(nei && nei->type == CellType::PATH){
				ret[i / 2] = true;
			}
		}
		return ret;
	}




	Cell* getRandomCell(enum CellType type){
		while(true){
			int index = rand() % cells.size();
			Cell* ret = &cells[index];
			if(ret->type == type || type == CellType::ANY)
				return ret; 
		}
		return nullptr;
	}

	int countDirectNeighbours(Cell* cell, enum CellType type){
		if(cell == nullptr)
			return 0;
		int ret = 0;
		for(int i = 0; i < 8; i += 2){
			Cell* nei = getNeiCell(cell, static_cast<enum Dirs>(i));
			if(nei == nullptr)
				continue;
			if(nei->type == type)
				ret++;
		}

		return ret;
	}

	bool isStraightWall(Cell* cell){
		if(cell == nullptr || cell->type == CellType::PATH)
			return false;
		int count = 0;
		int neis = 0;
		for(int i = 0; i < 8; i += 2){
			Cell* nei = getNeiCell(cell, static_cast<enum Dirs>(i));
			if(nei == nullptr){
				return false;
			}
			if(nei->type == CellType::PATH){
				neis++;
				count += i;
			}
		}
		return (neis == 2 && (count == 4 || count == 8)) ? true : false;
	}

	void generateOpenSpaceArena(int obstacles = 5){
		clear();
		for(auto& cell: cells){
			if(cell.x != 0 && cell.x != width - 1 && cell.y != 0 && cell.y != height - 1)
				cell.type = CellType::PATH;
		}
		int obst_count = width * height * obstacles / 100;
		//int obstacle_rate = width * obstacles / 20;

		for(int i = 0; i < obst_count; i++){
			Cell* cell = getRandomCell(CellType::PATH);
			cell->type = CellType::WALL;	
		}
	}
	void generateRandomMaze(int straightness = 5){
		clear();
		Cell* cell = getRandomCell(CellType::ANY);

		cell->type = CellType::PATH;
		int prevDir = -1;
		while(true){
			std::vector<bool> probDirs = getProbDirections(cell);
			int count = 0;
			
			for(int i = 0; i < 4; i++){
				bool dir = probDirs[i];
				if(dir){
					count++;
					if(i == prevDir)
						count += straightness;
				}
			}

			if(count == 0) {
				cell = getRandomNewNodeCell();
				if(cell == nullptr)
					break;
				prevDir = -1;
				continue;
			}

			int next_dir = (rand() % count) + 1;
			for(int i = 0; i < 4; i++){
				if(probDirs[i] == true){
					next_dir--;
					if(i == prevDir)
						next_dir -= straightness;
				}
				if(next_dir <= 0){
					cell = getNeiCell(cell, static_cast<enum Dirs>(i * 2));
					cell->type = CellType::PATH;
					prevDir = i;
					break;
				}
			}
		}
	//generating cycles:
		int cycles = width * height / 50;
		for(int i = 0; i < cycles; i++){
			int attemptsPassed = 0;
			do{
				cell = getRandomCell(CellType::WALL);
				attemptsPassed++;
			}while(!isStraightWall(cell) || attemptsPassed < 1000);
			
			cell->type = CellType::PATH;
		}

	};

	std::list<Cell> findPath(int x1, int y1, int x2, int y2,
	 std::function<bool(const Cell*, const Cell*)> rule = 
	 [](const Cell* cell1, const Cell* cell2) -> bool { return cell1->type == CellType::PATH && cell2->type == CellType::PATH; } ) const{
		std::list<Cell> path;
		if(isOutOfbounds(x1, y1) || isOutOfbounds(x2, y2))
			return path;
		const Cell* cur = &cells[y1 * width + x1];
		const Cell* goal = &cells[y2 * width + x2];		

		std::list<const Cell*> frontier;
		std::unordered_map<const Cell*, bool> visited;
		std::unordered_map<const Cell*, const Cell*> came_from;
		
		frontier.emplace_back(cur);
		
		came_from[cur] = nullptr;
		visited[cur] = true;

		bool flag = false;

		while(!frontier.empty()){
			cur = frontier.front();
			frontier.pop_front();

			if(cur == goal){
				flag = true;
				break;
			}

			for(int i = 0; i < 8; i += 2){
				const Cell* nei = getNeiCell(cur, static_cast<enum Dirs>(i));
				if(nei && (visited.find(nei) == visited.end()) && rule(cur, nei)){
					visited[nei] = true;
					came_from[nei] = cur;
					frontier.emplace_back(nei);
				}
			}
		}

		if(flag){
			path.emplace_front(*goal);
			cur = goal;

			while(true){
				const Cell* prev = came_from[cur];
				if(prev == nullptr){
					break;
				}
				cur = prev;
				path.emplace_front(*cur);
			}
		}


		return path;
	}

	CellType getType(int x, int y){
		if(isOutOfbounds(x, y))
			return CellType::ERR;

		int index = y * width + x;

		return cells[index].type;

	};

	int getWidth(){
		return width;
	};

	int getHeight(){
		return height;
	}

	void freeGameObjects(){
		for(auto& object: objects){
			if(object != nullptr){
				delete object;
				object = nullptr;
			}
		}		
	}
	virtual ~CellField(){
	};
};

namespace MazeGame{

extern CellField  gameField;

};

Cell* GameObject::getCell(){
	return MazeGame::gameField.getCell(static_cast<int>(round(x)), static_cast<int>(round(y)));
}


Cell* CellField::getRandomNewNodeCell(){
	Cell* cur = getRandomCell(CellType::PATH);
	std::list<Cell*> frontier;
	std::unordered_map<Cell*, bool> visited;

	frontier.push_back(cur);
	visited[cur] = true;

	while(!frontier.empty()){
		cur = frontier.front();
		frontier.pop_front();

		std::vector<bool> dirs = getProbDirections(cur);
		int count  = 0;
		for(auto dir: dirs)
			if(dir)
				count++;
		if(count > 0)
			return cur;

		for(int i = 0; i < 8; i += 2){
			Cell* nei = getNeiCell(cur, static_cast<enum Dirs>(i));
			if(nei && nei->type == CellType::PATH && visited.find(nei) == visited.end()){
				visited[nei] = true;
				frontier.push_back(nei);
			}
		}
	}

	return nullptr;

}




//GAME OBJECTS


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
		Cell* dest = MazeGame::gameField.getNeiCell(getCell(), static_cast<enum Dirs>(dir));
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

		Cell* dest = MazeGame::gameField.getCell(x, y);

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
				parent  =destination;

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
