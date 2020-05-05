#include <deque>
#include <cmath>

namespace triGraphic {
using Rotation = std::pair<glm::vec3, float>; // < axis, rotSpeed>

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