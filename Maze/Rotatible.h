#include <deque>
#include <cmath>

namespace triGraphic {
using Rotation = std::pair<std::pair<glm::vec3, glm::vec3>, float>; // <<point1, point2>, rotSpeed>

class Rotatible{
protected:
	std::deque<Rotation> rotations;
public:

	void addNewRotationBack(Rotation const &rot){
		rotations.push_back(rot);
	};

	void addNewRotationFront(Rotation const &rot){
		rotations.push_front(rot);
	};

	virtual void rotate(float dt) = 0;

	virtual ~Rotatible() {};
};

};