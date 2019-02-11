// Valkun.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// INCLUDES & DEFINES////////

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <chrono>

#include <VulkanUtils.h>
#include <EasyImage.h>
#include <DepthImage.h>
#include <Vertex.h>
#include <Mesh.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <InputHandler.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// GLOBAL VARS ////////

VkInstance	instance; 
std::vector<VkPhysicalDevice> physicalDevices;
VkSurfaceKHR surface; 
VkDevice	device;
VkSwapchainKHR swapchain = VK_NULL_HANDLE; 
VkImageView *imageViews;
VkFramebuffer *framebuffers; 
VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass; 
VkPipeline pipeline;
VkCommandPool commandPool;
VkCommandBuffer* commandBuffers; 
VkSemaphore semaphoreImageAvailable; 
VkSemaphore semaphoreRenderingDone;
VkQueue queue;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferDeviceMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferDeviceMemory;
VkBuffer uniformBuffer;
VkDeviceMemory uniformBufferMemory;
struct UniformData {
	glm::mat4 MVP;
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
	glm::vec4 lightPos = glm::vec4(1.25f, 8.35f, 0.0f, 0.0f);
} uniformData;
uint32_t numImagesInSwapchain = 0;

GLFWwindow *window;
uint32_t width = 960;
uint32_t height = 540;
const VkFormat ourFormat = VK_FORMAT_B8G8R8A8_UNORM; //check if valid

struct
{
	VkDescriptorSetLayout material;
	VkDescriptorSetLayout scene;
} descriptorSetLayouts;
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSetScene;

DepthImage depthImage;

VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
glm::vec3 eyePos = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 lookDir = glm::vec3(1.0f, 0.0f, 0.0f);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// HELPER CLASSES/STRUCTS ////////

// Shader properites for a material
// Will be passed to the shaders using push constant
struct SceneMaterialProperties
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float opacity;
};

// Stores info on the materials used in the scene
struct SceneMaterial
{
	std::string name;
	// Material properties
	SceneMaterialProperties properties;
	// The example only uses a diffuse channel
	EasyImage diffuse;
	// The material's descriptor contains the material descriptors
	VkDescriptorSet descriptorSet;
	// Pointer to the pipeline used by this material
	VkPipeline *pipeline;
};
std::vector<SceneMaterial*> materials; // = std::vector<SceneMaterial>(0);

// Stores per-mesh Vulkan resources
struct ScenePart
{
	// Index of first index in the scene buffer
	uint32_t indexBase = 0;
	uint32_t indexCount = 0;

	// Pointer to the material used by this mesh
	SceneMaterial *material = nullptr;
};
std::vector<ScenePart> meshes; // = std::vector<ScenePart>(0);

std::vector<Vertex> vertices = {
	/*Vertex({ -0.5f, -0.5f,  0.0f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }),
	Vertex({  0.5f,  0.5f,  0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }),
	Vertex({ -0.5f,  0.5f,  0.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }),
	Vertex({  0.5f, -0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }),

	Vertex({ -0.5f, -0.5f, -1.0f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }),
	Vertex({  0.5f,  0.5f, -1.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }),
	Vertex({ -0.5f,  0.5f, -1.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }),
	Vertex({  0.5f, -0.5f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f })*/
};

std::vector<uint32_t> indices = {
	/*0, 1, 2, 0, 3, 1, 
	4, 5, 6, 4, 7, 5*/
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// MISC HELPERS ////////

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

std::vector<VkPhysicalDevice> getAllPhysicalDevices() {
	uint32_t numOfPhysicalDevices = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, nullptr);
	ASSERT_VULKAN(result);
	std::vector<VkPhysicalDevice> physicalDevices = std::vector<VkPhysicalDevice>(numOfPhysicalDevices);
	result = vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, physicalDevices.data());
	ASSERT_VULKAN(result);

	return physicalDevices;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// VULKAN STARTUP HELPERS ////////

void createInstance() {
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Valkun Engine Sample";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Valkun Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

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

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
	ASSERT_VULKAN(result);
}

void printInstanceLayers() {
	uint32_t numOfLayers = 0;
	VkResult result = vkEnumerateInstanceLayerProperties(&numOfLayers, nullptr);
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
}

void printInstanceExtensions() {
	uint32_t numOfExtensions = 0;
	VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &numOfExtensions, nullptr);
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
}

void createGlfwWindowSurface() {
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	ASSERT_VULKAN(result);
}

void printPhysicalDevicesStats() {
	for (uint32_t i = 0; i < physicalDevices.size(); i++) {
		printStats(physicalDevices[i]);
	}
}

void createLogicalDevice() {
	float queuePrios[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0; //TODO Choose correct family index //e.g. the one with graphics, compute, transfer and sparse queue available at once?
	deviceQueueCreateInfo.queueCount = 1; //TODO Check if this amount is valid
	deviceQueueCreateInfo.pQueuePriorities = queuePrios;

	VkPhysicalDeviceFeatures usedFeatures = {};
	usedFeatures.samplerAnisotropy = VK_TRUE;

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
	VkResult result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
	ASSERT_VULKAN(result);
}

void initQueue() {
	vkGetDeviceQueue(device, 0, 0, &queue);
}

void checkSurfaceSupport() {
	VkBool32 surfaceSupport = false;
	VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], /*deviceQueueCreateInfo.queueFamilyIndex*/ 0, surface, &surfaceSupport);
	ASSERT_VULKAN(result);

	if (!surfaceSupport) {
		std::cerr << "Surface not supported!" << std::endl;
		__debugbreak();
	}
}

void createSwapchain() {
	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3; //TODO check if valid
	swapchainCreateInfo.imageFormat = ourFormat; //TODO check if valid
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; //TODO check if valid
	swapchainCreateInfo.imageExtent = VkExtent2D{ width, height };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO check if valid
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode; //TODO check for best
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = swapchain;

	VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
	ASSERT_VULKAN(result);
}

void createImageViews() {
	vkGetSwapchainImagesKHR(device, swapchain, &numImagesInSwapchain, nullptr);
	VkImage *swapchainImages = new VkImage[numImagesInSwapchain];
	VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &numImagesInSwapchain, swapchainImages);
	ASSERT_VULKAN(result);

	imageViews = new VkImageView[numImagesInSwapchain];
	for (int i = 0; i < numImagesInSwapchain; i++) {
		createImageView(device, swapchainImages[i], ourFormat, VK_IMAGE_ASPECT_COLOR_BIT, imageViews[i]); 
	}

	delete[] swapchainImages;
}

void createRenderPass() {
	VkAttachmentDescription presentAttachmentDesc;
	presentAttachmentDesc.flags = 0;
	presentAttachmentDesc.format = ourFormat;
	presentAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	presentAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	presentAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	presentAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	presentAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	presentAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	presentAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReference;
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachmentDesc = DepthImage::getDepthAttachment(physicalDevices[0]);

	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;

	std::vector<VkAttachmentDescription> attachments; 
	attachments.push_back(presentAttachmentDesc); 
	attachments.push_back(depthAttachmentDesc); 

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	VkResult result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
	ASSERT_VULKAN(result);
}

void createDescriptorSetLayout()
{
	// Descriptor set and pipeline layouts
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;

	// Set 0: Scene matrices
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
	descriptorSetLayoutBinding.binding = 0; //0 == MVP, see shader.vert
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	setLayoutBindings.push_back(descriptorSetLayoutBinding);

	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts.scene);
	ASSERT_VULKAN(result);
	
	// Set 1: Material data
	setLayoutBindings.clear();
	VkDescriptorSetLayoutBinding samplerDescriptorSetLayoutBinding;
	samplerDescriptorSetLayoutBinding.binding = 0; //not 1?
	samplerDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerDescriptorSetLayoutBinding.descriptorCount = 1;
	samplerDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	setLayoutBindings.push_back(samplerDescriptorSetLayoutBinding);

	result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts.material);
	ASSERT_VULKAN(result);
}

void createPipeline() {
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

	auto vertexBindingDescription = Vertex::getBindingDescription();
	auto vertexAttributeDescriptions = Vertex::getAttributeDescriptions(); 

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = nullptr;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { width, height };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = nullptr;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreateInfo.pNext = nullptr;
	rasterizationCreateInfo.flags = 0;
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationCreateInfo.frontFace = frontFace;
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationCreateInfo.depthBiasClamp = 0.0f;
	rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.pNext = nullptr;
	multisampleCreateInfo.flags = 0;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.minSampleShading = 1.0f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = DepthImage::getDepthStencilStateCreateInfoOpaque(); 

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.pNext = nullptr;
	colorBlendCreateInfo.flags = 0;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.flags = 0;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	// Setup pipeline layout
	std::array<VkDescriptorSetLayout, 2> setLayouts = { descriptorSetLayouts.scene, descriptorSetLayouts.material };
	// We will be using a push constant block to pass material properties to the fragment shaders
	VkPushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SceneMaterialProperties);
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	ASSERT_VULKAN(result);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	ASSERT_VULKAN(result);
}

void createFramebuffers() {
	framebuffers = new VkFramebuffer[numImagesInSwapchain];
	for (size_t i = 0; i < numImagesInSwapchain; i++) {
		std::vector<VkImageView> attachmentViews; 
		attachmentViews.push_back(imageViews[i]); 
		attachmentViews.push_back(depthImage.getImageView()); 

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = attachmentViews.size();
		framebufferCreateInfo.pAttachments = attachmentViews.data();
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &(framebuffers[i]));
		ASSERT_VULKAN(result);
	}
}

void createCommandPool() {
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = 0; //check if valid - get correct queue with VK_QUEUE_GRAPHICS_BIT

	VkResult result = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
	ASSERT_VULKAN(result);
}

void createDepthImage() {
	depthImage.create(device, physicalDevices[0], commandPool, queue, width, height); 
}

void createCommandBuffers() {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = numImagesInSwapchain;

	commandBuffers = new VkCommandBuffer[numImagesInSwapchain];
	VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);
	ASSERT_VULKAN(result);
}

void loadAsset_MeshAndTex(const char *meshPath, const char *texPath) {
	SceneMaterial* smat = new SceneMaterial();
	materials.push_back(smat);
	smat->name = texPath;
	smat->pipeline = &pipeline;
	smat->diffuse.load(texPath);
	smat->diffuse.upload(device, physicalDevices[0], commandPool, queue);

	size_t numVertsBefore = vertices.size();
	size_t numIndicesBefore = indices.size();

	Mesh tmpMesh;
	tmpMesh.setVertexStore(&vertices);
	tmpMesh.setIndexStore(&indices);
	tmpMesh.create(meshPath);

	size_t numVertsAfter = vertices.size();
	size_t numIndicesAfter = indices.size();

	ScenePart sp;
	sp.indexBase = numIndicesBefore;
	sp.indexCount = numIndicesAfter - numIndicesBefore;
	sp.material = smat;
	meshes.emplace_back(sp);
}

void createVertexBuffer() {
	createAndUploadBuffer(device, physicalDevices[0], queue, commandPool, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferDeviceMemory);
}

void createIndexBuffer() {
	createAndUploadBuffer(device, physicalDevices[0], queue, commandPool, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferDeviceMemory);
}

void createUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(uniformData);
	createBuffer(device, physicalDevices[0], bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBufferMemory);
}

void createDescriptorPool() {
	VkDescriptorPoolSize descriptorPoolSize;
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = static_cast<uint32_t>(materials.size());

	VkDescriptorPoolSize samplerPoolSize;
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = static_cast<uint32_t>(materials.size());

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
	descriptorPoolSizes.push_back(descriptorPoolSize);
	descriptorPoolSizes.push_back(samplerPoolSize); 

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(materials.size()) + 1;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	VkResult result = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
	ASSERT_VULKAN(result); 
}

void createDescriptorSet() {
	// Material descriptor sets
	for (size_t i = 0; i < materials.size(); i++)
	{
		// Descriptor set
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayouts.material;

		VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &materials[i]->descriptorSet);
		ASSERT_VULKAN(result);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		// todo : only use image sampler descriptor set and use one scene ubo for matrices

		// Binding 0: Diffuse texture
		VkDescriptorImageInfo descriptorImageInfo;
		descriptorImageInfo.sampler = materials[i]->diffuse.getSampler();
		descriptorImageInfo.imageView = materials[i]->diffuse.getImageView();
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet writeDescriptorSet;
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.pNext = nullptr;
		writeDescriptorSet.dstSet = materials[i]->descriptorSet;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &descriptorImageInfo;
		writeDescriptorSet.pBufferInfo = nullptr;
		writeDescriptorSet.pTexelBufferView = nullptr;

		writeDescriptorSets.push_back(writeDescriptorSet);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	// Scene descriptor set
	VkDescriptorSetAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayouts.scene;

	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSetScene);
	ASSERT_VULKAN(result);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	// Binding 0 : Vertex shader uniform buffer
	VkDescriptorBufferInfo descriptorUniformBufferInfo;
	descriptorUniformBufferInfo.buffer = uniformBuffer;
	descriptorUniformBufferInfo.offset = 0;
	descriptorUniformBufferInfo.range = sizeof(uniformData);
	VkWriteDescriptorSet descriptorWrite;
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext = nullptr;
	descriptorWrite.dstSet = descriptorSetScene;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pBufferInfo = &descriptorUniformBufferInfo;
	descriptorWrite.pTexelBufferView = nullptr;

	writeDescriptorSets.push_back(descriptorWrite);

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void recordCommandBuffers() {
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	for (size_t i = 0; i < numImagesInSwapchain; i++) {
		VkResult result = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);
		ASSERT_VULKAN(result);

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { width, height };
		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearValue depthClearValue = { 1.0f, 0.0f };
		std::vector<VkClearValue> clearValues; 
		clearValues.push_back(clearValue); 
		clearValues.push_back(depthClearValue); 
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport); 

		VkRect2D scissor; 
		scissor.offset = { 0, 0 }; 
		scissor.extent = { width, height }; 
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor); 

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto mesh : meshes) 
		{
			//if ((renderSingleScenePart) && (i != scenePartIndex))
			//	continue;

			// We will be using multiple descriptor sets for rendering
			// In GLSL the selection is done via the set and binding keywords
			// VS: layout (set = 0, binding = 0) uniform UBO;
			// FS: layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;

			std::array<VkDescriptorSet, 2> descriptorSets;
			// Set 0: Scene descriptor set containing global matrices
			descriptorSets[0] = descriptorSetScene;
			// Set 1: Per-Material descriptor set containing bound images
			descriptorSets[1] = mesh.material->descriptorSet;

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *mesh.material->pipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

			// Pass material properies via push constants
			vkCmdPushConstants(
				commandBuffers[i],
				pipelineLayout,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SceneMaterialProperties),
				&mesh.material->properties);

			vkCmdDrawIndexed(commandBuffers[i], mesh.indexCount, 1, mesh.indexBase, 0, 0);
		}
		
		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		ASSERT_VULKAN(result);
	}
}

void createSemaphores() {
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	VkResult result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreImageAvailable);
	ASSERT_VULKAN(result);
	result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreRenderingDone);
	ASSERT_VULKAN(result);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// MAIN FUNCS ////////

void recreateSwapchain() {
	//wait until physical device is done working
	vkDeviceWaitIdle(device);

	//destroy old buffers, pipeline, etc
	depthImage.destroy();
	vkFreeCommandBuffers(device, commandPool, numImagesInSwapchain, commandBuffers);
	delete[] commandBuffers;

	for (size_t i = 0; i < numImagesInSwapchain; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}
	delete[] framebuffers;

	vkDestroyRenderPass(device, renderPass, nullptr);
	for (int i = 0; i < numImagesInSwapchain; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews;

	//create a new swapchain
	VkSwapchainKHR oldSwapchain = swapchain;
	createSwapchain();
	createImageViews();
	createRenderPass();
	createDepthImage();
	createFramebuffers();
	createCommandBuffers();
	recordCommandBuffers();

	//destroy old swapchain
	vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
}

void onWindowResized(GLFWwindow *window, int newWidth, int newHeight) {
	if (newWidth <= 0 || newHeight <= 0) return; //Do nothing for invalid args

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[0], surface, &surfaceCapabilities);

	if (newWidth > surfaceCapabilities.maxImageExtent.width) newWidth = surfaceCapabilities.maxImageExtent.width;
	if (newHeight > surfaceCapabilities.maxImageExtent.height) newHeight = surfaceCapabilities.maxImageExtent.height;
	
	width = newWidth;
	height = newHeight;
	recreateSwapchain();
	std::string title = "Valkun Sample (" + std::to_string(width) + "x" + std::to_string(height) + ")";
	glfwSetWindowTitle(window, title.c_str()); 
}

void startGlfw() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	std::string title = "Valkun Sample (" + std::to_string(width) + "x" + std::to_string(height) + ")";
	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	glfwSetWindowSizeCallback(window, onWindowResized);

	InputHandler::setEyePos(&eyePos); 
	InputHandler::setLookDir(&lookDir);
	glfwSetKeyCallback(window, InputHandler::key_callback);
	glfwSetCursorPosCallback(window, InputHandler::cursor_position_callback);
}

void startVulkan() {
	createInstance(); 
	physicalDevices = getAllPhysicalDevices(); 
	printInstanceLayers(); 
	printInstanceExtensions(); 
	createGlfwWindowSurface(); 
	printPhysicalDevicesStats(); 
	createLogicalDevice(); 
	initQueue(); 
	checkSurfaceSupport(); 

	createSwapchain(); 
	createImageViews(); 
	createRenderPass(); 
	createDescriptorSetLayout();
	createPipeline();
	createCommandPool();
	createDepthImage(); 
	createFramebuffers(); 
	createCommandBuffers();

	loadAsset_MeshAndTex("Resources/Models/tiger_i.obj", "Resources/Textures/tiger_i.png");
	loadAsset_MeshAndTex("Resources/Models/lp_tree.obj", "Resources/Textures/tex.png");
	createVertexBuffer();
	createIndexBuffer(); 
	createUniformBuffer(); 
	createDescriptorPool(); 
	createDescriptorSet();
	recordCommandBuffers();
	createSemaphores(); 
}

void drawFrame() {
	uint32_t imageIndex; 
	VkResult result = vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);
	ASSERT_VULKAN(result);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	}

	VkSubmitInfo submitInfo; 
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
	VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(commandBuffers[imageIndex]);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphoreRenderingDone;

	result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE); 
	ASSERT_VULKAN(result);

	VkPresentInfoKHR presentInfo; 
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphoreRenderingDone;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(queue, &presentInfo); 
	ASSERT_VULKAN(result); 
}

auto gameStartTime = std::chrono::high_resolution_clock::now(); 
void updateMVP() {
	auto frameTime = std::chrono::high_resolution_clock::now(); 

	float timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(frameTime - gameStartTime).count() / 1000.0f;

	glm::mat4 model = 
		glm::translate(
			glm::rotate(
				glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)), 
				timeSinceStart * glm::radians(30.0f), 
				glm::vec3(0.0f, 0.0f, 1.0f)), 
			glm::vec3(0, 0, -2.0f));
	glm::mat4 view = 
		glm::lookAt(
			eyePos, 
			eyePos + lookDir, 
			glm::vec3(0.0f, 0.0f, 1.0f)
		); 
	glm::mat4 projection = 
		glm::perspective(
			glm::radians(60.0f), 
			width / (float)height, 
			0.01f, 10.0f
		);
	projection[1][1] *= -1;

	uniformData.MVP = projection * view * model;


	/*if (attachLight)
	{
		scene->uniformData.lightPos = glm::vec4(-camera.position, 1.0f);
	}*/

	uniformData.projection = glm::mat4(1.0f); // camera.matrices.perspective;
	uniformData.view = glm::mat4(1.0f); // camera.matrices.view;
	uniformData.model = glm::mat4(1.0f);

	void* data;
	vkMapMemory(device, uniformBufferMemory, 0, sizeof(uniformData), 0, &data); 
	memcpy(data, &uniformData, sizeof(uniformData));
	vkUnmapMemory(device, uniformBufferMemory); 
}

void gameLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents(); 

		updateMVP(); 

		drawFrame(); 
	}
}

void shutdownVulkan() {
	vkDeviceWaitIdle(device);

	depthImage.destroy(); 

	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.material, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.scene, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkFreeMemory(device, uniformBufferMemory, nullptr);
	vkDestroyBuffer(device, uniformBuffer, nullptr); 

	vkFreeMemory(device, indexBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);

	vkFreeMemory(device, vertexBufferDeviceMemory, nullptr); 
	vkDestroyBuffer(device, vertexBuffer, nullptr); 

	for (size_t i = 0; i < materials.size(); i++) {
		delete materials[i]; 
	}
	materials.clear(); 
	meshes.clear(); 

	vkDestroySemaphore(device, semaphoreImageAvailable, nullptr);
	vkDestroySemaphore(device, semaphoreRenderingDone, nullptr); 

	vkFreeCommandBuffers(device, commandPool, numImagesInSwapchain, commandBuffers); 
	delete[] commandBuffers;

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (size_t i = 0; i < numImagesInSwapchain; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}
	delete[] framebuffers; 

	vkDestroyPipeline(device, pipeline, nullptr); 
	vkDestroyRenderPass(device, renderPass, nullptr); 
	for (int i = 0; i < numImagesInSwapchain; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews; 
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	vkDestroyShaderModule(device, shaderModuleFrag, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr); 
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw() {
	glfwDestroyWindow(window); 
	glfwTerminate(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// MAIN ////////

int main()
{
	startGlfw();
	startVulkan();
	gameLoop();
	shutdownVulkan();
	shutdownGlfw();

	return 0; 
}