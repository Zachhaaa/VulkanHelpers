#pragma once

#include <vulkan/vulkan.h>

namespace vlknh {

struct BufferCreateInfo {

	VkPhysicalDevice      physicalDevice;
	VkDeviceSize          size;
	VkBufferUsageFlags    usage;
	VkMemoryPropertyFlags properties;

};

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

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void     DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

bool            getQueueFamilyFlagsIndex   (VkPhysicalDevice physicalDevice, VkQueueFlags desiredQueueFlags, uint32_t* index);
bool            getQueueFamilyPresentIndex (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* index); 
VkShaderModule  createShaderModule         (VkDevice device, const char* fileName);
void            createBuffer               (VkDevice device, const BufferCreateInfo& createInfo, VkBuffer* buff, VkDeviceMemory* buffMem);
void            loadBuffer                 (VkDevice device, VkDeviceMemory buffMem, void* buffData, VkDeviceSize buffSize);
VkDescriptorSet createTextureImage         (VkDevice device, const TextureImageCreateInfo& createInfo, TextureImageResources& imgResources);

	namespace SingleTimeCommandBuffer {

	void        begin  (VkDevice device, VkCommandPool commandPool, VkCommandBuffer* cmdBuff);
	inline void copy   (VkCommandBuffer cmdBuff, VkDeviceSize buffSize, VkBuffer src, VkBuffer dst) {
		VkBufferCopy copyRegion = { 0, 0, buffSize };
		vkCmdCopyBuffer(cmdBuff, src, dst, 1, &copyRegion);
	}
	void        submit (VkDevice device, VkCommandBuffer cmdBuff, VkCommandPool commandPool, VkQueue graphicsQueue);

	}

}