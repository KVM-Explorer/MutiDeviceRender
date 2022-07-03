#pragma once
#include<vulkan/vulkan.hpp>
#include <string>
#include <stb_image.h>

class VkTexture {
public:
	struct Texture
	{
		vk::Image image;
		vk::ImageLayout layout;
		vk::CommandBuffer cmdBuffer;
		vk::ImageView imageView;
		vk::DeviceMemory deviceMem;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layCount;
		vk::DescriptorImageInfo descriptor;
		vk::Sampler sampler;
	};
	static Texture createTexture(vk::Device, std::string path);
	static Texture createTexture(vk::Device, vk::PhysicalDevice,uint32_t width, uint32_t height, vk::Format);

private:
	struct  MemRequiredInfo
	{
		uint32_t index;
		size_t size;
	};
	static Texture texture_;
public :
	
	static void clear();
	
	static vk::Buffer createStagingBuffer(vk::Device, vk::BufferUsageFlags flags);
	static vk::Image createImage(vk::Device device,vk::ImageUsageFlags flags);
	static vk::DeviceMemory allocateMemory(vk::Device device,vk::PhysicalDevice, vk::Image image);
	static MemRequiredInfo queryImageMemRequiredInfo(vk::Device,vk::PhysicalDevice,vk::Image image, vk::MemoryPropertyFlags flag);
	static vk::CommandBuffer createCmdBuffer(vk::Device);
	static vk::ImageLayout createImageLayout(vk::Device);

};