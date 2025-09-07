#pragma once


#include "../window/eno_window.hpp"


// stds
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

namespace Graphics
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() &&
				   presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		// Min/max number of images in swap chain
		// Min/max width and height of images
		VkSurfaceCapabilitiesKHR capabilities;

		// Surface formats (pixel format, color space)
		std::vector<VkSurfaceFormatKHR> formats;

		// Available presentation modes
		std::vector<VkPresentModeKHR> presentModes;
	};

	class EnoDevice
	{
	public:
#ifndef NDEBUG
		const bool enableValidationLayers = true;
#else
		const bool enableValidationLayer = false;
#endif
		
		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t currentFrame = 0;

		EnoDevice(Window::EnoWindow& window);
		~EnoDevice();

		void cleanup();

		// Accessors
		VkDevice getDevice() const;

		// Drawing
		void drawFrame();

		// Instance
		void createInstance();

		// Debugging
		void setupDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);

		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance,
			const VkDebugUtilsMessengerCreateInfoEXT*,
			const VkAllocationCallbacks*,
			VkDebugUtilsMessengerEXT*);

		void DestroyDebugUtilsMessengerEXT(
			VkInstance,
			VkDebugUtilsMessengerEXT,
			const VkAllocationCallbacks*);

		// Extension Checking
		std::vector<const char*> getRequiredExtensions();

		// Validation Layer Checking
		bool checkValidationLayerSupport();

		// Physical device
		void selectPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice);
		bool checkDeviceExtensionSupport(VkPhysicalDevice);

		// Logical Device
		void createLogicalDevice();

		// Surface
		void createSurface();

		// Queue Families
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

		// Swap Chain
		void createSwapchain();

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);

		// Image Views
		void createImageViews();

		// Render Pass
		void createRenderPass();

		// Graphics Pipeline
		void createGraphicsPipeline();
		VkShaderModule createShaderModule(const std::vector<char>&);

		static std::vector<char> readFile(const std::string&);

		// Framebuffer
		void createFrameBuffers();

		// Command Pool
		void createCommandPool();

		// Command Buffer
		void createCommandBuffers();
		void recordCommandBuffer(VkCommandBuffer, uint32_t);

		// Sync Objects
		void createSyncObjects();

	private:

		// Window
		Window::EnoWindow& window_;

		// Instance
		VkInstance instance_;

		// Debugging
		VkDebugUtilsMessengerEXT debugMessenger_;

		// Physical Device
		VkPhysicalDevice physicalDevice_{ VK_NULL_HANDLE };

		// Logical device
		VkDevice device_;

		// Queues
		VkQueue graphicsQueue_;
		VkQueue presentQueue_;

		// Surface
		VkSurfaceKHR surface_;

		// SwapChain
		VkSwapchainKHR swapChain_;
		std::vector<VkImage> swapChainImages_;
		VkFormat swapChainImageFormat_;
		VkExtent2D swapChainExtent_;

		// Image Views
		std::vector<VkImageView> swapChainImageViews_;

		// Render Pass
		VkRenderPass renderPass_;

		// Graphics Pipeline
		VkPipeline graphicsPipeline_;
		VkPipelineLayout pipelineLayout_;

		// Framebuffers
		std::vector<VkFramebuffer> swapChainFramebuffers_;

		// Command Pool
		VkCommandPool commandPool_;

		// Command Buffer
		std::vector<VkCommandBuffer> commandBuffers_;

		// Semaphores && Fences
		std::vector<VkSemaphore> imageAvailableSemaphores_;
		std::vector<VkSemaphore> renderFinishedSemaphores_;

		std::vector<VkFence> inFlightFences_;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}