#pragma once

# include "types/App.hpp"

# include <iostream>
# include <stdexcept>
# include <cstdlib>
# include <vector>
# include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	bool isComplete();
};

class	App {
public:
	void	run();

private:
	GLFWwindow	*window;
	VkInstance	instance;

	void	initWindow();
	void	initVulkan();
	void	createInstance();
	void	mainLoop();
	void	cleanup();

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerEXT	debugMessenger;
	#ifdef NDEBUG
	 const bool enableValidationLayers = false;
	 #define wout if (false) std::cerr
	#else
	 const bool enableValidationLayers = true;
	 #define wout std::cerr
	#endif

	bool	checkValidationLayerSupport();
	std::vector<const char*>	getRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void	setupDebugMessenger();
	VkResult	CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	void	DestroyDebugUtilsMessengerEXT(
		VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
	void	populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	void	pickPhysicalDevice();
	bool	isDeviceSuitable(VkPhysicalDevice device);
	int		rateDeviceSuitability(VkPhysicalDevice device);

	QueueFamilyIndices	findQueueFamilies(VkPhysicalDevice device);
	bool	hasQueueFamilies(VkPhysicalDevice device);

	VkDevice device;

	void	createLogicalDevice();

	VkQueue graphicsQueue;

	VkSurfaceKHR surface;

	void	createSurface();

	VkQueue presentQueue;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

};
