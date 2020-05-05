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
#include <list>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"
#include "InstanceData.h"

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define ENABLE_VALIDATION false


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 color;	

	Vertex(): color{1.0f, 1.0f, 1.0f}{		
	}
};
class VulkanExample : public VulkanExampleBase{

public:
	// Contains all Vulkan objects that are required to store and use a texture
	// Note that this repository contains a texture class (VulkanTexture.hpp) that encapsulates texture loading functionality in a class that is used in subsequent demos

	// Per-instance data block
	bool shouldRecreateInstances = false;

	struct Model{
		
		VkPipeline pipeline;
		VkDescriptorSet descriptorSet;
		vks::Texture2D texture;
		vks::Model model;

		std::vector<InstanceData> instances;
		std::list<InstanceView> instanceViews;
		vks::Buffer instanceBuf;

		virtual ~Model(){};
	};

	std::vector<Model> models;

	void constructModel(std::string model_filename, std::string texture_filename, Model& model){

		model.model.loadFromFile("./data/Models/" + model_filename + ".dae", vertexLayout, 0.1f, vulkanDevice, queue);

		// Textures
		VkFormat texFormat;
		// Get supported compressed texture format
		
		texFormat = VK_FORMAT_R8G8B8A8_UNORM;

		model.texture.loadFromFile("./data/Textures/" + texture_filename + ".ktx", texFormat, vulkanDevice, queue);

	}


	// Vertex layout for the models
	vks::VertexLayout vertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION,
		vks::VERTEX_COMPONENT_NORMAL,
		vks::VERTEX_COMPONENT_UV,
		vks::VERTEX_COMPONENT_COLOR,
	});


	// Contains the instanced data

	struct UBOVS {
		glm::mat4 projectionMatrix;
		glm::vec4 viewPos;
		glm::vec4 lightDirection = glm::vec4(0.7f, 2.0f, 1.2f, 0.0f);
	} uboVS;

	struct {
		vks::Buffer scene;
	} uniformBuffers;

	VkPipelineLayout pipelineLayout;

	VkDescriptorSetLayout descriptorSetLayout;



	VulkanExample(std::string windowName) : VulkanExampleBase(ENABLE_VALIDATION)
	{
		zoom = -2.5f;
		rotation = { 0.0f, 15.0f, 0.0f };
		title = windowName;
		settings.overlay = true;
    	UIOverlay.visible = true;	}

	~VulkanExample()
	{
		for(auto& model: models){
			vkDestroyPipeline(device, model.pipeline, nullptr);
		}
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		for(auto& model: models){
			//vkDestroyBuffer(device, model.instanceBuf.buffer, nullptr);
			//vkFreeMemory(device, model.instanceBuf.memory, nullptr);
			model.instanceBuf.destroy();
			model.model.destroy();
			model.texture.destroy();
		}
		uniformBuffers.scene.destroy();
	}

	// Enable physical device features required for this example				
	virtual void getEnabledFeatures()
	{
		// Enable anisotropic filtering if supported
		if (deviceFeatures.samplerAnisotropy) {
			enabledFeatures.samplerAnisotropy = VK_TRUE;
		}
		// Enable texture compression  
		if (deviceFeatures.textureCompressionBC) {
			enabledFeatures.textureCompressionBC = VK_TRUE;
		}
		else if (deviceFeatures.textureCompressionASTC_LDR) {
			enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
		}
		else if (deviceFeatures.textureCompressionETC2) {
			enabledFeatures.textureCompressionETC2 = VK_TRUE;
		}
	};	

	void buildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };


			for(auto& model: models){
				if(model.instances.empty())
					continue;

				vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &model.descriptorSet, 0, NULL);
				vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, model.pipeline);
				vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &model.model.vertices.buffer, offsets);
				// Binding point 1 : Instance data buffer
				vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &model.instanceBuf.buffer, offsets);
				vkCmdBindIndexBuffer(drawCmdBuffers[i], model.model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(drawCmdBuffers[i], model.model.indexCount, model.instances.size(), 0, 0, 0);
			
			}

			drawUI(drawCmdBuffers[i]);

			vkCmdEndRenderPass(drawCmdBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}

	template<typename PairIt>
	void loadAssets(PairIt begin, PairIt end)
	{
		while(begin != end){
			models.push_back(Model{});
			constructModel(begin->first, begin->second, (*(models.end() - 1)));
			begin++;
		}
	}

	void setupDescriptorPool()
	{
		// Example uses one ubo 
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2),
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo =
			vks::initializers::descriptorPoolCreateInfo(
				poolSizes.size(),
				poolSizes.data(),
				2);

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
	}

	void setupDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
		{
			// Binding 0 : Vertex shader uniform buffer
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT,
				0),
			// Binding 1 : Fragment shader combined sampler
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				1),
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout =
			vks::initializers::descriptorSetLayoutCreateInfo(
				setLayoutBindings.data(),
				setLayoutBindings.size());

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(
				&descriptorSetLayout,
				1);

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
	}

	void setupDescriptorSet()
	{
		VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;			

		descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);;


		for(auto& model: models){
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &model.descriptorSet));
			writeDescriptorSets = {			
				vks::initializers::writeDescriptorSet(model.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &uniformBuffers.scene.descriptor),	// Binding 0 : Vertex shader uniform buffer			
				vks::initializers::writeDescriptorSet(model.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &model.texture.descriptor)	// Binding 1 : Color map 
			};
			vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

		}

	}

	void preparePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				0,
				VK_FALSE);

		VkPipelineRasterizationStateCreateInfo rasterizationState =
			vks::initializers::pipelineRasterizationStateCreateInfo(
				VK_POLYGON_MODE_FILL,
				VK_CULL_MODE_BACK_BIT,
				VK_FRONT_FACE_CLOCKWISE,
				0);

		VkPipelineColorBlendAttachmentState blendAttachmentState =
			vks::initializers::pipelineColorBlendAttachmentState(
				0xf,
				VK_FALSE);

		VkPipelineColorBlendStateCreateInfo colorBlendState =
			vks::initializers::pipelineColorBlendStateCreateInfo(
				1,
				&blendAttachmentState);

		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			vks::initializers::pipelineDepthStencilStateCreateInfo(
				VK_TRUE,
				VK_TRUE,
				VK_COMPARE_OP_LESS_OR_EQUAL);

		VkPipelineViewportStateCreateInfo viewportState =
			vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

		VkPipelineMultisampleStateCreateInfo multisampleState =
			vks::initializers::pipelineMultisampleStateCreateInfo(
				VK_SAMPLE_COUNT_1_BIT,
				0);

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState =
			vks::initializers::pipelineDynamicStateCreateInfo(
				dynamicStateEnables.data(),
				dynamicStateEnables.size(),
				0);

		// Load shaders
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vks::initializers::pipelineCreateInfo(
				pipelineLayout,
				renderPass,
				0);

		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();

		for(auto& model: models){
			// This example uses two different input states, one for the instanced part and one for non-instanced rendering
			VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
			std::vector<VkVertexInputBindingDescription> bindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

			// Vertex input bindings
			// The instancing pipeline uses a vertex input state with two bindings
			bindingDescriptions = {
				// Binding point 0: Mesh vertex layout description at per-vertex rate
				vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, vertexLayout.stride(), VK_VERTEX_INPUT_RATE_VERTEX),
				// Binding point 1: Instanced data at per-instance rate
				vks::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
			};

			// Vertex attribute bindings
			// Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
			// instanced.vert:
			//	layout (location = 0) in vec3 inPos;		Per-Vertex
			//	...
			//	layout (location = 4) in vec3 instancePos;	Per-Instance
			attributeDescriptions = {
				// Per-vertex attributees
				// These are advanced for each vertex fetched by the vertex shader
				vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 0: Position			
				vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Location 1: Normal			
				vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),		// Location 2: Texture coordinates			
				vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),	// Location 3: Color
				// Per-Instance attributes
				// These are fetched for each instance rendered
				vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 4: Position
				vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Location 5: Rotation
				vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 6, VK_FORMAT_R32_SFLOAT,sizeof(float) * 6),			// Location 6: Scale
			};
			inputState.pVertexBindingDescriptions = bindingDescriptions.data();
			inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

			pipelineCreateInfo.pVertexInputState = &inputState;

			// Instancing pipeline
			shaderStages[0] = loadShader("shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			shaderStages[1] = loadShader("shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
			// Use all input bindings and attribute descriptions
			inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
			inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &model.pipeline));
		}
	}

	void prepareInstanceData()
	{

		for(auto& model: models){
			if(model.instances.empty())
				continue;
			// Staging
			// Instanced data is static, copy to device local memory 
			// This results in better performance

			VK_CHECK_RESULT(vulkanDevice->createBuffer(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&model.instanceBuf,
				model.instances.size() * sizeof(InstanceData),
				model.instances.data()));

			VK_CHECK_RESULT(model.instanceBuf.map());
/*
			struct {
				VkDeviceMemory memory;
				VkBuffer buffer;
			} stagingBuffer;

			VK_CHECK_RESULT(vulkanDevice->createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				model.instanceBuf.size,
				&stagingBuffer.buffer,
				&stagingBuffer.memory,
				model.instances.data()));

			VK_CHECK_RESULT(vulkanDevice->createBuffer(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				model.instanceBuf.size,
				&model.instanceBuf.buffer,
				&model.instanceBuf.memory));

			// Copy to staging buffer
			VkCommandBuffer copyCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			VkBufferCopy copyRegion = { };
			copyRegion.size = model.instanceBuf.size;
			vkCmdCopyBuffer(
				copyCmd,
				stagingBuffer.buffer,
				model.instanceBuf.buffer,
				1,
				&copyRegion);

			VulkanExampleBase::flushCommandBuffer(copyCmd, queue, true);

			model.instanceBuf.descriptor.range = model.instanceBuf.size;
			model.instanceBuf.descriptor.buffer = model.instanceBuf.buffer;
			model.instanceBuf.descriptor.offset = 0;

			// Destroy staging resources
			vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
			vkFreeMemory(device, stagingBuffer.memory, nullptr);
*/
		}
	}

	void prepareUniformBuffers()
	{
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniformBuffers.scene,
			sizeof(uboVS)));

		// Map persistent
		VK_CHECK_RESULT(uniformBuffers.scene.map());

		updateUniformBuffers(true);
	}

	void updateUniformBuffers(bool viewChanged = true)
	{
		if (viewChanged)
		{
			uboVS.projectionMatrix = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
			uboVS.projectionMatrix = glm::rotate(uboVS.projectionMatrix, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
			uboVS.projectionMatrix = glm::rotate(uboVS.projectionMatrix, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
			uboVS.projectionMatrix = glm::translate(uboVS.projectionMatrix, cameraPos);
			uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 1000.0f) * uboVS.projectionMatrix;
			uboVS.viewPos = glm::vec4(-cameraPos, 0.0f);

		}



		memcpy(uniformBuffers.scene.mapped, &uboVS, sizeof(uboVS));

	}

	void updateInstanceBuffers()
	{
		for(auto& model: models){
			if(model.instances.empty())
				continue;
			memcpy(model.instanceBuf.mapped, model.instances.data(), model.instances.size() * sizeof(InstanceData));
		}


	}
	void draw()
	{

		if(shouldRecreateInstances){
			updateInstanceBuffers();
			buildCommandBuffers();
			shouldRecreateInstances = false;
		}


		updateInstanceBuffers();

		VulkanExampleBase::prepareFrame();

		// Command buffer to be sumitted to the queue
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

		// Submit to queue
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanExampleBase::submitFrame();
	}

	void moveModels(){
		for(auto& model: models){
			for(auto& instance: model.instances)
				instance.pos += glm::vec3{1.0f, 0.0f, 0.0f};
		}
		updateInstanceBuffers();
	}

	template<typename PairIt>
	void prepare(PairIt begin, PairIt end)
	{
		std::cout << "Preparing Vulkan" << std::endl;
		VulkanExampleBase::prepare();
		std::cout << "Vk base prepared" << std::endl;
		loadAssets(begin, end);
		std::cout << "Assets loaded" << std::endl;
		prepareInstanceData();
		std::cout << "Instance data prepared" << std::endl;
		prepareUniformBuffers();
		std::cout << "Uniform buffers prepared" << std::endl;
		setupDescriptorSetLayout();
		std::cout << "DescriptoLayout prepared" << std::endl;
		preparePipelines();
		std::cout << "Pipelines prepared" << std::endl;
		setupDescriptorPool();
		std::cout << "Descriptor pool prepared" << std::endl;
		setupDescriptorSet();
		std::cout << "Desctiptor set prepared" << std::endl;
		buildCommandBuffers();
		std::cout << "Command Buffer prepared" << std::endl;
		prepared = true;
	}


	InstanceView const* addInstance(int model_id){


		Model& model = models[model_id];
		model.instances.emplace_back();
		model.instanceViews.emplace_back(&(*(model.instances.end() - 1)));
		
		auto view_it = model.instanceViews.begin();
		auto inst_it = model.instances.begin();
		auto view_end = model.instanceViews.end();
		
		for(; view_it != view_end; view_it++, inst_it++)
			(*view_it).reset(&(*inst_it));

		model.instanceBuf.destroy();

		model.instanceBuf.size = model.instances.size() * sizeof(InstanceData);

		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&model.instanceBuf,
			model.instances.size() * sizeof(InstanceData),
			model.instances.data()));

		VK_CHECK_RESULT(model.instanceBuf.map());

		shouldRecreateInstances = true;


		return &(*(--model.instanceViews.end()));
	}

	void returnInstance(InstanceView const* instance){
		for(auto& model: models){
			bool flag = false;
			auto view_it = model.instanceViews.begin();
			auto inst_it = model.instances.begin();
			while( inst_it != model.instances.end()){
				if(!flag){
					if(&(*view_it) == instance){
						view_it = model.instanceViews.erase(view_it);
						inst_it = model.instances.erase(inst_it);
						flag = true;
						continue;
					}
					else{
						++view_it;
						++inst_it;
					}
				}
				else{
					(*view_it).reset(&(*inst_it));
						++view_it;
						++inst_it;					
				}
			}


		}

		shouldRecreateInstances = true;
	}

	virtual void render()
	{
		if (!prepared)
		{
			return;
		}
		draw();
		if (!paused)
		{
			updateUniformBuffers(false);
		}
	}




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
