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
	vk::SwapchainKHR swapchain_;

private:
	vk::Instance createInstance();
	vk::SurfaceKHR createSurface(GLFWwindow* window);
	void createPhysicalDevice();
	vk::Device createDevice(RAII::Device &device);
	void createQueue();
	vk::SwapchainKHR createSwapchain();

	RAII::QueueFamilyIndices queryPhysicalDevice(vk::PhysicalDevice physical_device);
	RAII::SwapChainRequiredInfo querySwapChainRequiredInfo(uint32_t w, uint32_t h);
};