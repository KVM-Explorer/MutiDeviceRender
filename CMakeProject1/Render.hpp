#pragma once
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

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
	static void createCommonPipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader);
	static void createComputerPipeline(vk::ShaderModule computerShader);
	static vk::ShaderModule createShaderModule(const char* filename);
	static void render();
	static void waitIdle();
	
private:
	static const uint32_t textureWidth = 256;
	static const uint32_t textureHeight= 256;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsIndices;
		std::optional<uint32_t> presentIndices;
		std::optional<uint32_t> computerIndices;
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

	struct Texture {
		vk::Image image;
		vk::DeviceMemory memory;
		vk::ImageLayout layout;
		vk::Sampler sampler;
		vk::ImageView imageView;
		vk::DescriptorImageInfo descriptor;
	};

	struct Computer
	{
		std::vector<vk::DescriptorSet> descriptorSet;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::DescriptorPool descriptorPool;
		vk::PipelineLayout pipelineLayout;
		vk::CommandPool commandPool;
		vk::CommandBuffer commandBuffer;
		vk::Fence fence;
	};

	struct Graphics
	{
		vk::DescriptorSetLayout descriptorSetLayout;
		std::vector<vk::DescriptorSet> descriptorSet;
		vk::DescriptorPool descriptorPool;
		vk::PipelineLayout pipelineLayout;
		vk::Fence fence;
		vk::CommandBuffer commandBuffer;
		vk::CommandPool commandPool;
		void release(vk::Device device)
		{
			/*device.freeCommandBuffers(commandPool, commandBuffer);
			device.destroyCommandPool(commandPool);*/
			device.destroyFence(fence);
			for(auto & descriptor_set:descriptorSet)
			{
				device.freeDescriptorSets(descriptorPool, descriptor_set);
			}
			device.destroyDescriptorPool(descriptorPool);
			device.destroyDescriptorSetLayout(descriptorSetLayout);
		}
	};

	static QueueFamilyIndices queueIndices_;
	static SwapChainRequiredInfo requiredInfo_;

	static vk::Instance instance_;
	static vk::SurfaceKHR surface_;
	static vk::PhysicalDevice physicalDevice_;
	static vk::Device	device_;
	static vk::Queue	graphicQueue_;
	static vk::Queue	presentQueue_;
	static vk::Queue	computeQueue_;
	static vk::SwapchainKHR swapchain_;
	static std::vector<vk::Image> images_;
	static std::vector<vk::ImageView> imageViews_;
	static std::vector<vk::ShaderModule> shaderModules_;
	static vk::Pipeline	pipeline_;
	
	static vk::RenderPass renderPass_;
	static std::vector<vk::Framebuffer> frameBuffer_;
	static vk::CommandPool commandPool_;
	static vk::CommandBuffer commandBuffer_;
	static vk::Semaphore imageAvaliableSemaphore_;
	static vk::Semaphore renderFinishSemaphore_;;
	static vk::Buffer vertexBuffer_;
	static vk::DeviceMemory vertexMemory_;
	static Texture texture_;
	static vk::DescriptorSetLayout descriptorSetLayout_;
	static vk::PipelineLayout pipelineLayout_;
	static vk::DescriptorPool descriptorPool_;
	static std::vector<vk::DescriptorSet> descriptorSet_;
	static vk::Pipeline computerPipeline_;
	static Computer computer_;
	static Graphics graphics_;

	static vk::Instance createInstance(std::vector<const char*>& extensions);
	static vk::SurfaceKHR createSurface(GLFWwindow* window);
	static vk::PhysicalDevice pickupPhysicalDevice();
	static vk::Device createDevice();
	static vk::SwapchainKHR createSwapchain();
	static std::vector<vk::ImageView>	createImageViews();
	static vk::PipelineLayout createCommonPipelineLayout();
	static vk::RenderPass createRenderPass();
	static std::vector<vk::Framebuffer> createFreamebuffers();
	static vk::CommandPool createCommandPool(vk::CommandPoolCreateFlagBits flags, uint32_t index);
	static vk::CommandBuffer createCommandBuffer(vk::CommandPool command_pool);
	static void recordCommand(vk::CommandBuffer buffer,vk::Framebuffer framebuffer);
	static vk::Semaphore createSemaphore();
	static vk::Fence createFence();
	static vk::Buffer createVertexBufferDefine(vk::BufferUsageFlags flag);
	static vk::Image createImageDefine(vk::ImageUsageFlags flag);
	static vk::DeviceMemory allocateMem(vk::Buffer buffer);
	static vk::DeviceMemory allocateMem(vk::Image image);
	static void createBuffer();

	static void createTexture();
	static vk::Sampler createTextureSampler();
	static vk::ImageView createTextureViewImage(vk::Image);
	static void  setImageLayout(vk::CommandBuffer cmd_buffer,vk::Image image,
		vk::ImageAspectFlags flags, vk::ImageLayout old_layout,vk::ImageLayout new_layout);
	static vk::ImageMemoryBarrier createImageMemoryBarrier();
	static void flushCommandBuffer(vk::CommandBuffer cmd_buffer);

	static vk::DescriptorSetLayoutBinding setLayoutBinding(vk::DescriptorType type, vk::ShaderStageFlagBits flags,
		uint32_t binding, uint32_t descriptorCount = 1);

	// CommonPipeline Resource
	static void createCommonDescriptor();
	static vk::DescriptorSetLayout createCommonDescriptorSetLayout();
	static vk::DescriptorPool createCommonDescriptorPool();
	static std::vector<vk::DescriptorSet> createCommonDescriptorSet();
	// ComputerPipeline Resource
	static vk::DescriptorSetLayout createComputerDescriptorSetLayout();
	static vk::PipelineLayout createComputerPipelineLayout();
	static vk::DescriptorPool createComputerDescriptorPool();
	static std::vector<vk::DescriptorSet> createComputerDescriptorSet();
	static vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet descriptor_set, vk::DescriptorType type,
		uint32_t binding, vk::DescriptorImageInfo image_info);
	static void recordRayTraceCommand();

	static QueueFamilyIndices queryPhysicalDevice();
	static SwapChainRequiredInfo querySwapChainRequiredInfo(int w,int h);
	static MemRequiredInfo queryBufferMemRequiredInfo(vk::Buffer, vk::MemoryPropertyFlags flag);
	static MemRequiredInfo queryImageMemRequiredInfo(vk::Image, vk::MemoryPropertyFlags flag);

};