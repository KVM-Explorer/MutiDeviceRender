#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include "RAII.hpp"
#include <fstream>
#include <istream>

class MultiRender
{
public:
	void init(GLFWwindow* window);
	void release();
	void render();
	void waitIdle();

	vk::ShaderModule createShaderModule(const char* filename, int device_index);
	void createComputerPipeline(vk::ShaderModule computerShader);
	// create iGPU dGPU pipeline
	void createCommonPipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader, int device_index);

private:
	vk::Instance instance_;
	vk::SurfaceKHR surface_;
	RAII::Device iGPU_;
	RAII::Device dGPU_;
	RAII::SwapChainRequiredInfo swapchainRequiredInfo_;
	int acquiredNext{-1 };
	
	

private:
	vk::Instance createInstance();
	vk::SurfaceKHR createSurface(GLFWwindow* window);
	void createPhysicalDevice();
	vk::Device createDevice(RAII::Device &device);
	void createQueue();
	vk::SwapchainKHR createSwapchain(vk::Device device, RAII::QueueFamilyIndices);
	std::vector<vk::ImageView> createSwapchainImageViews(vk::Device device, RAII::SwapChain swapchain);
	vk::RenderPass createRenderPass(vk::Device, vk::AttachmentLoadOp load_op, vk::AttachmentStoreOp store_op, vk::ImageLayout init_layout, vk::ImageLayout
	                                final_layout);
	std::vector<vk::Framebuffer> createFrameBuffers(vk::Device, vk::RenderPass render_pass, RAII::SwapChain swapchain);


	// Pipeline
	vk::CommandPool createCommandPool(vk::Device device,vk::CommandPoolCreateFlagBits flags, uint32_t index);
	vk::CommandBuffer createCommandBuffer(vk::Device device,vk::CommandPool command_pool);
	void endSingleCommand(RAII::Device& device, vk::CommandBuffer& command, vk::CommandPool pool, vk::Queue& queue);
	vk::CommandBuffer startSingleCommand(RAII::Device& device, vk::CommandPool command_pool);
	vk::Semaphore createSemaphore(vk::Device device);
	vk::Fence createFence(vk::Device device);
	void recordCommand(RAII::Device device,vk::CommandBuffer buffer, vk::Framebuffer frame);
	vk::PipelineLayout createPipelineLayout(RAII::Device device);
	void createRenderDescriptor(RAII::Device device);

	// Descriptor
	void createVertexBuffer(RAII::Device &device);
	vk::DeviceMemory allocateMemory(RAII::Device device, vk::Buffer buffer);
	vk::Buffer       createBuffer(RAII::Device device, vk::BufferUsageFlags flags);

	// Image
	vk::Image createImage(vk::Device device, vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags flags);
	vk::DeviceMemory allocateImageMemory(RAII::Device& device, vk::MemoryPropertyFlags flags, vk::Image image);;
	// Query
	RAII::QueueFamilyIndices queryPhysicalDeviceQueue(vk::PhysicalDevice physical_device);
	RAII::SwapChainRequiredInfo querySwapChainRequiredInfo(uint32_t w, uint32_t h);
	RAII::MemRequiredInfo queryBufferMemRequiredInfo(RAII::Device device, vk::Buffer buffer, vk::MemoryPropertyFlags flags);
	RAII::MemRequiredInfo queryImageMemRequiredInfo(RAII::Device device, vk::Image image, vk::MemoryPropertyFlags flags);
	void querySupportBlit(RAII::Device& device);

	//transfer Image between GPU
	void copyPresentImage(RAII::Device &src, RAII::Device& dst, uint32_t src_index, uint32_t dst_index);
	vk::ImageMemoryBarrier insertImageMemoryBarrier(RAII::Device device,
	                                                vk::CommandBuffer command_buffer,
	                                                vk::Image image,
	                                                vk::AccessFlags src_access,
	                                                vk::AccessFlags dst_access,
	                                                vk::ImageLayout old_layout,
	                                                vk::ImageLayout new_layout,
	                                                vk::PipelineStageFlags src_mask, vk::PipelineStageFlags dst_mask);
	
	// render
	std::tuple<uint32_t, uint32_t> commonPrepare();
	void prepareTexture();
	void renderBydGPU(uint32_t igpu_index, uint32_t dgpu_index);
	void renderByiGPU(uint32_t igpu_index, uint32_t dgpu_index);
	void presentImage(uint32_t igpu_index);
};