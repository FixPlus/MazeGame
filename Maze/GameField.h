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


namespace MazeGame{


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
				return;
			}
		}
	}

	virtual ~Cell(){};
};



extern bool should_update_static_vertices;

class CellField{
	
	std::vector<Cell> cells;
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

	CellField& operator=(CellField&& another){
		if(&another != this){
			cells = another.cells;
			another.cells.clear();
			width = another.width;
			height = another.height;
		}
		return *this;
	}

	Cell* getCell(int x, int y){
		return &cells[y * width + x];
	}


	Cell const * getCell(int x, int y) const{
		return &cells[y * width + x];
	}


	void setType(int x, int y, CellType type){
		if(isOutOfbounds(x, y) || (x == 0 || x == width - 1) || (y == 0 || y == height - 1))
			return;

		int index = y * width + x;

		cells[index].type = type;

		MazeGame::should_update_static_vertices = true;

	};

	void clear(CellType type = CellType::WALL){
		for(auto& cell: cells)
			cell.type = type;
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


	// up right down left
	std::vector<bool> openSideFaces(int x, int y) const{
		std::vector<bool> ret;
		if(isOutOfbounds(x, y))
			return ret;

		ret.resize(4, false);
		
		Cell const* cur = &cells[y * width + x];
		if(cur->type == CellType::PATH)
			return ret;

		for(int i = 0; i < 8; i += 2){
			Cell const* nei = getNeiCell(cur, static_cast<enum Dirs>(i));
			if(nei && nei->type == CellType::PATH){
				ret[i / 2] = true;
			}
		}
		return ret;
	}




	Cell* getRandomCell(std::function<bool(Cell*)> rule = [](Cell*){ return true;}){
		while(true){
			int index = rand() % cells.size();
			Cell* ret = &cells[index];
			if(rule(ret))
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
			Cell* cell = getRandomCell([](Cell* c){ return c->type == CellType::PATH;});
			cell->type = CellType::WALL;	
		}
	}
	
	void generateRandomMaze(int straightness = 5, float cycleness = 1.0){
		clear();
		Cell* cell = getRandomCell();

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
		int cycles = width * height / 50 * cycleness;
		for(int i = 0; i < cycles; i++){
			int attemptsPassed = 0;
			do{
				cell = getRandomCell([](Cell* c){ return c->type == CellType::WALL;});
				attemptsPassed++;
			}while(!isStraightWall(cell) && attemptsPassed < 1000);
			
			if(isStraightWall(cell))
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

	int const& getWidth() const{
		return width;
	};

	int const& getHeight() const{
		return height;
	}


	int polysRequested() const{
		int count = 0;
		for(int x = 0; x < getWidth(); x++)
			for(int y = 0; y < getHeight(); y++){
				count++;
				std::vector<bool> sides= openSideFaces(x,y);
				for(auto side: sides)
					if(side)
						count++;
			}
		return count * 2;
	}


	virtual ~CellField(){
	};
};







Cell* CellField::getRandomNewNodeCell(){
	Cell* cur = getRandomCell([](Cell* c){ return c->type == CellType::PATH;});
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




};