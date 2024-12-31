#pragma once

#include <vulkan/vulkan.h>

namespace vlknh {

struct BufferCreateInfo {

	VkPhysicalDevice      physicalDevice;
	VkDeviceSize          size;
	VkBufferUsageFlags    usage;
	VkMemoryPropertyFlags properties;

};

struct LoadLocalBufferInfo {

	VkPhysicalDevice physicalDevice;
	VkCommandPool    commandPool; 
	VkQueue          commandQueue; 
	VkDeviceSize     size;
	void*            data; 

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

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void     destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

bool            getQueueFamilyFlagsIndex   (VkPhysicalDevice physicalDevice, VkQueueFlags desiredQueueFlags, uint32_t* index);
bool            getQueueFamilyPresentIndex (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* index); 
VkShaderModule  createShaderModule         (VkDevice device, const char* fileName);
void            createBuffer               (VkDevice device, const BufferCreateInfo& createInfo, VkBuffer* buff, VkDeviceMemory* buffMem);
void            loadBuffer                 (VkDevice device, VkDeviceMemory buffMem, void* buffData, VkDeviceSize buffSize);
void            loadDeviceLocalBuffer      (VkDevice device, const LoadLocalBufferInfo& loadInfo, VkBuffer deviceLocalBuffer);
VkDescriptorSet createTextureImage         (VkDevice device, const TextureImageCreateInfo& createInfo, TextureImageResources& imgResources);

	class SingleTimeCommandBuffer {
	public: 

		SingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue deviceQueue, bool beginRecordingCmds = true);

		void begin  ();
		void copy   (VkDeviceSize buffSize, VkBuffer src, VkBuffer dst); 
		void submit ();
		
		~SingleTimeCommandBuffer();

	public:

		VkCommandBuffer m_cmdBuff; 

	private: 

		VkDevice        m_device; 
		VkQueue         m_deviceQueue; 
		VkCommandPool   m_commandPool;
		bool			m_bufferRecording; 
		bool            m_bufferSubmitted; 

	}; 

}