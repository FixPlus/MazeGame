
#include "VulkanExample.h"


VulkanExample::VulkanExample(std::string windowName) : VulkanExampleBase(ENABLE_VALIDATION)
{
	zoom = -2.5f;
	rotation = { 0.0f, 15.0f, 0.0f };
	title = windowName;
	settings.overlay = true;
    UIOverlay.visible = true;
}

VulkanExample::~VulkanExample()
{
	// Clean up used Vulkan resources 
	// Note : Inherited destructor cleans up resources stored in base class

	for(int i = 0; i < NUMBER_OF_TEXTURES; i++)
		destroyTextureImage(texture[i]);

	vkDestroyPipeline(device, pipelines.solid, nullptr);

	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vertexBuffer.destroy();
	indexBuffer.destroy();
	uniformBufferVS.destroy();
}

// Enable physical device features required for this example				
void VulkanExample::getEnabledFeatures()
{
	// Enable anisotropic filtering if supported
	if (deviceFeatures.samplerAnisotropy) {
		enabledFeatures.samplerAnisotropy = VK_TRUE;
	};
}

/*
	Upload texture image data to the GPU

	Vulkan offers two types of image tiling (memory layout):

	Linear tiled images:
		These are stored as is and can be copied directly to. But due to the linear nature they're not a good match for GPUs and format and feature support is very limited.
		It's not advised to use linear tiled images for anything else than copying from host to GPU if buffer copies are not an option.
		Linear tiling is thus only implemented for learning purposes, one should always prefer optimal tiled image.

	Optimal tiled images:
		These are stored in an implementation specific layout matching the capability of the hardware. They usually support more formats and features and are much faster.
		Optimal tiled images are stored on the device and not accessible by the host. So they can't be written directly to (like liner tiled images) and always require 
		some sort of data copy, either from a buffer or	a linear tiled image.
	
	In Short: Always use optimal tiled images for rendering.
*/
void VulkanExample::loadTexture()
{
	// We use the Khronos texture format (https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/) 

	VkBool32 useStaging = true;

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

for(int i = 0; i < NUMBER_OF_TEXTURES; i++){
		std::string filename = texture_folder + texture_files[i];//"data/metalplate01_rgba.ktx";
		// Texture data contains 4 channels (RGBA) with unnormalized 8-bit values, this is the most commonly supported format

	#if defined(__ANDROID__)
		// Textures are stored inside the apk on Android (compressed)
		// So they need to be loaded via the asset manager
		AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
		assert(asset);
		size_t size = AAsset_getLength(asset);
		assert(size > 0);

		void *textureData = malloc(size);
		AAsset_read(asset, textureData, size);
		AAsset_close(asset);

		gli::texture2d tex2D(gli::load((const char*)textureData, size));
	#else
		gli::texture2d tex2D(gli::load(filename));
	#endif

		assert(!tex2D.empty());

		texture[i].width = static_cast<uint32_t>(tex2D[0].extent().x);
		texture[i].height = static_cast<uint32_t>(tex2D[0].extent().y);
		texture[i].mipLevels = static_cast<uint32_t>(tex2D.levels());

		// We prefer using staging to copy the texture data to a device local optimal image

		// Only use linear tiling if forced
		bool forceLinearTiling = false;
		if (forceLinearTiling) {
			// Don't use linear if format is not supported for (linear) shader sampling
			// Get device properites for the requested texture format
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
			useStaging = !(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
		}

		VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs = {};

		if (useStaging) {
			// Copy data to an optimal tiled image
			// This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

			// Create a host-visible staging buffer that contains the raw image data
			// This buffer will be the data source for copying texture data to the optimal tiled image on the device
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
			bufferCreateInfo.size = tex2D.size();
			// This buffer is used as a transfer source for the buffer copy
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		
			VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

			// Get memory requirements for the staging buffer (alignment, memory type bits)
			vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);
			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type index for a host visible buffer
			memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
			VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

			// Copy texture data into host local staging buffer
			uint8_t *data;
			VK_CHECK_RESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
			memcpy(data, tex2D.data(), tex2D.size());
			vkUnmapMemory(device, stagingMemory);

			// Setup buffer copy regions for each mip level
			std::vector<VkBufferImageCopy> bufferCopyRegions;
			uint32_t offset = 0;

			for (uint32_t j = 0; j < texture[i].mipLevels; j++) {
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = j;
				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[j].extent().x);
				bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[j].extent().y);
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopyRegion);

				offset += static_cast<uint32_t>(tex2D[j].size());
			}

			// Create optimal tiled target image on the device
			VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = texture[i].mipLevels;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			// Set initial layout of the image to undefined
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.extent = { texture[i].width, texture[i].height, 1 };
			imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture[i].image));

			vkGetImageMemoryRequirements(device, texture[i].image, &memReqs);
			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture[i].deviceMemory));
			VK_CHECK_RESULT(vkBindImageMemory(device, texture[i].image, texture[i].deviceMemory, 0));

			VkCommandBuffer copyCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			// Image memory barriers for the texture image

			// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
			VkImageSubresourceRange subresourceRange = {};
			// Image only contains color data
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			// Start at first mip level
			subresourceRange.baseMipLevel = 0;
			// We will transition on all mip levels
			subresourceRange.levelCount = texture[i].mipLevels;
			// The 2D texture only has one layer
			subresourceRange.layerCount = 1;

			// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
			VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
			imageMemoryBarrier.image = texture[i].image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
			vkCmdPipelineBarrier(
				copyCmd,
				VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);

			// Copy mip levels from staging buffer
			vkCmdCopyBufferToImage(
				copyCmd,
				stagingBuffer,
				texture[i].image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32_t>(bufferCopyRegions.size()),
				bufferCopyRegions.data());

			// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
			// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			vkCmdPipelineBarrier(
				copyCmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);

			// Store current layout for later reuse
			texture[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VulkanExampleBase::flushCommandBuffer(copyCmd, queue, true);

			// Clean up staging resources
			vkFreeMemory(device, stagingMemory, nullptr);
			vkDestroyBuffer(device, stagingBuffer, nullptr);
//			std::cout << "Texture[" << i << "].deviceMemory = " << reinterpret_cast<long>(texture[i].deviceMemory) << std::endl;
		} else {
			// Copy data to a linear tiled image

			VkImage mappableImage;
			VkDeviceMemory mappableMemory;

			// Load mip map level 0 to linear tiling image
			VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			imageCreateInfo.extent = { texture[i].width, texture[i].height, 1 };
			VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &mappableImage));

			// Get memory requirements for this image like size and alignment
			vkGetImageMemoryRequirements(device, mappableImage, &memReqs);
			// Set memory allocation size to required memory size
			memAllocInfo.allocationSize = memReqs.size;
			// Get memory type that can be mapped to host memory
			memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &mappableMemory));
			VK_CHECK_RESULT(vkBindImageMemory(device, mappableImage, mappableMemory, 0));

			// Map image memory
			void *data;
			VK_CHECK_RESULT(vkMapMemory(device, mappableMemory, 0, memReqs.size, 0, &data));
			// Copy image data of the first mip level into memory
			memcpy(data, tex2D[0].data(), tex2D[0].size());
			vkUnmapMemory(device, mappableMemory);

			// Linear tiled images don't need to be staged and can be directly used as textures
			texture[i].image = mappableImage;
			texture[i].deviceMemory = mappableMemory;
			texture[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			
			// Setup image memory barrier transfer image to shader read layout
			VkCommandBuffer copyCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			// The sub resource range describes the regions of the image we will be transition
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;

			// Transition the texture image layout to shader read, so it can be sampled from
			VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
			imageMemoryBarrier.image = texture[i].image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			vkCmdPipelineBarrier(
				copyCmd,
				VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);

			VulkanExampleBase::flushCommandBuffer(copyCmd, queue, true);
		}
	// Create a texture sampler
	// In Vulkan textures are accessed by samplers
	// This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
	// Note: Similar to the samplers available with OpenGL 3.3

		VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.mipLodBias = 0.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		// Set max level-of-detail to mip level count of the texture
		sampler.maxLod = (useStaging) ? (float)texture[i].mipLevels : 0.0f;
		// Enable anisotropic filtering
		// This feature is optional, so we must check if it's supported on the device
		if (vulkanDevice->features.samplerAnisotropy) {
			// Use max. level of anisotropy for this example
			sampler.maxAnisotropy = vulkanDevice->properties.limits.maxSamplerAnisotropy;
			sampler.anisotropyEnable = VK_TRUE;
		} else {
			// The device does not support anisotropic filtering
			sampler.maxAnisotropy = 1.0;
			sampler.anisotropyEnable = VK_FALSE;
		}
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture[i].sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges

		VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
		// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;
		// Linear tiling usually won't support mip maps
		// Only set mip map count if optimal tiling is used
		view.subresourceRange.levelCount = (useStaging) ? texture[i].mipLevels : 1;
		// The view will be based on the texture's image
		view.image = texture[i].image;
		VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture[i].view));
	}
}

// Free all Vulkan resources used by a texture object
void VulkanExample::destroyTextureImage(Texture texture)
{
	vkDestroyImageView(device, texture.view, nullptr);
	vkDestroyImage(device, texture.image, nullptr);
	vkDestroySampler(device, texture.sampler, nullptr);
	vkFreeMemory(device, texture.deviceMemory, nullptr);
}

void VulkanExample::buildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = defaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
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

		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

		drawUI(drawCmdBuffers[i]);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}
}

void VulkanExample::draw()
{
	VulkanExampleBase::prepareFrame();

	// Command buffer to be sumitted to the queue
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

	// Submit to queue
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	VulkanExampleBase::submitFrame();
}

void VulkanExample::prepareVertices(int n_stat_verts, int n_dyn_verts)
{

	localDynamicVertices.clear();
	localDynamicIndices.clear();

	Vertex example = Vertex{};
	localDynamicVertices.resize(n_dyn_verts, example);

	for(int i = 0; i < n_dyn_verts; i++)
		localDynamicIndices.push_back(i);


	localDynamicVerticesSize = static_cast<uint32_t>(localDynamicVertices.size()) * sizeof(Vertex);

	localDynamicIndicesSize =  static_cast<uint32_t>(localDynamicIndices.size()) * sizeof(uint32_t);


	localStaticVertices.clear();
	localStaticIndices.clear();

	example = Vertex{};
	localStaticVertices.resize(n_stat_verts, example);

	for(int i = 0; i < n_stat_verts; i++)
		localStaticIndices.push_back(i + n_dyn_verts);


	localStaticVerticesSize = static_cast<uint32_t>(localStaticVertices.size()) * sizeof(Vertex);

	localStaticIndicesSize = static_cast<uint32_t>(localStaticIndices.size()) * sizeof(uint32_t);

	std::cout << "Static vertices: " << localStaticVerticesSize/ sizeof(Vertex) << "; Dynamic vertices: " << localDynamicVerticesSize/ sizeof(Vertex) << std::endl;

	// Setup vertices for a single uv-mapped quad made from two triangles
	std::vector<Vertex> vertices;
	for(int i = 0; i < localDynamicVertices.size(); i++){
		vertices.push_back(localDynamicVertices[i]);
	}

	for(int i = 0; i < localStaticVertices.size(); i++){
		vertices.push_back(localStaticVertices[i]);
	}


	// Setup indices
	std::vector<uint32_t> indices;

	for(int i = 0; i < localDynamicIndices.size(); i++){
		indices.push_back(localDynamicIndices[i]);
	}

	for(int i = 0; i < localStaticVertices.size(); i++){
		indices.push_back(localStaticIndices[i]);
	}


	indexCount = static_cast<uint32_t>(indices.size());

	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the gpu memory
	// Vertex buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexBuffer,
		vertices.size() * sizeof(Vertex),
		vertices.data()));
	// Index buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexBuffer,
		indices.size() * sizeof(uint32_t),
		indices.data()));
}

void VulkanExample::setupVertexDescriptions()
{
	// Binding description
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0] =
		vks::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID, 
			sizeof(Vertex), 
			VK_VERTEX_INPUT_RATE_VERTEX);

	// Attribute descriptions
	// Describes memory layout and shader positions
	vertices.attributeDescriptions.resize(4);
	// Location 0 : Position
	vertices.attributeDescriptions[0] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, position));			
	// Location 1 : Texture coordinates
	vertices.attributeDescriptions[1] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, uv));
	// Location 2 : Vertex color
	vertices.attributeDescriptions[2] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, color));
	// Location 3 : Vertex normal
	vertices.attributeDescriptions[3] =
		vks::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			3,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, normal));

	vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void VulkanExample::setupDescriptorPool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NUMBER_OF_TEXTURES),
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = 
		vks::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data(),
			NUMBER_OF_TEXTURES + 1);

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void VulkanExample::setupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = 
	{
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
			VK_SHADER_STAGE_VERTEX_BIT, 
			0),
	};

	for(int i = 1; i < NUMBER_OF_TEXTURES + 1; i++){
		// Binding i : Fragment shader image sampler
		setLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
			VK_SHADER_STAGE_FRAGMENT_BIT,
			i));
	}

	VkDescriptorSetLayoutCreateInfo descriptorLayout = 
		vks::initializers::descriptorSetLayoutCreateInfo(
			setLayoutBindings.data(),
			static_cast<uint32_t>(setLayoutBindings.size()));

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(
			&descriptorSetLayout,
			1);

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void VulkanExample::setupDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo = 
		vks::initializers::descriptorSetAllocateInfo(
			descriptorPool,
			&descriptorSetLayout,
			1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));


	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			// Binding 0 : Vertex shader uniform buffer
			vks::initializers::writeDescriptorSet(
				descriptorSet, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				0, 
				&uniformBufferVS.descriptor)		
	};

	// Setup a descriptor image info for the current texture to be used as a combined image sampler
	for(int i = 1; i < NUMBER_OF_TEXTURES + 1; i++){
		VkDescriptorImageInfo textureDescriptor;
		textureDescriptor.imageView = texture[i - 1].view;				// The image's view (images are never directly accessed by the shader, but rather through views defining subresources)
		textureDescriptor.sampler = texture[i - 1].sampler;			// The sampler (Telling the pipeline how to sample the texture, including repeat, border, etc.)
		textureDescriptor.imageLayout = texture[i - 1].imageLayout;	// The current layout of the image (Note: Should always fit the actual use, e.g. shader read)


		writeDescriptorSets.push_back(
			// Binding 1 : Fragment shader texture sampler
			//	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
			vks::initializers::writeDescriptorSet(
				descriptorSet, 				
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// The descriptor set will use a combined image sampler (sampler and image could be split)
				i,												// Shader binding point 1
				&textureDescriptor)								// Pointer to the descriptor image for our texture
		);
	}

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void VulkanExample::preparePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vks::initializers::pipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vks::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
			static_cast<uint32_t>(dynamicStateEnables.size()),
			0);

	// Load shaders
	std::array<VkPipelineShaderStageCreateInfo,2> shaderStages;

	shaderStages[0] = loadShader("shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader("shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vks::initializers::pipelineCreateInfo(
			pipelineLayout,
			renderPass,
			0);

	pipelineCreateInfo.pVertexInputState = &vertices.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid));
}

// Prepare and initialize uniform buffer containing shader uniforms
void VulkanExample::prepareUniformBuffers()
{
	// Vertex shader uniform buffer block
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBufferVS,
		sizeof(uboVS),
		&uboVS));

	updateUniformBuffers();
}

void VulkanExample::updateUniformBuffers()
{
	// Vertex shader
	uboVS.projectionMatrix = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.projectionMatrix = glm::rotate(uboVS.projectionMatrix, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.projectionMatrix = glm::rotate(uboVS.projectionMatrix, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	uboVS.projectionMatrix = glm::translate(uboVS.projectionMatrix, cameraPos);
	uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 1000.0f) * uboVS.projectionMatrix;
	uboVS.viewPos = glm::vec4(-cameraPos, 0.0f);

	uboVS.lightDirection = lightDirection;

	VK_CHECK_RESULT(uniformBufferVS.map());
	memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
	uniformBufferVS.unmap();

	VK_CHECK_RESULT(vertexBuffer.map());
	memcpy(vertexBuffer.mapped,localDynamicVertices.data(), localDynamicVerticesSize);
	uniformBufferVS.unmap();
}


void VulkanExample::updateStaticVertices(){
	uint8_t *pData;
	VK_CHECK_RESULT(vertexBuffer.map());
	memcpy(vertexBuffer.mapped + localDynamicVerticesSize,localStaticVertices.data(), localStaticVerticesSize);
	uniformBufferVS.unmap();

}


void VulkanExample::prepare(int n_stat_verts, int n_dyn_verts)
{
	VulkanExampleBase::prepare();
	loadTexture();
	prepareVertices(n_stat_verts, n_dyn_verts);
	setupVertexDescriptions();
	prepareUniformBuffers();
	setupDescriptorSetLayout();
	preparePipelines();
	setupDescriptorPool();
	setupDescriptorSet();
	buildCommandBuffers();
	prepared = true;
}

void VulkanExample::render()
{
	if (!prepared)
		return;
	draw();
}

void VulkanExample::mouseMoved(double x, double y, bool &handled){
//		std::cout << mousePos.x << " " << x << std::endl;
//		std::cout << mousePos.y << " " << y << std::endl;
/*	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
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
*/
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
	if (overlay->header("Settings")) {
		if (overlay->sliderFloat("LOD bias", &uboVS.lodBias, 0.0f, 10)) {
			updateUniformBuffers();
		}
	}
}

void VulkanExample::viewChanged()
{
	// This function is called by the base example class each time the view is changed by user input
	updateUniformBuffers();
}

