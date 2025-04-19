#pragma once

#include <vulkan/vulkan.h>

namespace vlknh {

struct ResourceContext {

	VkPhysicalDevice      physicalDevice;
	VkDevice              device;
	VkQueue               graphicsQueue;
	VkCommandPool         commandPool;

};
/*
struct TextureImageCreateInfo {

	VkPhysicalDevice      physicalDevice;
	VkCommandPool         commandPool;
	VkQueue               graphicsQueue;
	VkDescriptorPool      descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkSampler             sampler;
	VkExtent2D            imageSize;
	unsigned char*        pPixelData;

};
struct TextureImageResources {

	VkImage*         pTexImg;
	VkDeviceMemory*  pTexImgMem;
	VkImageView*     pTexImgView;

};
*/

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void     destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

bool            getQueueFamilyFlagsIndex(VkPhysicalDevice physicalDevice, VkQueueFlags desiredQueueFlags, uint32_t* index);
bool            getQueueFamilyPresentIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* index);
VkShaderModule  createShaderModule(VkDevice device, const char* fileName);

//VkDescriptorSet createTextureImage         (VkDevice device, const TextureImageCreateInfo& createInfo, TextureImageResources& imgResources);

class Buffer {
public:

	void create(const ResourceContext* resourceContext, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void createLoadDeviceLocal(const ResourceContext* resourceContext, VkBufferUsageFlags usage, size_t size, const void* data); 	// Properties automatically has VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, no need to include it.
	void load(const void* data);
	void destroy();
	     ~Buffer(); 

public:

	VkBuffer               m_Buffer = VK_NULL_HANDLE;
	VkDeviceMemory         m_Memory = VK_NULL_HANDLE;
	size_t                 m_Size = 0;

private:

	const ResourceContext* m_ResourceContext = nullptr;

};


class SingleTimeCommandBuffer {
public:

	SingleTimeCommandBuffer(const ResourceContext* resourceContext, bool beginRecordingCmds = true);

	void begin();
	void copy(size_t buffSize, VkBuffer src, VkBuffer dst);
	void submit();

	~SingleTimeCommandBuffer();

private:

	const ResourceContext* m_ResourceContext = nullptr;
	VkCommandBuffer        m_CmdBuffer = VK_NULL_HANDLE;
	bool                   m_BufferRecording = false;

};

}