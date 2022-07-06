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
	iGPU_.queueIndices = queryPhysicalDeviceQueue(iGPU_.physicalDevice);
	dGPU_.queueIndices = queryPhysicalDeviceQueue(dGPU_.physicalDevice);

	iGPU_.device = createDevice(iGPU_);
	dGPU_.device = createDevice(dGPU_);
	CHECK_NULL(iGPU_.device)
	CHECK_NULL(dGPU_.device)

	// create Command Queue
	createQueue();

	// ceate Swap chain
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	swapchainRequiredInfo_ = querySwapChainRequiredInfo(w, h);
	swapchain_.swapchain = createSwapchain();
	swapchain_.images = iGPU_.device.getSwapchainImagesKHR(swapchain_.swapchain);
	swapchain_.imageViews = createSwapchainImageViews();

	// Render Pass
	iGPU_.renderPass = createRenderPass(iGPU_.device);
	dGPU_.renderPass = createRenderPass(dGPU_.device);
	
	// Frame Buffer TODO Only iGPU
	iGPU_.frameBuffer = createFrameBuffers(iGPU_.device,iGPU_.renderPass);
	//dGPU_.frameBuffer = createFrameBuffers(dGPU_.device,dGPU_.renderPass);

	// CommandPool Command Buffer
	
	iGPU_.graphicPipeline.commandPool = createCommandPool(iGPU_.device,vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
		iGPU_.queueIndices.computerIndices.value());
	CHECK_NULL(iGPU_.graphicPipeline.commandPool)
	iGPU_.graphicPipeline.commandBuffer = createCommandBuffer(iGPU_.device, iGPU_.graphicPipeline.commandPool);
	CHECK_NULL(iGPU_.graphicPipeline.commandBuffer)

	dGPU_.graphicPipeline.commandPool = createCommandPool(dGPU_.device, vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			dGPU_.queueIndices.computerIndices.value());
	CHECK_NULL(dGPU_.graphicPipeline.commandPool)
	dGPU_.graphicPipeline.commandBuffer = createCommandBuffer(dGPU_.device, dGPU_.graphicPipeline.commandPool);
	CHECK_NULL(dGPU_.graphicPipeline.commandBuffer)

	
		// Semaphore include semaphore and fence
	iGPU_.graphicPipeline.imageAvaliableSemaphore = createSemaphore(iGPU_.device);
	iGPU_.graphicPipeline.presentAvaliableSemaphore = createSemaphore(iGPU_.device);
	iGPU_.graphicPipeline.fence = createFence(iGPU_.device);

	dGPU_.graphicPipeline.imageAvaliableSemaphore = createSemaphore(dGPU_.device);
	//dGPU_.graphicPipeline.presentAvaliableSemaphore = createSemaphore(dGPU_.device);
	dGPU_.graphicPipeline.fence = createFence(dGPU_.device);

	


}

void MultiRender::release()
{

	// semaphore
	iGPU_.device.destroySemaphore(iGPU_.graphicPipeline.presentAvaliableSemaphore);
	iGPU_.device.destroySemaphore(iGPU_.graphicPipeline.imageAvaliableSemaphore);
	iGPU_.device.destroyFence(iGPU_.graphicPipeline.fence);

	dGPU_.device.destroySemaphore(dGPU_.graphicPipeline.imageAvaliableSemaphore);
	dGPU_.device.destroyFence(dGPU_.graphicPipeline.fence);
	

	iGPU_.device.freeCommandBuffers(iGPU_.graphicPipeline.commandPool, iGPU_.graphicPipeline.commandBuffer);
	dGPU_.device.freeCommandBuffers(dGPU_.graphicPipeline.commandPool, dGPU_.graphicPipeline.commandBuffer);
	iGPU_.device.destroyCommandPool(iGPU_.graphicPipeline.commandPool);
	dGPU_.device.destroyCommandPool(dGPU_.graphicPipeline.commandPool);
	for(auto &frame_buffer:iGPU_.frameBuffer)
	{
		iGPU_.device.destroyFramebuffer(frame_buffer);
	}
	iGPU_.device.destroyRenderPass(iGPU_.renderPass);
	dGPU_.device.destroyRenderPass(dGPU_.renderPass);
	for(auto &view:swapchain_.imageViews)
	{
		iGPU_.device.destroyImageView(view);
	}
	iGPU_.device.destroySwapchainKHR(swapchain_.swapchain);
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
		std::array<uint32_t, 1> indices{
			iGPU_.queueIndices.graphicsIndices.value(),
		};
		info.setQueueFamilyIndices(indices);
		info.setImageSharingMode(vk::SharingMode::eExclusive);	// span different device
	}
	else
	{
		std::array<uint32_t, 2> indices{
			iGPU_.queueIndices.graphicsIndices.value(),
			iGPU_.queueIndices.presentIndices.value(),
		};
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

std::vector<vk::ImageView> MultiRender::createSwapchainImageViews()
{
	std::vector<vk::ImageView> views(swapchain_.images.size());
	for (int i = 0; i < views.size(); i++)
	{
		vk::ImageViewCreateInfo info;
		info.setImage(swapchain_.images[i])
			.setFormat(swapchainRequiredInfo_.format.format)
			.setViewType(vk::ImageViewType::e2D);
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)
			.setLevelCount(1)
			.setLayerCount(1)
			.setBaseArrayLayer(0)
			.setAspectMask(vk::ImageAspectFlagBits::eColor);
		info.setSubresourceRange(range);
		vk::ComponentMapping mapping;
		info.setComponents(mapping);

		views[i] = iGPU_.device.createImageView(info);
	}

	return views;
}

vk::RenderPass MultiRender::createRenderPass(vk::Device device)
{

	vk::RenderPassCreateInfo info;
	vk::AttachmentDescription attachment_desc;
	attachment_desc.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFormat(swapchainRequiredInfo_.format.format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	info.setAttachments(attachment_desc);

	vk::SubpassDescription subpass_desc;
	vk::AttachmentReference refer;
	refer.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	refer.setAttachment(0);
	subpass_desc.setColorAttachments(refer);
	subpass_desc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

	info.setSubpasses(subpass_desc);

	return device.createRenderPass(info);
}

std::vector<vk::Framebuffer> MultiRender::createFrameBuffers(vk::Device device, vk::RenderPass render_pass)
{
	// 对于每个imageview 创建framebuffer
	std::vector<vk::Framebuffer> result;

	for (int i = 0; i < swapchain_.imageViews.size(); i++)
	{
		vk::FramebufferCreateInfo info;
		info.setRenderPass(render_pass);
		info.setLayers(1);
		info.setWidth(swapchainRequiredInfo_.extent.width);
		info.setHeight(swapchainRequiredInfo_.extent.height);
		info.setAttachments(swapchain_.imageViews[i]);

		result.push_back(device.createFramebuffer(info));
	}
	return result;
}

vk::CommandPool MultiRender::createCommandPool(vk::Device device, vk::CommandPoolCreateFlagBits flags, uint32_t index)
{
	vk::CommandPoolCreateInfo info;
	info.setFlags(flags);
	info.setQueueFamilyIndex(index);

	return device.createCommandPool(info);
}

vk::CommandBuffer MultiRender::createCommandBuffer(vk::Device device, vk::CommandPool command_pool)
{
	vk::CommandBufferAllocateInfo info;
	info.setCommandPool(command_pool);
	info.setCommandBufferCount(1);
	info.setLevel(vk::CommandBufferLevel::ePrimary);

	//TODO 注意command buffer
	return device.allocateCommandBuffers(info)[0];
}

vk::Semaphore MultiRender::createSemaphore(vk::Device device)
{
	vk::SemaphoreCreateInfo info;

	return device.createSemaphore(info);
}

vk::Fence MultiRender::createFence(vk::Device device)
{
	vk::FenceCreateInfo info;
	return device.createFence(info);
}

RAII::QueueFamilyIndices MultiRender::queryPhysicalDeviceQueue(vk::PhysicalDevice physical_device)
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


