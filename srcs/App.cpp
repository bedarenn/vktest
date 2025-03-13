#include "App.hpp"

#include <map>

#include <cstring>
#include <iomanip>
#include <curses.h>
#include <set>

void	App::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void	App::initWindow() {
	if (!glfwInit())
		throw (std::runtime_error("glfwInit"));
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, NAME, nullptr, nullptr);
	if (!window)
		throw (std::runtime_error("glfwCreateWindow"));
}

void	App::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
}

void	App::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Init AppInfo
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = NAME;
	appInfo.applicationVersion = W_VK_ARG_VERSION(VK_MAKE_VERSION);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = W_VK_ARG_VERSION(VK_MAKE_VERSION);
	appInfo.apiVersion = W_VK_API_VERSION;

	// Init InstanceCreateInfo
	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	createInfo.enabledExtensionCount = 0;
	createInfo.ppEnabledExtensionNames = nullptr;

	// InstanceCreateInfo flags
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	// InstanceCreateInfo Layers
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}

	// InstanceCreateInfo extensions
	std::vector<const char*> requiredExtensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");

	// Extension support
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Info print
	wout << "available extensions:\n";
	uint32_t	i;
	for (const VkExtensionProperties& extension : extensions) {
		for (i = 0; i < createInfo.enabledExtensionCount && strcmp(createInfo.ppEnabledExtensionNames[i], extension.extensionName); i++) ;
		wout << (i != createInfo.enabledExtensionCount ? "✅​" : "❌​") << '\t' << extension.extensionName << '\n';
	}
}

static void	keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		 glfwSetWindowShouldClose(window, GLFW_TRUE);
	(void)scancode;
	(void)mods;
}

void	App::mainLoop() {
	glfwSetKeyCallback(window, keyCallback);
	while (!glfwWindowShouldClose(window))
		glfwPollEvents();
}

void	App::cleanup() {
	if (device)
	vkDestroyDevice(device, nullptr);
	if (enableValidationLayers)
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool	App::checkValidationLayerSupport() {
	uint32_t	layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties>	availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		size_t	i;
		for (i = 0; i < layerCount && strcmp(layerName, availableLayers[i].layerName); i++) ;
		if (i == layerCount)
			return (false);
	}
	return (true);
}

std::vector<const char*>	App::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	return (extensions);
}

/*
VkDebugUtilsMessageSeverityFlagBitsEXT:
	VERBOSE:	Diagnostic message
	INFO:		Informational message like the creation of a resource
	WARNING:	Message about behavior that is not necessarily an error, but very likely a bug in your application
	ERROR:		Message about behavior that is invalid and may cause crashes
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT
VkDebugUtilsMessageTypeFlagBitsEXT:
	GENERAL:		Some event has happened that is unrelated to the specification or performance
	VALIDATION:		Something has happened that violates the specification or indicates a possible mistake
	PERFORMANCE:	Potential non-optimal use of Vulkan
	VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
	VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT
*/
VKAPI_ATTR VkBool32 VKAPI_CALL	App::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	switch (messageSeverity) {
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT):
		wout << COLOR_VERBOSE; break;
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT):
		wout << COLOR_INFO; break;
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT):
		wout << COLOR_WARNING; break;
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT):
		wout << COLOR_ERROR; break;
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT):
		wout << COLOR_STD; break;
	}
	(void)messageType;
	wout << pCallbackData->pMessage << std::endl;
	wout << COLOR_STD;
	(void)pUserData;
	return (VK_FALSE);
}

void	App::setupDebugMessenger() {
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to set up debug messenger!");
}

VkResult	App::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void	App::DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

void	App::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = this;
}

void	App::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0)
		physicalDevice = candidates.rbegin()->second;
	else
		throw std::runtime_error("failed to find a suitable GPU!");
}

bool	App::isDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// bool	suit = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool	suit = indices.isComplete() && extensionsSupported;
	wout << (suit ? "✅​" : "❌​") << deviceProperties.deviceName << std::endl;

	return (suit);
}

bool App::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

int	App::rateDeviceSuitability(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int	l_2D = deviceProperties.limits.maxImageDimension2D;
	int	l_3D = deviceProperties.limits.maxImageDimension3D;

	bool	is_gpu = (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
					|| deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
					|| deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);

	// Application can't function without geometry shaders
	bool	have_shdrs = deviceFeatures.geometryShader;

	int	score = (l_3D + (is_gpu * 1000)) * have_shdrs;

	wout << deviceProperties.deviceName << std::endl;
	wout << (is_gpu ? "GPU" : "CPU");
	wout << " 2D:" << l_2D << " 3D:" << l_3D;
	wout << "\t| " << (have_shdrs ? "✅​" : "❌​") << " geometryShader";
	wout << "\t| " << score << std::endl;

	return (score);
}

/*
	VK_QUEUE_GRAPHICS_BIT = 0x00000001,
	VK_QUEUE_COMPUTE_BIT = 0x00000002,
	VK_QUEUE_TRANSFER_BIT = 0x00000004,
	VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
	VK_QUEUE_PROTECTED_BIT = 0x00000010,
	VK_QUEUE_VIDEO_DECODE_BIT_KHR = 0x00000020,
	VK_QUEUE_VIDEO_ENCODE_BIT_KHR = 0x00000040,
	VK_QUEUE_OPTICAL_FLOW_BIT_NV = 0x00000100,
*/
QueueFamilyIndices	App::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		wout << i << ": " << std::hex << queueFamily.queueFlags << std::dec << "\t" << COLOR_WR_FLAG;
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			wout << COLOR_R_FLAG << "VK_QUEUE_GRAPHICS_BIT " << COLOR_WR_FLAG;
		}
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			wout << "VK_QUEUE_COMPUTE_BIT ";
		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			wout << "VK_QUEUE_TRANSFER_BIT ";
		if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			wout << "VK_QUEUE_SPARSE_BINDING_BIT ";
		if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT)
			wout << "VK_QUEUE_PROTECTED_BIT ";
		if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			wout << "VK_QUEUE_VIDEO_DECODE_BIT_KHR ";
		if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			wout << "VK_QUEUE_VIDEO_ENCODE_BIT_KHR ";
		if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
			wout << "VK_QUEUE_OPTICAL_FLOW_BIT_NV ";
		i++;
		wout << COLOR_STD << std::endl;
	}

	i = 0;
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
	if (presentSupport) {
		indices.presentFamily = i;
		wout << i << ": presentSupport" << std::endl;
	}

	return (indices);
}

bool	App::hasQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	return (indices.isComplete());
}

bool	QueueFamilyIndices::isComplete() {
	return (graphicsFamily.has_value() && presentFamily.has_value());
}

void	App::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	device = nullptr;
	if (vkCreateDevice(physicalDevice,&createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void	App::createSurface() {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
}
