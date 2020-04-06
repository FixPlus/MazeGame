#pragma once
#include "VulkanExample.h"
using vertIterator = std::vector<Vertex>::iterator;

namespace triGraphic {


class DrawableTriangle {
	
	vertIterator vertices;
public:	

	explicit DrawableTriangle() {};

	bool operator==(DrawableTriangle const& another) const{
		return vertices == another.vertices;
	}
	void setIt(vertIterator iter){ vertices = iter; };
	
	void setColor(glm::vec3 const &newColor){
		vertices[0].color = newColor;
		vertices[1].color = newColor;
		vertices[2].color = newColor;

	};

	void move(glm::vec3 const &shift){
		vertices[0].position += shift;
		vertices[1].position += shift;
		vertices[2].position += shift;

	};

	void setTextureId(int id){
		for(int i = 0; i < 3; i++)
			vertices[i].uv[3] = static_cast<float>(id);
	}

	void setupNormal(){
		glm::vec3 vector1 = vertices[1].position - vertices[0].position;
		glm::vec3 vector2 = vertices[2].position - vertices[0].position;
		glm::vec3 normal = glm::cross(vector1, vector2);
		glm::normalize(normal);
		vertices[0].normal = normal;
		vertices[1].normal = normal;
		vertices[2].normal = normal;

	};

	void reverseNormal(){
		vertices[0].normal *= -1;
		vertices[1].normal *= -1;
		vertices[2].normal *= -1;		
	}

	Vertex& vertex(int id) const{ return vertices[id % 3];};

	virtual ~DrawableTriangle(){};
};





};



