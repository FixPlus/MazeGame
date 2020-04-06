#pragma once
#include "DrawableTriangle.h"
#include "drawer.h"



namespace triGraphic {

class DTManager final {
	std::list<DrawableTriangle> staticTriangles;
	std::list<DrawableTriangle> dynamicTriangles;


public:
	DTManager(Drawer* drawer = NULL, int static_tri_count = 0, int dynamic_tri_count = 0) {
		if(drawer){
			std::cout << static_tri_count << ":" << dynamic_tri_count << std::endl;
			std::vector<DrawableTriangle> statTrisTemp;
			std::vector<DrawableTriangle> dynamicTrisTemp;
			statTrisTemp.resize(static_tri_count);
			dynamicTrisTemp.resize(dynamic_tri_count);

			drawer->connect(statTrisTemp.begin(), statTrisTemp.end(), dynamicTrisTemp.begin(), dynamicTrisTemp.end());


			for(auto& tri: statTrisTemp)
				staticTriangles.push_back(tri);

			for(auto& tri: dynamicTrisTemp)
				dynamicTriangles.push_back(tri);
		}
	};

	bool applyForDynamicTringles(int count, std::vector<DrawableTriangle>& container) {
		if(container.size() > 0 || dynamicTriangles.size() < count)
			return false;

		for(int i = 0; i < count; i++){
			container.push_back(dynamicTriangles.front());
			dynamicTriangles.pop_front();
		}


		return true;

	};

	bool applyForStaticTringles(int count, std::vector<DrawableTriangle>& container) {
		if(container.size() > 0 || staticTriangles.size() < count)
			return false;

		for(int i = 0; i < count; i++){
			container.push_back(staticTriangles.front());
			staticTriangles.pop_front();
		}


		return true;

	};

	static void resetTriangles(std::vector<DrawableTriangle>& container){
		for(auto& tri: container){
			tri.vertex(0).position = {0.0f, 0.0f , 0.0f};
			tri.vertex(1).position = {0.0f, 0.0f , 0.0f};
			tri.vertex(2).position = {0.0f, 0.0f , 0.0f};
		}
	}

	void returnDynamicTriangles(std::vector<DrawableTriangle>& container){
		resetTriangles(container);
		for(auto& tri: container)
			dynamicTriangles.push_back(tri);

	};

	void returnStaticTriangles(std::vector<DrawableTriangle>& container){
		resetTriangles(container);
		for(auto& tri: container)
			staticTriangles.push_back(tri);


	};



};

};