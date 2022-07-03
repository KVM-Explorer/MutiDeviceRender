#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <optional>
#include <GLFW/glfw3.h>
#include <fstream>
#include <array>
#include <exception>
#include <numeric>

#undef max

class Render
{
public:
	static void init(GLFWwindow* window);
	static void quit();
	static void createPipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader);
	static vk::ShaderModule createShaderModule(const char* filename);
	static void render();
	static void waitIdle();
	
private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsIndices;
		std::optional<uint32_t> presentIndices;
 	};
	struct SwapChainRequiredInfo
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		vk::Extent2D	extent;
		vk::SurfaceFormatKHR format;
		vk::PresentModeKHR presentMode{};
		uint32_t	imageCount{};
	};
	

	struct  MemRequiredInfo
	{
		uint32_t index;
		size_t size;
	};

	static QueueFamilyIndices queueIndices_;
	static SwapChainRequiredInfo requiredInfo_;

	static vk::Instance instance_;
	static vk::SurfaceKHR surface_;
	static vk::PhysicalDevice physicalDevice_;
	static vk::Device	device_;
	static vk::Queue	graphicQueue_;
	static vk::Queue	presentQueue_;
	static vk::SwapchainKHR swapchain_;
	static std::vector<vk::Image> images_;
	static std::vector<vk::ImageView> imageViews_;
	static std::vector<vk::ShaderModule> shaderModules_;
	static vk::Pipeline	pipeline_;
	static vk::PipelineLayout layout_;
	static vk::RenderPass renderPass_;
	static std::vector<vk::Framebuffer> frameBuffer_;
	static vk::CommandPool commandPool_;
	static vk::CommandBuffer commandBuffer_;
	static vk::Semaphore imageAvaliableSemaphore_;
	static vk::Semaphore renderFinishSemaphore_;;
	static vk::Fence fence_;
	static vk::Buffer vertexBuffer_;
	static vk::DeviceMemory vertexMemory_;
	static uint32_t textureWidth;
	static uint32_t textureHeight;
	

	static vk::Instance createInstance(std::vector<const char*>& extensions);
	static vk::SurfaceKHR createSurface(GLFWwindow* window);
	static vk::PhysicalDevice pickupPhysicalDevice();
	static vk::Device createDevice();
	static vk::SwapchainKHR createSwapchain();
	static std::vector<vk::ImageView>	createImageViews();
	static vk::PipelineLayout createLayout();
	static vk::RenderPass createRenderPass();
	static std::vector<vk::Framebuffer> createFreamebuffers();
	static vk::CommandPool createCommandPool();
	static vk::CommandBuffer createCommandBuffer();
	static void recordCommand(vk::CommandBuffer buffer,vk::Framebuffer framebuffer);
	static vk::Semaphore createSemaphore();
	static vk::Fence createFence();
	static vk::Buffer createBuffer(vk::BufferUsageFlags flag);
	static vk::DeviceMemory allocateMem(vk::Buffer buffer);
	static vk::DeviceMemory allocateMem(vk::Image image);
	static vk::Image createTextureImage();

	static QueueFamilyIndices queryPhysicalDevice();
	static SwapChainRequiredInfo querySwapChainRequiredInfo(int w,int h);
	static MemRequiredInfo queryBufferMemRequiredInfo(vk::Buffer, vk::MemoryPropertyFlags flag);
	static MemRequiredInfo queryImageMemRequiredInfo(vk::Image, vk::MemoryPropertyFlags flag);

};