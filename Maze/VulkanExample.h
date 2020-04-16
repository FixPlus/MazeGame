#pragma once
/*
* Vulkan Example - Texture loading (and display) example (including mip maps)
*
* Copyright (C) 2016-2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false

enum { NUMBER_OF_TEXTURES = 1};

const std::string texture_folder = "./data/";
const std::vector<std::string> texture_files = {"texture_1.ktx" };


// Vertex layout for this example
struct Vertex {
	glm::vec3 position;
	glm::vec3 uv;
	glm::vec3 color;	
	glm::vec3 normal;

	Vertex(): color{1.0f, 1.0f, 1.0f}{		
	}
};

class VulkanExample : public VulkanExampleBase{

public:
	// Contains all Vulkan objects that are required to store and use a texture
	// Note that this repository contains a texture class (VulkanTexture.hpp) that encapsulates texture loading functionality in a class that is used in subsequent demos
	struct Texture {
		VkSampler sampler;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
	};

	Texture texture[NUMBER_OF_TEXTURES];

	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount;

	vks::Buffer uniformBufferVS;

	struct {
		glm::mat4 projectionMatrix;
		glm::vec4 lightDirection;
		glm::vec4 viewPos;
		float lodBias = 0.0f;
	} uboVS;

	glm::vec4 lightDirection = {0.7f, 2.0f, 1.2f, 0.0f}; 

	struct {
		VkPipeline solid;
	} pipelines;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	std::vector<Vertex> localDynamicVertices;
	uint32_t localDynamicVerticesSize;
	std::vector<uint32_t> localDynamicIndices;
	uint32_t localDynamicIndicesSize;

	std::vector<Vertex> localStaticVertices;
	uint32_t localStaticVerticesSize;
	std::vector<uint32_t> localStaticIndices;
	uint32_t localStaticIndicesSize;

	VulkanExample(std::string windowName);

	~VulkanExample();

	virtual void getEnabledFeatures();

	void loadTexture();

	// Free all Vulkan resources used by a texture object
	void destroyTextureImage(Texture texture);

	void buildCommandBuffers();

	void draw();

	void prepareVertices(int n_stat_verts, int n_dyn_verts);


	void setupVertexDescriptions();

	void setupDescriptorPool();

	void setupDescriptorSetLayout();

	void setupDescriptorSet();

	void preparePipelines();

	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers();

	void updateUniformBuffers();

	void updateStaticVertices();

	void prepare(int n_stat_verts, int n_dyn_verts);


	virtual void render();


	void moveCameraForward(float distance);
	void moveCameraBackward(float distance);
	void moveCameraLeft(float distance);
	void moveCameraRight(float distance);
	void moveCameraUp(float distance);
	void moveCameraDown(float distance);
	void mouseMoved(double x, double y, bool &handled);	

	virtual void viewChanged();

    void OnUpdateUIOverlay(vks::UIOverlay *overlay);

};
