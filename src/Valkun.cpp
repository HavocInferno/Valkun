// Valkun.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
//#include "vulkan/vulkan.h"


#define ASSERT_VULKAN(val)\
		if(val != VK_SUCCESS) {\
			__debugbreak();\
		}
VkResult	result;

VkInstance	instance; 
VkSurfaceKHR surface; 
VkDevice	device;
VkSwapchainKHR swapchain; 
VkImageView *imageViews;
VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;
uint32_t numImagesInSwapchain = 0;

GLFWwindow *window;
const uint32_t WIDTH = 960;
const uint32_t HEIGHT = 540;

void printStats(VkPhysicalDevice &device) {
	VkPhysicalDeviceProperties properties; 
	vkGetPhysicalDeviceProperties(device, &properties); 

	std::cout << std::endl; 
	std::cout << "Name:                     " << properties.deviceName << std::endl; 
	uint32_t apiVer = properties.apiVersion; 
	std::cout << "API Version:              " << VK_VERSION_MAJOR(apiVer) << "." << VK_VERSION_MINOR(apiVer) << "." << VK_VERSION_PATCH(apiVer) << std::endl; 
	std::cout << "Driver Version:           " << properties.driverVersion << std::endl; 
	std::cout << "Vendor ID:                " << properties.vendorID << std::endl; 
	std::cout << "Device ID:                " << properties.deviceID << std::endl; 
	std::cout << "Device Type               " << properties.deviceType << std::endl; 
	std::cout << "discreteQueuePriorities:  " << properties.limits.discreteQueuePriorities << std::endl; 

	VkPhysicalDeviceFeatures features; 
	vkGetPhysicalDeviceFeatures(device, &features); 
	std::cout << "Geometry Shader:          " << features.geometryShader << std::endl; 

	VkPhysicalDeviceMemoryProperties memProp; 
	vkGetPhysicalDeviceMemoryProperties(device, &memProp); 
	
	uint32_t numOfQueueFamilies = 0; 
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numOfQueueFamilies, nullptr); 
	VkQueueFamilyProperties *familyProperties = new VkQueueFamilyProperties[numOfQueueFamilies]; 
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numOfQueueFamilies, familyProperties); 

	std::cout << "Number of Queue Families: " << numOfQueueFamilies << std::endl; 

	for (uint32_t i = 0; i < numOfQueueFamilies; i++) {
		std::cout << std::endl;
		std::cout << "Queue Family #" << i << std::endl;
		std::cout << "VK_QUEUE_GRAPHICS_BIT       " << ((familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_COMPUTE_BIT        " << ((familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_TRANSFER_BIT       " << ((familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_SPARSE_BINDING_BIT " << ((familyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_PROTECTED_BIT      " << ((familyProperties[i].queueFlags & VK_QUEUE_PROTECTED_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_FLAG_BITS_MAX_ENUM " << ((familyProperties[i].queueFlags & VK_QUEUE_FLAG_BITS_MAX_ENUM) != 0) << std::endl;

		std::cout << "Queue Count: " << familyProperties[i].queueCount << std::endl; 
		std::cout << "Timestamp Valid Bits: " << familyProperties[i].timestampValidBits << std::endl; 

		uint32_t width = familyProperties[i].minImageTransferGranularity.width; 
		uint32_t height = familyProperties[i].minImageTransferGranularity.height;
		uint32_t depth = familyProperties[i].minImageTransferGranularity.depth;
		std::cout << "Min Image Timestap Granularity: " << width << ", " << height << ", " << depth << std::endl; 
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities; 
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities); 
	std::cout << "\nSurface capabilities: " << std::endl; 
	std::cout << "\tminImageCount           " << surfaceCapabilities.minImageCount << std::endl;
	std::cout << "\tmaxImageCount           " << surfaceCapabilities.maxImageCount << std::endl;
	std::cout << "\tcurrentExtent           " << surfaceCapabilities.currentExtent.width << "x" << surfaceCapabilities.currentExtent.height << std::endl;
	std::cout << "\tminImageExtent          " << surfaceCapabilities.minImageExtent.width << "x" << surfaceCapabilities.minImageExtent.height << std::endl;
	std::cout << "\tmaxImageExtent          " << surfaceCapabilities.maxImageExtent.width << "x" << surfaceCapabilities.maxImageExtent.height << std::endl;
	std::cout << "\tmaxImageArrayLayers     " << surfaceCapabilities.maxImageArrayLayers << std::endl;
	std::cout << "\tsupportedTransforms     " << surfaceCapabilities.supportedTransforms << std::endl;
	std::cout << "\tcurrentTransform        " << surfaceCapabilities.currentTransform << std::endl;
	std::cout << "\tsupportedCompositeAlpha " << surfaceCapabilities.supportedCompositeAlpha << std::endl;
	std::cout << "\tsupportedUsageFlags     " << surfaceCapabilities.supportedUsageFlags << std::endl;

	uint32_t numFormats = 0; 
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, nullptr); 
	VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[numFormats]; 
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, surfaceFormats); 

	std::cout << std::endl;
	std::cout << "Num of Formats: " << numFormats << std::endl;
	for (int i = 0; i < numFormats; i++) {
		std::cout << "Format: " << surfaceFormats[i].format << std::endl; 
	}

	uint32_t numPresentationModes = 0; 
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numPresentationModes, nullptr); 
	VkPresentModeKHR *presentModes = new VkPresentModeKHR[numPresentationModes]; 
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numPresentationModes, presentModes); 

	std::cout << std::endl; 
	std::cout << "Num of Presentation Modes: " << numPresentationModes << std::endl; 
	for (int i = 0; i < numPresentationModes; i++) {
		std::cout << "Presentaion Mode: " << presentModes[i] << std::endl; 
	}

	std::cout << std::endl; 
	delete[] familyProperties; 
	delete[] surfaceFormats; 
	delete[] presentModes; 
}

std::vector<char> readFile(const std::string &filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate); 

	if (file) {
		size_t fileSize = (size_t)file.tellg(); 
		std::vector<char> fileBuffer(fileSize); 
		file.seekg(0); 
		file.read(fileBuffer.data(), fileSize); 
		file.close(); 
		return fileBuffer;
	}
	else {
		throw std::runtime_error("Failed to open file " + filename); 
	}
}

void startGlfw() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); 

	std::string title = "Valkun Sample (" + std::to_string(WIDTH) + "x" + std::to_string(HEIGHT) + ")";
	window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
}

void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule) {
	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = code.size();
	shaderCreateInfo.pCode = (uint32_t*)code.data();

	VkResult result = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, shaderModule); 
	ASSERT_VULKAN(result); 
}

void startVulkan() {
	std::cout << "Hello Vorld!" << std::endl << std::endl;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Valkun Engine Sample";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Valkun Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint32_t numOfLayers = 0;
	result = vkEnumerateInstanceLayerProperties(&numOfLayers, nullptr);
	std::vector<VkLayerProperties> layers = std::vector<VkLayerProperties>(numOfLayers);
	result = vkEnumerateInstanceLayerProperties(&numOfLayers, layers.data());
	ASSERT_VULKAN(result);

	std::cout << "Number of Instance Layers: " << numOfLayers << std::endl;
	for (uint32_t i = 0; i < numOfLayers; i++) {
		std::cout << std::endl;
		std::cout << "Name:         " << layers[i].layerName << std::endl;
		std::cout << "Spec Version: " << layers[i].specVersion << std::endl;
		std::cout << "Impl Version: " << layers[i].implementationVersion << std::endl;
		std::cout << "Description:  " << layers[i].description << std::endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint32_t numOfExtensions = 0;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &numOfExtensions, nullptr);
	std::vector<VkExtensionProperties> extensions = std::vector<VkExtensionProperties>(numOfExtensions);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &numOfExtensions, extensions.data());
	ASSERT_VULKAN(result);

	std::cout << std::endl;
	std::cout << "Number of Instance Extensions: " << numOfExtensions << std::endl;
	for (uint32_t i = 0; i < numOfExtensions; i++) {
		std::cout << std::endl;
		std::cout << "Name: " << extensions[i].extensionName << std::endl;
		std::cout << "Spec Version: " << extensions[i].specVersion << std::endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	uint32_t numGlfwExtensions = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&numGlfwExtensions);

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = validationLayers.size();
	instanceInfo.ppEnabledLayerNames = validationLayers.data();
	instanceInfo.enabledExtensionCount = numGlfwExtensions;
	instanceInfo.ppEnabledExtensionNames = glfwExtensions;

	result = vkCreateInstance(&instanceInfo, nullptr, &instance);
	ASSERT_VULKAN(result);

	result = glfwCreateWindowSurface(instance, window, nullptr, &surface); 
	ASSERT_VULKAN(result); 

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint32_t numOfPhysicalDevices = 0;
	result = vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices = std::vector<VkPhysicalDevice>(numOfPhysicalDevices);
	result = vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, physicalDevices.data());
	ASSERT_VULKAN(result);

	for (uint32_t i = 0; i < numOfPhysicalDevices; i++) {
		printStats(physicalDevices[i]);
	}

	float queuePrios[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0; //TODO Choose correct family index //e.g. the one with graphics, compute, transfer and sparse queue available at once?
	deviceQueueCreateInfo.queueCount = 1; //TODO Check if this amount is valid
	deviceQueueCreateInfo.pQueuePriorities = queuePrios;

	VkPhysicalDeviceFeatures usedFeatures = {};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &usedFeatures;

	//TODO pick "best" device instead of first device
	result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
	ASSERT_VULKAN(result);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);

	VkBool32 surfaceSupport = false; 
	result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], deviceQueueCreateInfo.queueFamilyIndex, surface, &surfaceSupport);
	ASSERT_VULKAN(result); 

	if (!surfaceSupport) {
		std::cerr << "Surface not supported!" << std::endl; 
		__debugbreak(); 
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo; 
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3; //TODO check if valid
	swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM; //TODO check if valid
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; //TODO check if valid
	swapchainCreateInfo.imageExtent = VkExtent2D{ WIDTH, HEIGHT };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO check if valid
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //TODO check for best
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain); 
	ASSERT_VULKAN(result); 

	vkGetSwapchainImagesKHR(device, swapchain, &numImagesInSwapchain, nullptr); 
	VkImage *swapchainImages = new VkImage[numImagesInSwapchain];
	result = vkGetSwapchainImagesKHR(device, swapchain, &numImagesInSwapchain, swapchainImages); 
	ASSERT_VULKAN(result); 

	imageViews = new VkImageView[numImagesInSwapchain]; 
	for (int i = 0; i < numImagesInSwapchain; i++) {
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM; //TODO check if valid
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1; //VR relevant!

		result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]); 
		ASSERT_VULKAN(result); 
	}

	auto shaderCodeVert = readFile("Resources/vert.spv"); 
	auto shaderCodeFrag = readFile("Resources/frag.spv");

	createShaderModule(shaderCodeVert, &shaderModuleVert);
	createShaderModule(shaderCodeFrag, &shaderModuleFrag);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
	shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoVert.pNext = nullptr;
	shaderStageCreateInfoVert.flags = 0;
	shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfoVert.module = shaderModuleVert;
	shaderStageCreateInfoVert.pName = "main";
	shaderStageCreateInfoVert.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
	shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoFrag.pNext = nullptr;
	shaderStageCreateInfoFrag.flags = 0;
	shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfoFrag.module = shaderModuleFrag;
	shaderStageCreateInfoFrag.pName = "main";
	shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { shaderStageCreateInfoVert, shaderStageCreateInfoFrag };
	
	delete[] swapchainImages; 
}

void gameLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents(); 
	}
}

void shutdownVulkan() {
	result = vkDeviceWaitIdle(device);
	ASSERT_VULKAN(result);

	for (int i = 0; i < numImagesInSwapchain; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews; 
	vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	vkDestroyShaderModule(device, shaderModuleFrag, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr); 
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw() {
	glfwDestroyWindow(window); 
}

int main()
{
	startGlfw();
	startVulkan();
	gameLoop();
	shutdownVulkan();
	shutdownGlfw();

	return 0; 
}