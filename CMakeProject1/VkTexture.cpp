#include "VkTexture.hpp"

void VkTexture::clear()
{ 
	Texture texture;
	texture_ = texture;
}

VkTexture::Texture VkTexture::createTexture(vk::Device device, std::string path)
{
	// TODO Load Image Texture from file
	//int width, height, channels;
	//stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	//vk::DeviceSize image_size = static_cast<vk::DeviceSize>(width) * height * 4;

	//if (!pixels) throw std::runtime_error("failed to load texture");

	//vk::Buffer staging_buffer;
	//vk::DeviceMemory staging_mem;

	//vk::BufferCreateInfo staging_info;
	//staging_info.setSize(image_size)
	//		.set;


	//stbi_image_free(pixels);
}

VkTexture::Texture VkTexture::createTexture(vk::Device device, vk::PhysicalDevice physical_device,uint32_t width, uint32_t height, vk::Format format)
{
	texture_.width = width; 
	texture_.height = height;


	texture_.image = createImage(device,vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);
	texture_.deviceMem = allocateMemory(device, physical_device, texture_.image);

	device.bindImageMemory(texture_.image, texture_.deviceMem, 0);

	texture_.cmdBuffer = createCmdBuffer(device, );

	return texture_;
}

vk::Buffer VkTexture::createStagingBuffer(vk::Device device, vk::BufferUsageFlags flags)
{
	// TODO Load Image Texture from file
	//vk::BufferCreateInfo info;
	//info.setSharingMode(vk::SharingMode::eExclusive)
	//	.setSize(sizeof(vertices))
	//	.setUsage(flags);

	//return device.createBuffer(info);
	return vk::Buffer();
}

vk::Image VkTexture::createImage(vk::Device device,vk::ImageUsageFlags flags)
{
	vk::Image texture;
	vk::ImageCreateInfo info;
	vk::DeviceMemory memory;

	info.setImageType(vk::ImageType::e2D)
		.setExtent({texture_.width,texture_.height,1})
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(vk::Format::eR8G8B8A8Srgb)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setSamples(vk::SampleCountFlagBits::e1)		// TODO multi-sample num;
		.setUsage(flags);								// TODO set flag

	return device.createImage(info);
}

vk::DeviceMemory VkTexture::allocateMemory(vk::Device device,vk::PhysicalDevice physical_device, vk::Image image)
{
	auto requirement = queryImageMemRequiredInfo(device,physical_device,image,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::MemoryAllocateInfo info;
	info.setAllocationSize(requirement.size)
		.setMemoryTypeIndex(requirement.index);
	return device.allocateMemory(info);
}

VkTexture::MemRequiredInfo VkTexture::queryImageMemRequiredInfo(vk::Device device,vk::PhysicalDevice physical_device,vk::Image image, vk::MemoryPropertyFlags flag)
{
	MemRequiredInfo info;
	auto properties = physical_device.getMemoryProperties();
	auto requirement = device.getImageMemoryRequirements(image);
	info.size = requirement.size;
	info.index = 0;
	for (int i = 0; i < properties.memoryTypeCount; i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) &&
			(properties.memoryTypes[i].propertyFlags & (flag)))
		{
			info.index = i;
		}
	}
	return info;
}

vk::CommandBuffer VkTexture::createCmdBuffer(vk::Device device )
{
	// TODO create command buffer
	return vk::CommandBuffer();
}

vk::ImageLayout VkTexture::createImageLayout(vk::Device device)
{
	//vk::com
	//return vk::ImageLayout();
}



