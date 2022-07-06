#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include "RAII.hpp"

class MultiRender
{
public:
	void init(GLFWwindow* window);
	void release();
	void render();
	void waitIdle();

	vk::ShaderModule createShaderModule(const char* filename);
	void createComputerPipeline(vk::ShaderModule computerShader);
	// create iGPU dGPU pipeline
	void createCommonPipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader);

private:
	vk::Instance instance_;
	vk::SurfaceKHR surface_;
	RAII::Device iGPU_;
	RAII::Device dGPU_;
	RAII::SwapChainRequiredInfo swapchainRequiredInfo_;
	RAII::SwapChain swapchain_;
	RAII::Descriptor vertex_;
	

private:
	vk::Instance createInstance();
	vk::SurfaceKHR createSurface(GLFWwindow* window);
	void createPhysicalDevice();
	vk::Device createDevice(RAII::Device &device);
	void createQueue();
	vk::SwapchainKHR createSwapchain();
	std::vector<vk::ImageView> createSwapchainImageViews();
	vk::RenderPass createRenderPass(vk::Device);
	std::vector<vk::Framebuffer> createFrameBuffers(vk::Device, vk::RenderPass render_pass);

	// Pipeline
	vk::CommandPool createCommandPool(vk::Device device,vk::CommandPoolCreateFlagBits flags, uint32_t index);
	vk::CommandBuffer createCommandBuffer(vk::Device device,vk::CommandPool command_pool);
	vk::Semaphore createSemaphore(vk::Device device);
	vk::Fence createFence(vk::Device device);

	// Descriptor
	void createVertexBuffer(RAII::Device device);
	vk::DeviceMemory allocateMemory(RAII::Device device, vk::Buffer buffer);
	vk::Buffer       createBuffer(RAII::Device device, vk::BufferUsageFlags flags);


	RAII::QueueFamilyIndices queryPhysicalDeviceQueue(vk::PhysicalDevice physical_device);
	RAII::SwapChainRequiredInfo querySwapChainRequiredInfo(uint32_t w, uint32_t h);
	RAII::MemRequiredInfo queryBufferMemRequiredInfo(RAII::Device device, vk::Buffer buffer, vk::MemoryPropertyFlags flags);
};