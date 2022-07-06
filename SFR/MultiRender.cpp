#include "MultiRender.hpp"


#define CHECK_NULL(expr) \
	if(!(expr))		{	\
		throw std::runtime_error(#expr "is nullptr"); \
	}

void MultiRender::init(GLFWwindow* window)
{
	// Instance
	instance_ = createInstance();
	CHECK_NULL(instance_)
	// Surface
	surface_ = createSurface(window);
	CHECK_NULL(surface_)
	// physical GPU
	createPhysicalDevice();
	CHECK_NULL(iGPU_.physicalDevice)
	CHECK_NULL(dGPU_.physicalDevice)
	// Logic GPU
	iGPU_.queueIndices = queryPhysicalDevice(iGPU_.physicalDevice);
	dGPU_.queueIndices = queryPhysicalDevice(dGPU_.physicalDevice);

	iGPU_.device = createDevice(iGPU_);
	dGPU_.device = createDevice(dGPU_);
	CHECK_NULL(iGPU_.device)
	CHECK_NULL(dGPU_.device)

	// create Command Queue
	createQueue();

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	swapchainRequiredInfo_ = querySwapChainRequiredInfo(w, h);





	
}

void MultiRender::release()
{

	iGPU_.device.destroySwapchainKHR(swapchain_);
	iGPU_.device.destroy();
	dGPU_.device.destroy();
	instance_.destroySurfaceKHR(surface_);
	instance_.destroy();
}

void MultiRender::render()
{
}

void MultiRender::waitIdle()
{
}

vk::ShaderModule MultiRender::createShaderModule(const char* filename)
{
	return vk::ShaderModule();
}

void MultiRender::createComputerPipeline(vk::ShaderModule computerShader)
{
}

void MultiRender::createCommonPipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader)
{
}

vk::Instance MultiRender::createInstance()
{
	unsigned int count;
	std::cout << "Vulkan Extensions" << std::endl;

	glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions(count);
	auto ext_array = glfwGetRequiredInstanceExtensions(&count);

	for (unsigned int i = 0; i < count; i++)
	{
		std::cout << "\t" << ext_array[i] << std::endl;
		extensions[i] = ext_array[i];
	}
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	std::cout << "Layer Name:" << std::endl;

	std::array<const char*, 1> layers{ "VK_LAYER_KHRONOS_validation" };
	auto layer_names = vk::enumerateInstanceLayerProperties();
	for (auto& layer : layer_names)
	{
		std::cout << "\t" << layer.layerName << std::endl;
	}

	vk::InstanceCreateInfo info;
	info.setPEnabledExtensionNames(extensions);
	info.setPEnabledLayerNames(layers);

	return vk::createInstance(info);
}

vk::SurfaceKHR MultiRender::createSurface(GLFWwindow* window)
{
	VkSurfaceKHR surface;
	auto result = glfwCreateWindowSurface(instance_, window, nullptr, &surface);
	if (result != VkResult::VK_SUCCESS)
	{
		throw std::runtime_error("surface create failed");
	}

	return surface;
}

void MultiRender::createPhysicalDevice()
{
	auto phycial_device = instance_.enumeratePhysicalDevices();
	for (auto& device : phycial_device)
	{
		auto dproperty = device.getProperties();
		auto features = device.getFeatures();
		
	}
	// TODO 配置多设备
	iGPU_.physicalDevice = phycial_device[0];
	std::cout << "iGPU: " << phycial_device[0].getProperties().deviceName << std::endl;
	dGPU_.physicalDevice = phycial_device[1];
	std::cout << "dGPU: " << phycial_device[1].getProperties().deviceName << std::endl;

}



vk::Device MultiRender::createDevice(RAII::Device& device)
{
	std::vector<vk::DeviceQueueCreateInfo> queue_infos;
	// grahics and present
	if (device.queueIndices.graphicsIndices.value() == device.queueIndices.presentIndices.value())
	{
		vk::DeviceQueueCreateInfo info;
		float priority = 1.0f;
		info.setQueuePriorities(priority);
		info.setQueueFamilyIndex(device.queueIndices.graphicsIndices.value());
		queue_infos.push_back(info);
	}
	else
	{
		vk::DeviceQueueCreateInfo info1;
		float priority = 1.0f;
		info1.setQueuePriorities(priority);
		info1.setQueueFamilyIndex(device.queueIndices.graphicsIndices.value());


		vk::DeviceQueueCreateInfo info2;
		info2.setQueuePriorities(priority);
		info2.setQueueFamilyIndex(device.queueIndices.presentIndices.value());

		queue_infos.push_back(info1);
		queue_infos.push_back(info2);
	}

	std::array<const char*, 1> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo info;

	info.setQueueCreateInfos(queue_infos);
	info.setPEnabledExtensionNames(extensions);

	return device.physicalDevice.createDevice(info);
}

void MultiRender::createQueue()
{
	iGPU_.presentQueue = iGPU_.device.getQueue(iGPU_.queueIndices.presentIndices.value(), 0);
	iGPU_.graphicsQueue = iGPU_.device.getQueue(iGPU_.queueIndices.graphicsIndices.value(), 0);

	dGPU_.computerQueue = dGPU_.device.getQueue(dGPU_.queueIndices.computerIndices.value(), 0);
	dGPU_.graphicsQueue= dGPU_.device.getQueue(dGPU_.queueIndices.graphicsIndices.value(), 0);
}

vk::SwapchainKHR MultiRender::createSwapchain()
{
	vk::SwapchainCreateInfoKHR info;
	info.setImageColorSpace(swapchainRequiredInfo_.format.colorSpace)
		.setImageFormat(swapchainRequiredInfo_.format.format)
		.setMinImageCount(swapchainRequiredInfo_.imageCount)
		.setImageExtent(swapchainRequiredInfo_.extent)
		.setPresentMode(swapchainRequiredInfo_.presentMode)
		.setPreTransform(swapchainRequiredInfo_.capabilities.currentTransform);

	if (iGPU_.queueIndices.graphicsIndices.value() == iGPU_.queueIndices.presentIndices.value())
	{
		info.setQueueFamilyIndices(iGPU_.queueIndices.graphicsIndices.value());
		info.setImageSharingMode(vk::SharingMode::eConcurrent);	// span different device
	}
	else
	{
		std::array<uint32_t, 2> indices{ iGPU_.queueIndices.graphicsIndices.value(),
									iGPU_.queueIndices.presentIndices.value() };
		info.setQueueFamilyIndices(indices);
		info.setImageSharingMode(vk::SharingMode::eConcurrent);	// 队列不同，并行存储
	}
	info.setClipped(false);
	info.setSurface(surface_);
	info.setImageArrayLayers(1);	// 图层
	info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);	// 不透明
	info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	return iGPU_.device.createSwapchainKHR(info);
}

RAII::QueueFamilyIndices MultiRender::queryPhysicalDevice(vk::PhysicalDevice physical_device)
{

	auto familes = physical_device.getQueueFamilyProperties();
	RAII::QueueFamilyIndices indices;
	uint32_t idx = 0;
	for (auto& family : familes)
	{
		if (family.queueFlags | vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsIndices = idx;
		}

		if (physical_device.getSurfaceSupportKHR(idx, surface_))
		{
			indices.presentIndices = idx;
		}
		if (family.queueFlags | vk::QueueFlagBits::eCompute)
		{
			indices.computerIndices = idx;
		}
		if (indices.graphicsIndices &&
			indices.presentIndices &&
			indices.computerIndices) break;
		idx++;
	}
	return indices;
}

RAII::SwapChainRequiredInfo MultiRender::querySwapChainRequiredInfo(uint32_t w, uint32_t h)
{
	RAII::SwapChainRequiredInfo info;
	info.capabilities = iGPU_.physicalDevice.getSurfaceCapabilitiesKHR(surface_);
	auto formats = iGPU_.physicalDevice.getSurfaceFormatsKHR(surface_);
	info.format = formats[0];
	for (auto& format : formats)
	{
		if (format.format == vk::Format::eR8G8B8A8Srgb ||
			format.format == vk::Format::eB8G8R8A8Srgb)
		{
			info.format = format;
		}
	}
	info.extent.width = std::clamp<uint32_t>(w,
		info.capabilities.minImageExtent.width,
		info.capabilities.maxImageExtent.width);
	info.extent.height = std::clamp<uint32_t>(h,
		info.capabilities.minImageExtent.height,
		info.capabilities.maxImageExtent.height);
	info.imageCount = std::clamp<uint32_t>(2,
		info.capabilities.minImageCount,
		info.capabilities.maxImageCount);
	auto present_mode = iGPU_.physicalDevice.getSurfacePresentModesKHR(surface_);
	info.presentMode = vk::PresentModeKHR::eFifo;
	for (auto& present : present_mode)
	{
		if (present == vk::PresentModeKHR::eMailbox)
		{
			info.presentMode = present;
		}
	}

	return info;
}


