
#include "VulkanExample.h"



void VulkanExample::mouseMoved(double x, double y, bool &handled){
//		std::cout << mousePos.x << " " << x << std::endl;
//		std::cout << mousePos.y << " " << y << std::endl;
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if(mouseButtons.right == true){
		dir.x *= (float)(x - mousePos.x) / 10.0;
		dir.y *= (float)(y - mousePos.y) / 10.0;
		dir.z = 0.0f;
	}
	else
		if(mouseButtons.middle == true){
			dir.x = 0.0f;
			dir.y = 0.0f;
			dir.z *= (float)(y - mousePos.y) / 10.0;
		}
		else{
			dir = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	dir = dir * rotateMat;

	cameraPos += glm::vec3(dir);
}

void VulkanExample::moveCameraForward(float distance){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(0.0f, 0.0f, distance, 1.0f);
	dir = dir * rotateMat;
	cameraPos += glm::vec3(dir);
}

void VulkanExample::moveCameraBackward(float distance){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(0.0f, 0.0f, -distance, 1.0f);
	dir = dir * rotateMat;
	cameraPos += glm::vec3(dir);
}

void VulkanExample::moveCameraLeft(float distance){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(-distance, 0.0f, 0.0f, 1.0f);
	dir = dir * rotateMat;
	cameraPos += glm::vec3(dir);
}

void VulkanExample::moveCameraRight(float distance){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(distance, 0.0f, 0.0f, 1.0f);
	dir = dir * rotateMat;
	cameraPos += glm::vec3(dir);
}

void VulkanExample::moveCameraUp(float distance){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(0.0f, distance, 0.0f, 1.0f);
	dir = dir * rotateMat;
	cameraPos += glm::vec3(dir);
}

void VulkanExample::moveCameraDown(float distance){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(0.0f, -distance, 0.0f, 1.0f);
	dir = dir * rotateMat;
	cameraPos += glm::vec3(dir);
}

void VulkanExample::OnUpdateUIOverlay(vks::UIOverlay *overlay)
{
}

void VulkanExample::viewChanged()
{
	// This function is called by the base example class each time the view is changed by user input
	updateUniformBuffers();
}

