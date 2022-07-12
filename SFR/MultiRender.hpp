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
	
	

private:
	vk::Instance createInstance();
	vk::SurfaceKHR createSurface(GLFWwindow* window);
	void createPhysicalDevice();
	vk::Device createDevice(RAII::Device &device);
	void createQueue();
	vk::SwapchainKHR createSwapchain(vk::Device device, RAII::QueueFamilyIndices);
	std::vector<vk::ImageView> createSwapchainImageViews(vk::Device device, std::vector<vk::Image>& images);

	vk::RenderPass createRenderPass(vk::Device, vk::AttachmentLoadOp load_op, vk::AttachmentStoreOp store_op, vk::ImageLayout init_layout, vk::ImageLayout
	                                final_layout);
	std::vector<vk::Framebuffer> createFrameBuffers(vk::Device, vk::RenderPass render_pass, std::vector<vk::ImageView> image_views);
	vk::Framebuffer createFrameBuffer(vk::Device device, vk::RenderPass render_pass, vk::ImageView view);
	void initiGPUResource();
	void initdGPUResource();

	// Pipeline
	vk::CommandPool createCommandPool(vk::Device device,vk::CommandPoolCreateFlagBits flags, uint32_t index);
	vk::CommandBuffer createCommandBuffer(vk::Device device,vk::CommandPool command_pool);
	void endSingleCommand(RAII::Device& device, vk::CommandBuffer& command, vk::CommandPool pool, vk::Queue& queue);
	vk::CommandBuffer startSingleCommand(RAII::Device& device, vk::CommandPool command_pool);
	vk::Semaphore createSemaphore(vk::Device device);
	vk::Fence createFence(vk::Device device);
	
	vk::PipelineLayout createPipelineLayout(vk::Device device/*, vk::DescriptorSetLayout set_layout*/);
	void createRenderDescriptor(RAII::Device device);

	// Descriptor Buffer
	void createVertexBuffer(RAII::Device &device);
	vk::DeviceMemory allocateMemory(RAII::Device device, vk::Buffer buffer);
	vk::Buffer       createBuffer(RAII::Device device, vk::BufferUsageFlags flags);

	// Descriptor Image
	vk::Image createImage(vk::Device device, vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags flags,  vk::ImageTiling tiling);
	vk::DeviceMemory allocateImageMemory(RAII::Device& device, vk::MemoryPropertyFlags flags, vk::Image image);
	vk::ImageView createImageView(vk::Device device, vk::Image image);
	vk::Sampler createSampler(vk::Device device);
	vk::DescriptorSetLayoutBinding setLayoutBinding(vk::DescriptorType type, vk::ShaderStageFlagBits flags,uint32_t binding, uint32_t descriptorCount = 1);
	vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet descriptor_set, vk::DescriptorType type,uint32_t binding, vk::DescriptorImageInfo image_info);
	vk::DescriptorSetLayout createDescriptorSetLayout(vk::Device device);
	vk::DescriptorPool createDescriptorPool(vk::Device device);
	std::vector<vk::DescriptorSet> createDescriptorSet(vk::Device device, vk::DescriptorPool descriptor_pool, vk::DescriptorSetLayout set_layout, vk::DescriptorImageInfo descriptor);
	void convertImageLayout(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
	void savePPMImage(const char* data,vk::Extent2D extent, vk::SubresourceLayout layout);
	// Query
	RAII::QueueFamilyIndices queryPhysicalDeviceQueue(vk::PhysicalDevice physical_device);
	RAII::SwapChainRequiredInfo querySwapChainRequiredInfo(uint32_t w, uint32_t h);
	RAII::MemRequiredInfo queryBufferMemRequiredInfo(RAII::Device device, vk::Buffer buffer, vk::MemoryPropertyFlags flags);
	RAII::MemRequiredInfo queryImageMemRequiredInfo(RAII::Device device, vk::Image image, vk::MemoryPropertyFlags flags);
	void querySupportBlit(RAII::Device& device);

	//transfer Image between GPU
	void copyPresentImage(RAII::Device &src, RAII::Device& dst, uint32_t src_index, uint32_t dst_index);
	void copyOffscreenToMapping(RAII::Device& src, uint32_t src_index);
	void copyMappingToMapping(RAII::Device& src, RAII::Device& device);
	void copyMappingToPresent(RAII::Device& src, uint32_t src_index);
	vk::ImageMemoryBarrier insertImageMemoryBarrier(vk::CommandBuffer command_buffer,
	                                                vk::Image image,
	                                                vk::AccessFlags src_access,
	                                                vk::AccessFlags dst_access,
	                                                vk::ImageLayout old_layout,
	                                                vk::ImageLayout new_layout,
	                                                vk::PipelineStageFlags src_mask, vk::PipelineStageFlags dst_mask);

	
	// render
	uint32_t commonPrepare();
	void recordPresentCommand(RAII::Device device, vk::CommandBuffer buffer, vk::Framebuffer frame);
	void recordOffScreenCommand(RAII::Device device, vk::CommandBuffer buffer, vk::Framebuffer frame);
	void prepareTexture();
	void renderBydGPU();
	void renderByiGPU(uint32_t igpu_index);
	void presentImage(uint32_t igpu_index);
	void updatePresentImage(uint32_t igpu_index);
};