#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>
#include <glm/glm.hpp>
namespace RAII
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsIndices;
		std::optional<uint32_t> presentIndices;
		std::optional<uint32_t> computerIndices;
	};
	struct Descriptor
	{
		vk::Image image;
		vk::Buffer buffer;
		vk::DeviceMemory memory;

		vk::DescriptorSetLayout descriptorSetLayout;
		std::vector<vk::DescriptorSet> descriptorSet;
		vk::DescriptorPool descriptorPool;
	};
	struct Pipeline
	{
		std::vector<vk::ShaderModule> shaders;
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::CommandPool commandPool;
		vk::CommandBuffer commandBuffer;
		vk::Fence fence;
		vk::Semaphore imageAvaliableSemaphore;
		vk::Semaphore presentAvaliableSemaphore;
	};
	struct SwapChain
	{
		vk::SwapchainKHR swapchain;
		std::vector<vk::Image>	images;
		std::vector<vk::ImageView> imageViews;
	};
	struct Device
	{
		vk::Device device;
		vk::PhysicalDevice physicalDevice;
		QueueFamilyIndices queueIndices;
		vk::Queue graphicsQueue;
		vk::Queue computerQueue;
		vk::Queue presentQueue;
		vk::RenderPass renderPass;
		std::vector<vk::Framebuffer> frameBuffer;
		Pipeline computerPipeline;
		Pipeline graphicPipeline;
		SwapChain swapchain;
		Descriptor vertex;
		int index;
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
	//struct Graphics
	//{
	//	vk::DescriptorSetLayout descriptorSetLayout;
	//	std::vector<vk::DescriptorSet> descriptorSet;
	//	vk::DescriptorPool descriptorPool;
	//	vk::PipelineLayout pipelineLayout;
	//	vk::Fence fence;
	//	vk::CommandBuffer commandBuffer;
	//	vk::CommandPool commandPool;
	//	void release(vk::Device device)
	//	{
	//		/*device.freeCommandBuffers(commandPool, commandBuffer);
	//		device.destroyCommandPool(commandPool);*/
	//		device.destroyFence(fence);
	//		for (auto& descriptor_set : descriptorSet)
	//		{
	//			device.freeDescriptorSets(descriptorPool, descriptor_set);
	//		}
	//		device.destroyDescriptorPool(descriptorPool);
	//		device.destroyDescriptorSetLayout(descriptorSetLayout);
	//	}
	//};

	
}
