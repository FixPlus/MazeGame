#pragma once



struct InstanceData {
	glm::vec3 pos = {0.0f, 0.0f, 0.0f};
	glm::vec3 rot = {0.0f, 0.0f, 0.0f};
	float scale = 1.0f;
};


struct InstanceBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	size_t size = 0;
	VkDescriptorBufferInfo descriptor;
};

class InstanceView {
	InstanceData* instance_;
public:
	InstanceView(InstanceData* inst = nullptr): instance_(inst){}
	void reset(InstanceData* newInstance = nullptr) {
		instance_ = newInstance;
	}

	InstanceData* instance() const{
		return instance_;
	};

};
