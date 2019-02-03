/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// INCLUDES & DEFINES////////

#pragma once

#include <vector> 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include "vulkan/vulkan.h"

#define ASSERT_VULKAN(val)\
		if(val != VK_SUCCESS) {\
			__debugbreak();\
		}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// GLOBAL VARS ////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// FUNCS ////////

uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties); //TODO check if valid
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	__debugbreak();
	throw std::runtime_error("Found no correct memory type!");
}

void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer &buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory) {
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = deviceSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	VkResult result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
	ASSERT_VULKAN(result);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
	ASSERT_VULKAN(result);

	vkBindBufferMemory(device, buffer, deviceMemory, 0);
}

template <typename T>
void createAndUploadBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, std::vector<T> data, VkBufferUsageFlags usage, VkBuffer &buffer, VkDeviceMemory &deviceMemory) {
	VkDeviceSize bufferSize = sizeof(T) * data.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory);

	void *rawData;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &rawData);
	memcpy(rawData, data.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(device, physicalDevice, bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceMemory);

	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
