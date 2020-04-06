#include "DrawableTriangle.h"




const glm::vec3 VERTICAL_NORM = {0.0f, -1.0f, 0.0f};
const glm::vec3 EAST_NORM = {-1.0f, 0.0f, 0.0f};
const glm::vec3 WEST_NORM = {1.0f, 0.0f, 0.0f};
const glm::vec3 NORTH_NORM = {0.0f, 0.0f, -1.0f};
const glm::vec3 SOUTH_NORM = {0.0f, 0.0f, 1.0f};


class FieldModel : public Model{
	// Model inheritances:
	// int size_
	// bool initialized
	// vertIterator vertices;
	// std::vector<int> indices;	
	CellField& field;
	float cellSize = 10.0, wallHeight = 8.0, zeroLevel = 10.0f;

public:
	FieldModel(CellField& fd), field(fd){
		int size_per_level = (field.getWidth() + 1) * (field.getHeight() + 1);
		size_ =  size_per_level * 4;
		if(int y = 0; y < field.getHeight(); y++)
			if(int x = 0; x < field.getWidth(); x++){
				if(field.getType(x, y) == CT_PATH){
					int index1 = (field.getWidth() + 1) * y + x;
					int index2 = index1 + 1;
					int index3 = index1 + field.getWidth() + 1;
					int index4 = index3 + 1;

					indices.pushBack(index1);
					indices.pushBack(index2);
					indices.pushBack(index3);

					indices.pushBack(index2);
					indices.pushBack(index3);
					indices.pushBack(index4);
				}
				else{
					int index1_1 = (field.getWidth() + 1) * y + x + size_per_level;
					int index2_1 = index1_1 + 1 + size_per_level;
					int index3_1 = index1_1 + field.getWidth() + 1 + size_per_level;
					int index4_1 = index3_1 + 1 + size_per_level;

					int index1_2 = index1_1 + size_per_level;
					int index2_2 = index2_1 + size_per_level;
					int index3_2 = index3_1 + size_per_level;
					int index4_2 = index4_1 + size_per_level;

					int index1_3 = index1_2 + size_per_level;
					int index2_3 = index2_2 + size_per_level;
					int index3_3 = index3_2 + size_per_level;
					int index4_3 = index4_2 + size_per_level;

				// UPPER FACE

					indices.pushBack(index1_3);
					indices.pushBack(index2_3);
					indices.pushBack(index3_3);

					indices.pushBack(index2_3);
					indices.pushBack(index3_3);
					indices.pushBack(index4_3);

				//

					std::vector<bool> sides = field.openSideFaces(x,y);

				//	EAST FACE

				if(sides[3]){
				
					indices.pushBack(index1_2);
					indices.pushBack(index3_2);
					indices.pushBack(index3_1);

					indices.pushBack(index1_2);
					indices.pushBack(index1_1);
					indices.pushBack(index3_1);
				
				}

				//


				//	WEST FACE

				if(sides[1]){
				
					indices.pushBack(index2_2);
					indices.pushBack(index4_2);
					indices.pushBack(index4_1);

					indices.pushBack(index2_2);
					indices.pushBack(index2_1);
					indices.pushBack(index4_1);
				
				}
				
				//

				//	NORTH FACE

				if(sides[0]) {
				
					indices.pushBack(index1_2);
					indices.pushBack(index2_2);
					indices.pushBack(index2_1);

					indices.pushBack(index1_2);
					indices.pushBack(index1_1);
					indices.pushBack(index2_1);
				
				}
				
				//


				//	SOUTH FACE

				if(sides[2]){
				
					indices.pushBack(index4_2);
					indices.pushBack(index3_2);
					indices.pushBack(index3_1);

					indices.pushBack(index4_2);
					indices.pushBack(index4_1);
					indices.pushBack(index3_1);
				
				}
				
				//


				}
			}

	};

	void initialize(vertIterator initVertIt) override{
		vertices = initVertIt;
		int size_per_level = (field.getWidth() + 1) * (field.getHeight() + 1);

		if(int y = 0; y < field.getHeight(); y++)
			if(int x = 0; x < field.getWidth(); x++){
				int index1 = y * (field.getWidth() + 1) + x;
				int index2 = index1 + 1;
				int index3 = index1 + field.getWidth() + 1;
				int index4 = index3 + 1;
				glm::vec3 corner[4] = {{x * cellSize, zeroLevel, y* cellSize}, {(x + 1) * cellSize, zeroLevel, y* cellSize}, {x * cellSize, zeroLevel, (y + 1) * cellSize}, {(x + 1) * cellSize, zeroLevel, (y + 1)* cellSize}};

				for(int i = 0; i < 4; i++) {
					vertices[index1 + i * size_per_level].position = corner[0] + VERTICAL_NORM * wallHeight * (i < 2 ? 0 : 1);
					vertices[index1 + i * size_per_level].normal = (i == );
					vertices[index2 + i * size_per_level].position = corner[1] + VERTICAL_NORM * wallHeight * (i < 2 ? 0 : 1);
					vertices[index3 + i * size_per_level].position = corner[2] + VERTICAL_NORM * wallHeight * (i < 2 ? 0 : 1);
					vertices[index4 + i * size_per_level].position = corner[3] + VERTICAL_NORM * wallHeight * (i < 2 ? 0 : 1);
				}
			}
	};

}