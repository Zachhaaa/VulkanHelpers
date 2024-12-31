#include "VulkanHelpers.hpp"

#include <cstdio> 
#include <cassert>
#include <memory>
#include <vector>

VkResult vlknh::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void vlknh::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

bool vlknh::getQueueFamilyFlagsIndex(VkPhysicalDevice physicalDevice, VkQueueFlags desiredQueueFlags, uint32_t* index) {
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilies.data());

	*index = 0; 
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & desiredQueueFlags)
			return true;

		(*index)++; 
	}
	return false; 
}
bool vlknh::getQueueFamilyPresentIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* index) {
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

	*index = 0; 
	for (; *index < queueFamilyPropertyCount; (*index)++) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, *index, surface, &presentSupport);
		if (presentSupport) return true;
	}
	return false;
}
VkShaderModule vlknh::createShaderModule(VkDevice device, const char* fileName) {

	FILE* file = fopen(fileName, "rb");
	assert(file != NULL);

	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	std::unique_ptr<char[]> byteCode(new char[fileSize]);

	fread(byteCode.get(), sizeof(char), fileSize, file);

	fclose(file);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fileSize;
	createInfo.pCode    = (uint32_t*)byteCode.get();

	VkShaderModule shaderModule;
	VkResult err = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
	assert(err == VK_SUCCESS && "Shader Module Creation failed");

	return shaderModule;

}
void vlknh::createBuffer(VkDevice device, const BufferCreateInfo& createInfo, VkBuffer* buff, VkDeviceMemory* buffMem) {

	VkBufferCreateInfo buffInfo{};
	buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffInfo.size        = createInfo.size;
	buffInfo.usage       = createInfo.usage;
	buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult err = vkCreateBuffer(device, &buffInfo, nullptr, buff);
	assert(err == VK_SUCCESS && "Buffer creation failed");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, *buff, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(createInfo.physicalDevice, &memProps);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	uint32_t i = 0;
	for (; i < memProps.memoryTypeCount; i++) {
		if (memRequirements.memoryTypeBits & (1 << i) && memProps.memoryTypes[i].propertyFlags & createInfo.properties) {
			break;
		}
	}
	allocInfo.memoryTypeIndex = i;

	err = vkAllocateMemory(device, &allocInfo, nullptr, buffMem);
	assert(err == VK_SUCCESS && "Buffer allocation failed");

	vkBindBufferMemory(device, *buff, *buffMem, 0);

}
void vlknh::loadBuffer(VkDevice device, VkDeviceMemory buffMem, void* buffData, VkDeviceSize buffSize) {

	void* data;
	vkMapMemory(device, buffMem, 0, buffSize, 0, &data);
	memcpy(data, buffData, buffSize);
	vkUnmapMemory(device, buffMem);

}
void vlknh::loadDeviceLocalBuffer(VkDevice device, const LoadLocalBufferInfo& loadInfo,  VkBuffer deviceLocalBuffer) {
	
	VkBuffer       stageBuff; 
	VkDeviceMemory stageMem; 

	vlknh::BufferCreateInfo buffInfo{};
    buffInfo.physicalDevice = loadInfo.physicalDevice;
    buffInfo.size           = loadInfo.size;
    buffInfo.usage          = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffInfo.properties     = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	vlknh::createBuffer(device, buffInfo, &stageBuff, &stageMem);
	vlknh::loadBuffer(device, stageMem, loadInfo.data, loadInfo.size);

	SingleTimeCommandBuffer singleTimeBuff(device, loadInfo.commandPool, loadInfo.commandQueue); 
	singleTimeBuff.copy(loadInfo.size, stageBuff, deviceLocalBuffer);
	singleTimeBuff.submit(); 

	vkFreeMemory(device, stageMem, nullptr);
	vkDestroyBuffer(device, stageBuff, nullptr);

}
VkDescriptorSet vlknh::createTextureImage(VkDevice device, const TextureImageCreateInfo& createInfo, TextureImageResources& imgResources) {

	assert(createInfo.pPixelData != nullptr); 

	VkBuffer       stagingBuffer; 
	VkDeviceMemory stagingBuffMem; 

	BufferCreateInfo buffInfo{}; 
	buffInfo.physicalDevice	= createInfo.physicalDevice;
	buffInfo.size      		= createInfo.imageSize.width * createInfo.imageSize.height * 4; 
	buffInfo.usage          = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffInfo.properties     = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	createBuffer(device, buffInfo, &stagingBuffer, &stagingBuffMem); 
	loadBuffer(device, stagingBuffMem, createInfo.pPixelData, buffInfo.size); 

	VkImageCreateInfo imageInfo{}; 
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType     = VK_IMAGE_TYPE_2D;
	imageInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.extent        = { createInfo.imageSize.width , createInfo.imageSize.height, 1 }; 
	imageInfo.mipLevels     = 1;
	imageInfo.arrayLayers   = 1;
	imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL; 
	imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE; 
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult err = vkCreateImage(device, &imageInfo, nullptr, imgResources.pTexImg); 
	assert(err == VK_SUCCESS && "Texture image creation failed"); 

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, *imgResources.pTexImg, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(createInfo.physicalDevice, &memProps);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	uint32_t i = 0;
	for (; i < memProps.memoryTypeCount; i++) {
		if (memRequirements.memoryTypeBits & (1 << i) && memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			break;
		}
	}

	allocInfo.memoryTypeIndex = i;

	err = vkAllocateMemory(device, &allocInfo, nullptr, imgResources.pTexImgMem);
	assert(err == VK_SUCCESS && "Image Buffer allocation failed");

	vkBindImageMemory(device, *imgResources.pTexImg, *imgResources.pTexImgMem, 0);

	SingleTimeCommandBuffer singleTimeBuff(device, createInfo.commandPool, createInfo.graphicsQueue);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image               = *imgResources.pTexImg;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
		singleTimeBuff.m_cmdBuff,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel       = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount     = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { createInfo.imageSize.width, createInfo.imageSize.height, 1 };

	vkCmdCopyBufferToImage(
		singleTimeBuff.m_cmdBuff,
		stagingBuffer,
		*imgResources.pTexImg,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		singleTimeBuff.m_cmdBuff,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	
	singleTimeBuff.submit(); 

	vkFreeMemory        (device, stagingBuffMem, nullptr);
	vkDestroyBuffer     (device, stagingBuffer, nullptr);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image    = *imgResources.pTexImg;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format   = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel   = 0;
	viewInfo.subresourceRange.levelCount     = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount     = 1;

	err = vkCreateImageView(device, &viewInfo, nullptr, imgResources.pTexImgView); 
	assert(err == VK_SUCCESS && "Texture image view creation failed"); 

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool     = createInfo.descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts        = &createInfo.descriptorSetLayout;

	VkDescriptorSet desciptorSet; 
	
	err = vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &desciptorSet);
	assert(err == VK_SUCCESS && "Descriptor set creation failed");
	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler     = createInfo.sampler;
	descriptorImageInfo.imageView   = *imgResources.pTexImgView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeDescriptor{};
	writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptor.dstSet          = desciptorSet;
	writeDescriptor.dstBinding      = 0;
	writeDescriptor.dstArrayElement = 0;
	writeDescriptor.descriptorCount = 1;
	writeDescriptor.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptor.pImageInfo      = &descriptorImageInfo;

	vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, nullptr);

	return desciptorSet; 

}

vlknh::SingleTimeCommandBuffer::SingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue deviceQueue, bool beginRecordingCmds) {
	
	m_device          = device; 
	m_deviceQueue     = deviceQueue;
	m_commandPool     = commandPool;
	m_bufferRecording = false;
	m_bufferSubmitted = false;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool        = commandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(m_device, &allocInfo, &m_cmdBuff);

	if (beginRecordingCmds) {
		begin();
	}
}
void vlknh::SingleTimeCommandBuffer::begin() {

	assert(!m_bufferRecording && "Already began recording commands");
	if (m_bufferRecording) return;

	m_bufferRecording = true;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(m_cmdBuff, &beginInfo);

}
void vlknh::SingleTimeCommandBuffer::copy(VkDeviceSize buffSize, VkBuffer src, VkBuffer dst) {

	assert(m_bufferRecording && "Never called begin() on command buffer."); 

	VkBufferCopy copyRegion = { 0, 0, buffSize };
	vkCmdCopyBuffer(m_cmdBuff, src, dst, 1, &copyRegion);

}
void vlknh::SingleTimeCommandBuffer::submit() {
	
	assert(!m_bufferSubmitted && "Already submitted the buffer"); 
	if (m_bufferSubmitted) return;

	vkEndCommandBuffer(m_cmdBuff);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &m_cmdBuff;

	vkQueueSubmit(m_deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);

	vkQueueWaitIdle(m_deviceQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_cmdBuff);

}
vlknh::SingleTimeCommandBuffer::~SingleTimeCommandBuffer() {
	assert(m_bufferSubmitted); 
	if(m_bufferSubmitted) return; 
	submit(); 
}