#pragma once
#include "DrawableTriangle.h"
#include "DTManager.h"
#include "Rotatible.h"


/*
	MazeGame/Maze/Models.h

	Here lie all model classes (oh, really? ^_^)
	They all are inheritated from a virtual base class Model(declared below)

	Model uses DrawingTriangle(s) (Declared and defined in DrawableTriangle.h)
	to make a 3D graphical objects that would be then drawn on screen.
	To recieve the triangles it uses DTManager (Declared and defined in DTManager.h)
	

*/



namespace MazeGame{
	extern ::triGraphic::DTManager triManager;
};

namespace triGraphic{

using triIterator = std::vector<DrawableTriangle>::iterator;


class Model : public Rotatible{
protected:
	std::vector<DrawableTriangle> triangles;
	int size_;
	glm::vec3 node = {0.0f, 0.0f, 0.0f};
	void virtual setup() = 0;

public:
	Model(){

	}

	Model(Model const &another) = delete;


	bool operator==(Model const& another) const{
		return another.triangles == triangles;
	}


//	Model& virtual operator=(Model&& another) = 0;

	int size() const{
		return size_;
	}



	void set(glm::vec3 const &position){
		glm::vec3 shift = position - node;
		for(int i = 0; i < size_; i++)
			triangles[i].move(shift);

		node = position;

		for(auto& rot: rotations){
			rot.first.first += shift;
			rot.first.second += shift;

		}

	}

	void move(glm::vec3 const &shift){
		for(int i = 0; i < size_; i++)
			triangles[i].move(shift);

		node += shift;

		for(auto& rot: rotations){
			rot.first.first += shift;
			rot.first.second += shift;

		}
	}

	void setColor(glm::vec3 newColor){
		for(int i = 0; i < size_; i++){
			triangles[i].setColor(newColor);
		}
	}

	glm::vec3 getPosition(){
		return node;
	}

	void scale(float mult){
		for(int i = 0; i < size_; i++){
			auto& tri = triangles[i];
			for(int j = 0; j < 3; j++){
				tri.vertex(j).position -= node;
				tri.vertex(j).position *= mult;
				tri.vertex(j).position += node;

			}
		}
	}


	void rotate(float dt) {
		for(int i = 0; i < rotations.size(); i++){
			
			glm::vec3 rotRoot = rotations[i].first.first;
			glm::vec3 rotAxis = rotations[i].first.second - rotRoot;
			float rotSpeed = rotations[i].second;

			glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians(rotSpeed * dt), rotAxis);
			for(int j = i + 1; j < rotations.size(); j++){
				rotations[j].first.first -= rotRoot;
				rotations[j].first.first = glm::vec3(rotateMat * glm::vec4(rotations[j].first.first, 0.0f));
				rotations[j].first.first += rotRoot;

				rotations[j].first.second -= rotRoot;
				rotations[j].first.second = glm::vec3(rotateMat * glm::vec4(rotations[j].first.second, 0.0f));
				rotations[j].first.second += rotRoot;
			}

			for(int j = 0; j < size_; j++){
				DrawableTriangle& tri = triangles[j];
				for(int k = 0; k < 3; k++){
					tri.vertex(k).position -= rotRoot;
					tri.vertex(k).position = glm::vec3(rotateMat * glm::vec4(tri.vertex(k).position, 0.0f));
					tri.vertex(k).position += rotRoot;
					tri.vertex(k).normal = glm::vec3(rotateMat * glm::vec4(tri.vertex(k).normal, 0.0f));
				}
			}
		}
	};

	void rotate(glm::vec3 rotAxis, float angle) {
		glm::vec3 rotRoot = node;

		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians(angle), rotAxis);

		for(int j = 0; j < rotations.size(); j++){
			rotations[j].first.first -= rotRoot;
			rotations[j].first.first = glm::vec3(rotateMat * glm::vec4(rotations[j].first.first, 0.0f));
			rotations[j].first.first += rotRoot;

			rotations[j].first.second -= rotRoot;
			rotations[j].first.second = glm::vec3(rotateMat * glm::vec4(rotations[j].first.second, 0.0f));
			rotations[j].first.second += rotRoot;
		}


	
		for(int j = 0; j < size_; j++){
			DrawableTriangle& tri = triangles[j];
			for(int k = 0; k < 3; k++){
				tri.vertex(k).position -= rotRoot;
				tri.vertex(k).position = glm::vec3(rotateMat * glm::vec4(tri.vertex(k).position, 0.0f));
				tri.vertex(k).position += rotRoot;
				tri.vertex(k).normal = glm::vec3(rotateMat * glm::vec4(tri.vertex(k).normal, 0.0f));
			}
		}
	};

//	virtual void initialize(vertIterator initVertIt) = 0;
	virtual ~Model(){

	};
};

class StaticModel: public virtual Model {
public:
	explicit StaticModel(int size) {
		size_ = size;
		if(!MazeGame::triManager.applyForStaticTringles(size_, triangles)){
			std::cout << "Failed to get static triangles(" << size_ << ") ";
		}
		for(auto& tri: triangles)
			tri.setColor({1.0f, 1.0f, 1.0f});
	}

	virtual StaticModel& operator=(StaticModel&& another) {
		if(another == *this)
			return *this;
		MazeGame::triManager.returnStaticTriangles(triangles);

		triangles = another.triangles;
		return *this;		
	};

	~StaticModel() override{
		MazeGame::triManager.returnStaticTriangles(triangles);
	}
};

class DynamicModel: public virtual Model {
public:
	explicit DynamicModel(int size) {
		size_ = size;
		if(!MazeGame::triManager.applyForDynamicTringles(size_, triangles)){
			std::cout << "Failed to get dynamic triangles(" << size_ << ") ";
		}
		for(auto& tri: triangles)
			tri.setColor({1.0f, 1.0f, 1.0f});
	}


	virtual DynamicModel& operator=(DynamicModel&& another) {
		if(another == *this)
			return *this;

		MazeGame::triManager.returnDynamicTriangles(triangles);

		return *this;
	};

	~DynamicModel() override{
		MazeGame::triManager.returnDynamicTriangles(triangles);

	}
};

class LodableModel: public virtual Model {

	std::string filename_;
	float scale_;

protected:
	void setup() override{
		std::fstream input;
		input.open(filename_);
		if(!input.good()){
			std::cout << "Failed to load model " << filename_ << std::endl << "Could not open the file!" << std::endl;
			return;
		}
		int rubbish;
		input >> rubbish;
		for(int i = 0; i < size_; i++){
			for(int j = 0; j < 3; j++){
				float x, y, z;

				input >> x >> y >> z;

				if(!input.good()){
					std::cout  << "Failed to load model " << filename_ << std::endl << "File is broken" << std::endl;
					return;
				}
				triangles[i].vertex(j).position = {x, y, z};
			}

			triangles[i].setupNormal();
			int invert;

			input >> invert;

			if(!input.good()){
				std::cout  << "Failed to load model " << filename_ << std::endl << "File is broken" << std::endl;
				return;
			}

			if(invert == 1)
				triangles[i].reverseNormal();
			float red, green, blue;

			input >> red >> green >> blue;

			if(!input.good()){
				std::cout  << "Failed to load model " << filename_ << std::endl << "File is broken" << std::endl;
				return;
			}

			triangles[i].setColor({red, green, blue});
		}
		scale(scale_);
	};

public:
	explicit LodableModel(std::string filename,float scale = 1.0f): Model(), filename_(filename), scale_(scale){
		std::fstream input;
		input.open(filename_);
		if(!input.good()){
			std::cout << "Failed to load model " << filename_ << std::endl;
			return;
		}
		input >> size_;
		input.close();
	}
};

class DynamicLodableModel: public LodableModel, public DynamicModel {
public:
	explicit DynamicLodableModel(std::string filename,float scale = 1.0f): 
	Model(), LodableModel(filename, scale), DynamicModel(size_)
	{
		setup();
	};
};

class SizedObject {
protected:
	float geomSize;
public:
	explicit SizedObject(float size): geomSize(size){};
	float getGeomSize(){
		return geomSize;
	};

	virtual ~SizedObject(){};
};

class SimpleOctagon: public DynamicModel, public SizedObject {
	void setup() override{
		glm::vec3 vertexBase1 = {-geomSize/2.0, 0.0f, -geomSize/2.0f};
		glm::vec3 vertexBase2 = {geomSize/2.0, 0.0f, -geomSize/2.0f};
		glm::vec3 vertexBase3 = {geomSize/2.0, 0.0f, geomSize/2.0f};
		glm::vec3 vertexBase4 = {-geomSize/2.0, 0.0f, geomSize/2.0f};
		glm::vec3 vertexUpperCone = {0.0f, -geomSize/2.0f * sqrt(2.0f), 0.0f};
		glm::vec3 vertexLowerCone = {0.0f, geomSize/2.0f * sqrt(2.0f), 0.0f};

		triangles[0].vertex(0).position = vertexBase1;
//		triangles[0].vertex(0).uv = {0.0f, 0.0f};
		triangles[0].vertex(2).position = vertexBase2;
//		triangles[0].vertex(2).uv = {1.0f, 0.0f};
		triangles[0].vertex(1).position = vertexUpperCone;
//		triangles[0].vertex(1).uv = {1.0f, 1.0f};
		triangles[0].setupNormal();
		triangles[0].reverseNormal();

		triangles[1].vertex(0).position = vertexBase2;
//		triangles[1].vertex(0).uv = {0.0f, 0.0f};
		triangles[1].vertex(2).position = vertexBase3;
//		triangles[1].vertex(2).uv = {1.0f, 0.0f};
		triangles[1].vertex(1).position = vertexUpperCone;
//		triangles[1].vertex(1).uv = {1.0f, 1.0f};
		triangles[1].setupNormal();
		triangles[1].reverseNormal();

		triangles[2].vertex(0).position = vertexBase3;
//		triangles[2].vertex(0).uv = {0.0f, 0.0f};
		triangles[2].vertex(2).position = vertexBase4;
//		triangles[2].vertex(2).uv = {1.0f, 0.0f};
		triangles[2].vertex(1).position = vertexUpperCone;
//		triangles[2].vertex(1).uv = {1.0f, 1.0f};
		triangles[2].setupNormal();
		triangles[2].reverseNormal();

		triangles[3].vertex(0).position = vertexBase4;
//		triangles[3].vertex(0).uv = {0.0f, 0.0f};
		triangles[3].vertex(2).position = vertexBase1;
//		triangles[3].vertex(2).uv = {1.0f, 0.0f};
		triangles[3].vertex(1).position = vertexUpperCone;
//		triangles[3].vertex(1).uv = {1.0f, 1.0f};
		triangles[3].setupNormal();
		triangles[3].reverseNormal();

		triangles[4].vertex(0).position = vertexBase1;
		triangles[4].vertex(1).position = vertexBase2;
		triangles[4].vertex(2).position = vertexLowerCone;
		triangles[4].setupNormal();
		triangles[4].reverseNormal();

		triangles[5].vertex(0).position = vertexBase2;
		triangles[5].vertex(1).position = vertexBase3;
		triangles[5].vertex(2).position = vertexLowerCone;
		triangles[5].setupNormal();
		triangles[5].reverseNormal();

		triangles[6].vertex(0).position = vertexBase3;
		triangles[6].vertex(1).position = vertexBase4;
		triangles[6].vertex(2).position = vertexLowerCone;
		triangles[6].setupNormal();
		triangles[6].reverseNormal();

		triangles[7].vertex(0).position = vertexBase4;
		triangles[7].vertex(1).position = vertexBase1;
		triangles[7].vertex(2).position = vertexLowerCone;
		triangles[7].setupNormal();
		triangles[7].reverseNormal();

		addNewRotationFront({{vertexLowerCone, vertexUpperCone}, 90.0f});
	}
public:
	explicit SimpleOctagon(float size, glm::vec3 color = {1.0f, 1.0f, 1.0f}): Model(), DynamicModel(8), SizedObject(size){
		setup();
		setColor(color);
	}
};


class CoinModel: public DynamicModel, public SizedObject {
	void setup() override {
		std::vector<glm::vec3> radialPoints;
		for(int i = 0; i < 10; i++)
			radialPoints.push_back({geomSize * cos(glm::radians(static_cast<float>(i * 36))) * 0.5f,geomSize * sin(glm::radians(static_cast<float>(i * 36))) * 0.5f, 0.0f });
//FACES
		glm::vec3 zShift = {0.0f, 0.0f, geomSize / 5.0f};
		int index = 0;
		for(int i = 0; i < 8; i++){
			triangles[index].vertex(0).position = radialPoints[0] + zShift;
			triangles[index].vertex(1).position = radialPoints[i + 1] + zShift;
			triangles[index].vertex(2).position = radialPoints[(i + 2) % 10] + zShift;
			triangles[index].setupNormal();

			index++;
			
			triangles[index].vertex(0).position = radialPoints[0] - zShift;
			triangles[index].vertex(1).position = radialPoints[i + 1] - zShift;
			triangles[index].vertex(2).position = radialPoints[(i + 2) % 10] - zShift;
			triangles[index].setupNormal();
			triangles[index].reverseNormal();

			index++;
		}

//EDGE
		for(int i = 0; i < 10; i++){
			triangles[index].vertex(0).position = radialPoints[i] + zShift;
			triangles[index].vertex(1).position = radialPoints[(i + 1) % 10] + zShift;
			triangles[index].vertex(2).position = radialPoints[i] - zShift;
			triangles[index].setupNormal();
			triangles[index].reverseNormal();

			index++;
			
			triangles[index].vertex(0).position = radialPoints[i] - zShift;
			triangles[index].vertex(1).position = radialPoints[(i + 1) % 10] - zShift;
			triangles[index].vertex(2).position = radialPoints[(i + 1) % 10] + zShift;
			triangles[index].setupNormal();

			index++;			
		}
		addNewRotationFront({{{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, 120.0f});
	};

public:
	CoinModel(float size = 5.0f, glm::vec3 color = {0.8f, 0.85f, 0.0f}):Model(), DynamicModel(36), SizedObject(size){
		setup();
		setColor(color);
	};
};



class SimpleArrow: public DynamicModel, public SizedObject {

	void setup() override {
		glm::vec3 vertexNose = {0.0f, 0.0f, geomSize * 0.75f};
		glm::vec3 vertexLeftSide = {-geomSize/2.0, 0.0f, -geomSize * 0.25f};
		glm::vec3 vertexInnerNose = {0.0f, 0.0f, 0.0f};
		glm::vec3 vertexRightSide = { geomSize/2.0, 0.0f, -geomSize * 0.25f};

		glm::vec3 verticalUpShift = {0.0f, geomSize * 0.25f, 0.0f};
		glm::vec3 verticalDownShift = {0.0f, -geomSize * 0.25f, 0.0f};

//UPPER FACE

		triangles[0].vertex(0).position = vertexNose + verticalUpShift;
		triangles[0].vertex(1).position = vertexLeftSide + verticalUpShift;
		triangles[0].vertex(2).position = vertexInnerNose + verticalUpShift;
		triangles[0].setupNormal();
		triangles[0].reverseNormal();

		triangles[1].vertex(0).position = vertexNose + verticalUpShift;
		triangles[1].vertex(2).position = vertexRightSide + verticalUpShift;
		triangles[1].vertex(1).position = vertexInnerNose + verticalUpShift;
		triangles[1].setupNormal();
		triangles[1].reverseNormal();

//=======================

//LOWER FACE

		triangles[2].vertex(0).position = vertexNose + verticalDownShift;
		triangles[2].vertex(2).position = vertexLeftSide + verticalDownShift;
		triangles[2].vertex(1).position = vertexInnerNose + verticalDownShift;
		triangles[2].setupNormal();
		triangles[2].reverseNormal();

		triangles[3].vertex(0).position = vertexNose + verticalDownShift;
		triangles[3].vertex(1).position = vertexRightSide + verticalDownShift;
		triangles[3].vertex(2).position = vertexInnerNose + verticalDownShift;
		triangles[3].setupNormal();
		triangles[3].reverseNormal();

//=======================

//LEFT OUTER SIDE

		triangles[4].vertex(0).position = vertexNose + verticalDownShift;
		triangles[4].vertex(1).position = vertexLeftSide + verticalDownShift;
		triangles[4].vertex(2).position = vertexNose + verticalUpShift;
		triangles[4].setupNormal();
		triangles[4].reverseNormal();

		triangles[5].vertex(0).position = vertexLeftSide + verticalUpShift;
		triangles[5].vertex(1).position = vertexNose + verticalUpShift;
		triangles[5].vertex(2).position = vertexLeftSide + verticalDownShift;
		triangles[5].setupNormal();
		triangles[5].reverseNormal();
//========================

//RIGHT OUTER SIDE
		triangles[6].vertex(0).position = vertexNose + verticalDownShift;
		triangles[6].vertex(1).position = vertexRightSide + verticalDownShift;
		triangles[6].vertex(2).position = vertexNose + verticalUpShift;
		triangles[6].setupNormal();

		triangles[7].vertex(0).position = vertexRightSide + verticalUpShift;
		triangles[7].vertex(1).position = vertexNose + verticalUpShift;
		triangles[7].vertex(2).position = vertexRightSide + verticalDownShift;
		triangles[7].setupNormal();
//=========================

//LEFT INNER SIDE

		triangles[8].vertex(0).position = vertexInnerNose + verticalDownShift;
		triangles[8].vertex(1).position = vertexLeftSide + verticalUpShift;
		triangles[8].vertex(2).position = vertexLeftSide + verticalDownShift;
		triangles[8].setupNormal();
		triangles[8].reverseNormal();
		triangles[9].vertex(0).position = vertexLeftSide + verticalUpShift;
		triangles[9].vertex(1).position = vertexInnerNose + verticalDownShift;
		triangles[9].vertex(2).position = vertexInnerNose + verticalUpShift;
		triangles[9].setupNormal();
		triangles[9].reverseNormal();
//========================

//RIGHT INNER SIDE
		triangles[10].vertex(0).position = vertexInnerNose + verticalDownShift;
		triangles[10].vertex(2).position = vertexRightSide + verticalUpShift;
		triangles[10].vertex(1).position = vertexRightSide + verticalDownShift;
		triangles[10].setupNormal();
		triangles[10].reverseNormal();

		triangles[11].vertex(0).position = vertexRightSide + verticalUpShift;
		triangles[11].vertex(2).position = vertexInnerNose+ verticalDownShift;
		triangles[11].vertex(1).position = vertexInnerNose + verticalUpShift;
		triangles[11].setupNormal();
		triangles[11].reverseNormal();
//=========================
		addNewRotationFront({{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, 90.0f});

	}
public:
	explicit SimpleArrow(float size = 5.0f, glm::vec3 color = {1.0f, 0.0f, 0.0f}): Model(), DynamicModel(12), SizedObject(size){
		setup();
		setColor(color);
	}
};


const glm::vec3 VERTICAL_NORM = {0.0f, -1.0f, 0.0f};
const glm::vec3 EAST_NORM = {-1.0f, 0.0f, 0.0f};
const glm::vec3 WEST_NORM = {1.0f, 0.0f, 0.0f};
const glm::vec3 NORTH_NORM = {0.0f, 0.0f, -1.0f};
const glm::vec3 SOUTH_NORM = {0.0f, 0.0f, 1.0f};
const float zeroLevel = 10.0f;

int polysRequested(){
	int count = 0;
	for(int x = 0; x < MazeGame::gameField.getWidth(); x++)
		for(int y = 0; y < MazeGame::gameField.getHeight(); y++){
			count++;
			std::vector<bool> sides= MazeGame::gameField.openSideFaces(x,y);
			for(auto side: sides)
				if(side)
					count++;
		}
	return count * 2;
}

class FieldModel final: public StaticModel{
	std::vector<std::pair<float, float>> nodeHeights;
	std::vector<std::pair<float, float>> nodeShifts;
	std::vector<int> triIndices;



	float cellSize = 10.0, wallHeight = 8.0;

	void riseWall(int x, int y, float height){

		std::vector<bool> sides = MazeGame::gameField.openSideFaces(x,y);
		int index = triIndices[y * MazeGame::gameField.getWidth() + x];

		float corn1_height = nodeHeights[y * (MazeGame::gameField.getWidth() + 1) + x].first;
		float corn2_height = nodeHeights[y * (MazeGame::gameField.getWidth() + 1) + x + 1].first;
		float corn3_height = nodeHeights[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x + 1].first;
		float corn4_height = nodeHeights[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x].first;


		float corn1_Xshift = nodeShifts[y * (MazeGame::gameField.getWidth() + 1) + x].first;
		float corn2_Xshift = nodeShifts[y * (MazeGame::gameField.getWidth() + 1) + x + 1].first;
		float corn3_Xshift = nodeShifts[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x + 1].first;
		float corn4_Xshift = nodeShifts[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x].first;

		float corn1_Yshift = nodeShifts[y * (MazeGame::gameField.getWidth() + 1) + x].second;
		float corn2_Yshift = nodeShifts[y * (MazeGame::gameField.getWidth() + 1) + x + 1].second;
		float corn3_Yshift = nodeShifts[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x + 1].second;
		float corn4_Yshift = nodeShifts[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x].second;

			
		auto& upper1 = triangles[index];
	
		index++;
	
		auto& upper2 = triangles[index];
	
		index++;
	
		if(sides[3]){
			auto& east1 = triangles[index];
			index++;
			auto& east2 = triangles[index];
			index++;

			east1.vertex(0).position += VERTICAL_NORM * corn1_height + WEST_NORM * corn1_Xshift + SOUTH_NORM * corn1_Yshift;
			east2.vertex(1).position += VERTICAL_NORM * corn4_height + WEST_NORM * corn4_Xshift + SOUTH_NORM * corn4_Yshift;
			east2.vertex(2).position += VERTICAL_NORM * corn1_height + WEST_NORM * corn1_Xshift + SOUTH_NORM * corn1_Yshift;

			east1.vertex(2).uv = {1.0f, 1.0f, 0.0f};
			east1.vertex(1).uv = {1.0f, 0.0f, 0.0f};
			east1.vertex(0).uv = {0.0f, 0.0f, 0.0f};

			east2.vertex(2).uv = {0.0f, 0.0f, 0.0f};
			east2.vertex(0).uv = {1.0f, 1.0f, 0.0f};
        	east2.vertex(1).uv = {0.0f, 1.0f, 0.0f};

        	east1.setupNormal();
        	east1.reverseNormal();
        	east2.setupNormal();
        	east2.reverseNormal();


		}
		if(sides[1]){
			auto& west1 = triangles[index];
			index++;
			auto& west2 = triangles[index];
			index++;

			west1.vertex(0).position += VERTICAL_NORM * corn2_height + WEST_NORM * corn2_Xshift + SOUTH_NORM * corn2_Yshift;

			west2.vertex(1).position += VERTICAL_NORM * corn3_height + WEST_NORM * corn3_Xshift + SOUTH_NORM * corn3_Yshift;
			west2.vertex(2).position += VERTICAL_NORM * corn2_height + WEST_NORM * corn2_Xshift + SOUTH_NORM * corn2_Yshift;

			west1.vertex(2).uv = {1.0f, 1.0f, 0.0f};
			west1.vertex(1).uv = {1.0f, 0.0f, 0.0f};
			west1.vertex(0).uv = {0.0f, 0.0f, 0.0f};

			west2.vertex(2).uv = {0.0f, 0.0f, 0.0f};
			west2.vertex(0).uv = {1.0f, 1.0f, 0.0f};
        	west2.vertex(1).uv = {0.0f, 1.0f, 0.0f};

        	west1.setupNormal();
        	west2.setupNormal();
		}
		if(sides[0]){
			auto& north1 = triangles[index];
			index++;
			auto& north2 = triangles[index];
			index++;

			north1.vertex(0).position += VERTICAL_NORM * corn1_height + WEST_NORM * corn1_Xshift + SOUTH_NORM * corn1_Yshift;

			north2.vertex(1).position += VERTICAL_NORM * corn2_height + WEST_NORM * corn2_Xshift + SOUTH_NORM * corn2_Yshift;
			north2.vertex(2).position += VERTICAL_NORM * corn1_height + WEST_NORM * corn1_Xshift + SOUTH_NORM * corn1_Yshift;

			north1.vertex(2).uv = {1.0f, 1.0f, 0.0f};
			north1.vertex(1).uv = {1.0f, 0.0f, 0.0f};
			north1.vertex(0).uv = {0.0f, 0.0f, 0.0f};

			north2.vertex(2).uv = {0.0f, 0.0f, 0.0f};
			north2.vertex(0).uv = {1.0f, 1.0f, 0.0f};
        	north2.vertex(1).uv = {0.0f, 1.0f, 0.0f};

        	north1.setupNormal();
        	north2.setupNormal();
		}
		if(sides[2]){
			auto& south1 = triangles[index];
			index++;
			auto& south2 = triangles[index];
			index++;
			south1.vertex(0).position += VERTICAL_NORM * corn3_height + WEST_NORM * corn3_Xshift + SOUTH_NORM * corn3_Yshift;

			south2.vertex(1).position += VERTICAL_NORM * corn4_height + WEST_NORM * corn4_Xshift + SOUTH_NORM * corn4_Yshift;
			south2.vertex(2).position += VERTICAL_NORM * corn3_height + WEST_NORM * corn3_Xshift + SOUTH_NORM * corn3_Yshift;

			south1.vertex(2).uv = {1.0f, 1.0f, 0.0f};
			south1.vertex(1).uv = {1.0f, 0.0f, 0.0f};
			south1.vertex(0).uv = {0.0f, 0.0f, 0.0f};

			south2.vertex(2).uv = {0.0f, 0.0f, 0.0f};
			south2.vertex(0).uv = {1.0f, 1.0f, 0.0f};
        	south2.vertex(1).uv = {0.0f, 1.0f, 0.0f};

        	south1.setupNormal();
        	south2.setupNormal();

		}

		upper1.vertex(0).position += VERTICAL_NORM * corn1_height + WEST_NORM * corn1_Xshift + SOUTH_NORM * corn1_Yshift;

		upper1.vertex(1).position += VERTICAL_NORM * corn2_height + WEST_NORM * corn2_Xshift + SOUTH_NORM * corn2_Yshift;

		upper1.vertex(2).position += VERTICAL_NORM * corn3_height + WEST_NORM * corn3_Xshift + SOUTH_NORM * corn3_Yshift;


		upper1.vertex(2).uv = {1.0f, 1.0f, 0.0f};
		upper1.vertex(1).uv = {1.0f, 0.0f, 0.0f};
		upper1.vertex(0).uv = {0.0f, 0.0f, 0.0f};

		upper2.vertex(2).uv = {0.0f, 0.0f, 0.0f};
		upper2.vertex(0).uv = {1.0f, 1.0f, 0.0f};
        upper2.vertex(1).uv = {0.0f, 1.0f, 0.0f};

		upper2.vertex(0).position += VERTICAL_NORM * corn3_height + WEST_NORM * corn3_Xshift + SOUTH_NORM * corn3_Yshift;
		upper2.vertex(1).position += VERTICAL_NORM * corn4_height + WEST_NORM * corn4_Xshift + SOUTH_NORM * corn4_Yshift;
		upper2.vertex(2).position += VERTICAL_NORM * corn1_height + WEST_NORM * corn1_Xshift + SOUTH_NORM * corn1_Yshift;


		upper1.setColor({1.0f, 1.0f, 1.0f});
		upper2.setColor({1.0f, 1.0f, 1.0f});

		upper1.setupNormal();
		upper2.setupNormal();

		//east1.setupNormal();
		//east2.setupNormal();

		//west1.setupNormal();
		//west2.setupNormal();





	}

	void setup(){
		int index = 0;

		MazeGame::triManager.returnStaticTriangles(triangles);
		triangles.clear();
		MazeGame::triManager.applyForStaticTringles(polysRequested(), triangles);
		triIndices.resize(0);

		for(int y = 0; y < MazeGame::gameField.getHeight(); y++)
			for(int x = 0; x < MazeGame::gameField.getWidth(); x++){
				triIndices.push_back(index);
				glm::vec3 corner[4] = {{x * cellSize, zeroLevel, y* cellSize}, {(x + 1) * cellSize, zeroLevel, y* cellSize}, {(x + 1) * cellSize, zeroLevel, (y + 1)* cellSize}, {x * cellSize, zeroLevel, (y + 1) * cellSize}};

				std::vector<bool> sides = MazeGame::gameField.openSideFaces(x,y);

				float corn1_elevat = nodeHeights[y * (MazeGame::gameField.getWidth() + 1) + x].second;
				float corn2_elevat = nodeHeights[y * (MazeGame::gameField.getWidth() + 1) + x + 1].second;
				float corn3_elevat = nodeHeights[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x + 1].second;
				float corn4_elevat = nodeHeights[(y + 1) * (MazeGame::gameField.getWidth() + 1) + x].second;

				auto& upper1 = triangles[index];

				upper1.vertex(0).position = corner[0] + (MazeGame::gameField.getType(x, y) == MazeGame::CellType::PATH ? 1.0f : 0.0f ) * VERTICAL_NORM * corn1_elevat;
				upper1.vertex(1).position = corner[1] + (MazeGame::gameField.getType(x, y) == MazeGame::CellType::PATH ? 1.0f : 0.0f ) * VERTICAL_NORM * corn2_elevat;
				upper1.vertex(2).position = corner[2] + (MazeGame::gameField.getType(x, y) == MazeGame::CellType::PATH ? 1.0f : 0.0f ) * VERTICAL_NORM * corn3_elevat;
				upper1.vertex(2).uv = {1.0f, 1.0f, 0.0f};
				upper1.vertex(1).uv = {1.0f, 0.0f, 0.0f};
				upper1.vertex(0).uv = {0.0f, 0.0f, 0.0f};


				upper1.vertex(0).normal = VERTICAL_NORM;
				upper1.vertex(1).normal = VERTICAL_NORM;
				upper1.vertex(2).normal = VERTICAL_NORM;

				index++;

				auto& upper2 = triangles[index];

				upper2.vertex(0).position = corner[2] + (MazeGame::gameField.getType(x, y) == MazeGame::CellType::PATH ? 1.0f : 0.0f ) * VERTICAL_NORM * corn3_elevat;
				upper2.vertex(1).position = corner[3] + (MazeGame::gameField.getType(x, y) == MazeGame::CellType::PATH ? 1.0f : 0.0f ) * VERTICAL_NORM * corn4_elevat;
				upper2.vertex(2).position = corner[0] + (MazeGame::gameField.getType(x, y) == MazeGame::CellType::PATH ? 1.0f : 0.0f ) * VERTICAL_NORM * corn1_elevat;

				upper2.vertex(0).normal = VERTICAL_NORM;
				upper2.vertex(1).normal = VERTICAL_NORM;
				upper2.vertex(2).normal = VERTICAL_NORM;

				upper2.vertex(2).uv = {0.0f, 0.0f, 0.0f};
				upper2.vertex(0).uv = {1.0f, 1.0f, 0.0f};
        		upper2.vertex(1).uv = {0.0f, 1.0f, 0.0f};

				index++;

				if(sides[3]){

					auto& east1 = triangles[index];

					east1.vertex(0).position = corner[0];
					east1.vertex(1).position = corner[0] + VERTICAL_NORM * corn1_elevat;
					east1.vertex(2).position = corner[3] + VERTICAL_NORM * corn4_elevat;

					east1.vertex(0).normal = EAST_NORM;
					east1.vertex(1).normal = EAST_NORM;
					east1.vertex(2).normal = EAST_NORM;

					index++;

					auto& east2 = triangles[index];

					east2.vertex(0).position = corner[3] + VERTICAL_NORM * corn4_elevat;
					east2.vertex(1).position = corner[3];
					east2.vertex(2).position = corner[0];

					east2.vertex(0).normal = EAST_NORM;
					east2.vertex(1).normal = EAST_NORM;
					east2.vertex(2).normal = EAST_NORM;

					index++;
				}

				if(sides[1]){
					

					auto& west1 = triangles[index];

					west1.vertex(0).position = corner[1];
					west1.vertex(1).position = corner[1] + VERTICAL_NORM * corn2_elevat;
					west1.vertex(2).position = corner[2] + VERTICAL_NORM * corn3_elevat;

					west1.vertex(0).normal = WEST_NORM;
					west1.vertex(1).normal = WEST_NORM;
					west1.vertex(2).normal = WEST_NORM;

					index++;

					auto& west2 = triangles[index];

					west2.vertex(0).position = corner[2] + VERTICAL_NORM * corn3_elevat;
					west2.vertex(1).position = corner[2];
					west2.vertex(2).position = corner[1];

					west2.vertex(0).normal = WEST_NORM;
					west2.vertex(1).normal = WEST_NORM;
					west2.vertex(2).normal = WEST_NORM;

					index++;

				}

				if(sides[0]){

					auto& north1 = triangles[index];

					north1.vertex(0).position = corner[0];
					north1.vertex(1).position = corner[0] + VERTICAL_NORM * corn1_elevat;
					north1.vertex(2).position = corner[1] + VERTICAL_NORM * corn2_elevat;

					north1.vertex(0).normal = NORTH_NORM;
					north1.vertex(1).normal = NORTH_NORM;
					north1.vertex(2).normal = NORTH_NORM;

					index++;

					auto& north2 = triangles[index];

					north2.vertex(0).position = corner[1] + VERTICAL_NORM * corn2_elevat;
					north2.vertex(1).position = corner[1];
					north2.vertex(2).position = corner[0];

					north2.vertex(0).normal = NORTH_NORM;
					north2.vertex(1).normal = NORTH_NORM;
					north2.vertex(2).normal = NORTH_NORM;

					index++;

				}
				if(sides[2]){
					auto& south1 = triangles[index];

					south1.vertex(0).position = corner[2];
					south1.vertex(1).position = corner[2] + VERTICAL_NORM * corn3_elevat;
					south1.vertex(2).position = corner[3] + VERTICAL_NORM * corn4_elevat;

					south1.vertex(0).normal = SOUTH_NORM;
					south1.vertex(1).normal = SOUTH_NORM;
					south1.vertex(2).normal = SOUTH_NORM;

					index++;

					auto& south2 = triangles[index];

					south2.vertex(0).position = corner[3] + VERTICAL_NORM * corn4_elevat;
					south2.vertex(1).position = corner[3];
					south2.vertex(2).position = corner[2];

					south2.vertex(0).normal = SOUTH_NORM;
					south2.vertex(1).normal = SOUTH_NORM;
					south2.vertex(2).normal = SOUTH_NORM;

					index++;
				}

				upper1.setupNormal();
				upper2.setupNormal();


				//setColor(x, y, {0.8f, 0.4f, 0.2f});
				//setColor(x, y, {0.9f, 0.7f, 0.7f});

				setColor(x, y, {0.6f, 0.6f, 0.6f});
				if(MazeGame::gameField.getType(x, y) == MazeGame::CellType::WALL){
					// FIRE YELLOW setColor(x, y, {1.0f, 0.5f, 0.0f});
					setColor(x, y, {1.0f, 1.0f, 1.0f});
					riseWall(x, y, wallHeight);
				}


			}

	};


public:
	void setColor(int x, int y, glm::vec3 newColor){
		std::vector<bool> sides = MazeGame::gameField.openSideFaces(x,y);
		int count = 2;
		for(auto side: sides)
			if(side)
				count += 2;

		for(int i = 0; i < count; i++){
			auto& tri = triangles[triIndices[y * MazeGame::gameField.getWidth() + x] + i];
			tri.setColor(newColor);
		}		
	}

	void recreate(){
		setup();
	}

	FieldModel& operator=(FieldModel&& another){
		if(&another != this){
			MazeGame::triManager.returnStaticTriangles(triangles);	
			triangles = another.triangles;
			another.triangles.clear();

			triIndices = another.triIndices;
			nodeShifts = another.nodeShifts;
			nodeHeights = another.nodeHeights;
		}
		return *this;
	}

	float getCellSize() const{
		return cellSize;
	}

	float getWallHeight() const{
		return wallHeight;
	}

	float getZeroLevel() const {
		return zeroLevel;
	}

	FieldModel() : Model(), StaticModel(polysRequested()){
		std::cout << "Making Field model with " << size_ << " triangles" << std::endl;

		nodeHeights.resize((MazeGame::gameField.getHeight() + 1) * (MazeGame::gameField.getWidth() + 1));
		nodeShifts.resize((MazeGame::gameField.getHeight() + 1) * (MazeGame::gameField.getWidth() + 1));

		for(auto& h : nodeHeights){
			h.first = (static_cast<float>((rand() % 100)) *0.001 - 0.05)* wallHeight + wallHeight;
			h.second = (static_cast<float>((rand() % 100)) *0.001)* wallHeight;
		}

		for(auto& shift: nodeShifts){
			shift.first = (static_cast<float>((rand() % 100)) *0.001 - 0.05)* cellSize;
			shift.second = (static_cast<float>((rand() % 100)) *0.001 - 0.05)* cellSize;

		}

		setup();
	};
};



};
