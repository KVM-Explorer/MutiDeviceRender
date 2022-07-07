#include "MultiRender.hpp"


#define CHECK_NULL(expr) \
	if(!(expr))		{	\
		throw std::runtime_error(#expr "is nullptr"); \
	}

struct Vertex {
	glm::vec2 position;
	glm::vec3 color;

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		vk::VertexInputBindingDescription descripotion;
		descripotion.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)	// draw a vertex then to next or  draw a primitives then next
			.setStride(sizeof(Vertex)); // 顶点的size

		return descripotion;
	}
	static auto getAttrDescription()
	{
		static std::array<vk::VertexInputAttributeDescription, 2> description;
		description[0].setLocation(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(Vertex, position));
		description[1].setLocation(1)
			.setBinding(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(Vertex, color));
		return description;
	}
};

std::array nodes{
   Vertex{{-0.5,-0.5},{1,0,0}},
   Vertex{{-0.5,0.5},{0,1,0} },
   Vertex{{0.5,0.5},{0,0,1}},


   Vertex{{0.5,-0.5},{1,0,0}},
   Vertex{{-0.5,-0.5},{0,1,0} },
   Vertex{{0.5,0.5},{0,0,1}},
};


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
	iGPU_.swapchain.swapchain = createSwapchain(iGPU_.device,iGPU_.queueIndices);
	iGPU_.swapchain.images = iGPU_.device.getSwapchainImagesKHR(iGPU_.swapchain.swapchain);
	iGPU_.swapchain.imageViews = createSwapchainImageViews(iGPU_.device,iGPU_.swapchain);
	CHECK_NULL(iGPU_.swapchain.swapchain)

	dGPU_.swapchain.swapchain = createSwapchain(dGPU_.device,dGPU_.queueIndices);
	dGPU_.swapchain.images = dGPU_.device.getSwapchainImagesKHR(dGPU_.swapchain.swapchain);
	dGPU_.swapchain.imageViews = createSwapchainImageViews(dGPU_.device, dGPU_.swapchain);
	CHECK_NULL(dGPU_.swapchain.swapchain)

	// Render Pass
	iGPU_.renderPass = createRenderPass(iGPU_.device);
	dGPU_.renderPass = createRenderPass(dGPU_.device);
	
	// Frame Buffer TODO Only iGPU
	iGPU_.frameBuffer = createFrameBuffers(iGPU_.device,iGPU_.renderPass,iGPU_.swapchain);
	dGPU_.frameBuffer = createFrameBuffers(dGPU_.device,dGPU_.renderPass,dGPU_.swapchain);

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
	dGPU_.graphicPipeline.presentAvaliableSemaphore = createSemaphore(dGPU_.device);
	dGPU_.graphicPipeline.fence = createFence(dGPU_.device);

	// PipelineLayout
	iGPU_.graphicPipeline.pipelineLayout = createPipelineLayout(iGPU_);
	CHECK_NULL(iGPU_.graphicPipeline.pipelineLayout)
	dGPU_.graphicPipeline.pipelineLayout = createPipelineLayout(dGPU_);
	CHECK_NULL(dGPU_.graphicPipeline.pipelineLayout)


	// Vertex
	createVertexBuffer(iGPU_);
	createVertexBuffer(dGPU_);

	iGPU_.index = 0;
	dGPU_.index = 1;

}

void MultiRender::release()
{

	// command buffer
	iGPU_.device.freeCommandBuffers(iGPU_.graphicPipeline.commandPool, iGPU_.graphicPipeline.commandBuffer);
	dGPU_.device.freeCommandBuffers(dGPU_.graphicPipeline.commandPool, dGPU_.graphicPipeline.commandBuffer);
	iGPU_.device.destroyCommandPool(iGPU_.graphicPipeline.commandPool);
	dGPU_.device.destroyCommandPool(dGPU_.graphicPipeline.commandPool);
	// semaphore
	iGPU_.device.destroySemaphore(iGPU_.graphicPipeline.presentAvaliableSemaphore);
	iGPU_.device.destroySemaphore(iGPU_.graphicPipeline.imageAvaliableSemaphore);
	iGPU_.device.destroyFence(iGPU_.graphicPipeline.fence);
	dGPU_.device.destroySemaphore(dGPU_.graphicPipeline.presentAvaliableSemaphore);
	dGPU_.device.destroySemaphore(dGPU_.graphicPipeline.imageAvaliableSemaphore);
	dGPU_.device.destroyFence(dGPU_.graphicPipeline.fence);


	for (auto& frame_buffer : iGPU_.frameBuffer)
	{
		iGPU_.device.destroyFramebuffer(frame_buffer);
	}
	
	iGPU_.device.destroyPipeline(iGPU_.graphicPipeline.pipeline);
	iGPU_.device.destroyPipelineLayout(iGPU_.graphicPipeline.pipelineLayout);
	iGPU_.device.destroyRenderPass(iGPU_.renderPass);
	for(auto &shader:iGPU_.graphicPipeline.shaders)
	{
		iGPU_.device.destroyShaderModule(shader);
	}
	iGPU_.device.freeMemory(iGPU_.vertex.memory);
	iGPU_.device.destroyBuffer(iGPU_.vertex.buffer);

	for (auto& frame_buffer : dGPU_.frameBuffer)
	{
		dGPU_.device.destroyFramebuffer(frame_buffer);
	}
	dGPU_.device.destroyPipeline(dGPU_.graphicPipeline.pipeline);
	dGPU_.device.destroyPipelineLayout(dGPU_.graphicPipeline.pipelineLayout);
	dGPU_.device.destroyRenderPass(dGPU_.renderPass);

	for (auto& shader : dGPU_.graphicPipeline.shaders)
	{
		dGPU_.device.destroyShaderModule(shader);
	}
	dGPU_.device.freeMemory(dGPU_.vertex.memory);
	dGPU_.device.destroyBuffer(dGPU_.vertex.buffer);


	// swap chain
	for (auto& view : dGPU_.swapchain.imageViews)
	{
		dGPU_.device.destroyImageView(view);
	}
	dGPU_.device.destroySwapchainKHR(dGPU_.swapchain.swapchain);
	for(auto &view:iGPU_.swapchain.imageViews)
	{
		iGPU_.device.destroyImageView(view);
	}
	iGPU_.device.destroySwapchainKHR(iGPU_.swapchain.swapchain);

	iGPU_.device.destroy();
	dGPU_.device.destroy();
	instance_.destroySurfaceKHR(surface_);
	instance_.destroy();
}

void MultiRender::render()
{
	// TODO computer shader

	// iGPU Render
	iGPU_.device.resetFences(iGPU_.graphicPipeline.fence);
	auto result = iGPU_.device.acquireNextImageKHR(iGPU_.swapchain.swapchain,	// TODO update
		std::numeric_limits<uint64_t>::max(),
		iGPU_.graphicPipeline.imageAvaliableSemaphore);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("acquire image failed");
	}

	uint32_t image_index = result.value;

	// draw
	iGPU_.graphicPipeline.commandBuffer.reset();
	recordCommand(iGPU_,iGPU_.graphicPipeline.commandBuffer, iGPU_.frameBuffer[image_index]);	

	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submit_info;
	submit_info.setCommandBuffers(iGPU_.graphicPipeline.commandBuffer)
		.setSignalSemaphores(iGPU_.graphicPipeline.presentAvaliableSemaphore)
		.setWaitSemaphores(iGPU_.graphicPipeline.imageAvaliableSemaphore)
		.setWaitDstStageMask(flags);
	iGPU_.graphicsQueue.submit(submit_info,iGPU_.graphicPipeline.fence);
	// present
	vk::PresentInfoKHR present_info;
	present_info.setImageIndices(image_index)	// TODO update
		.setSwapchains(iGPU_.swapchain.swapchain)
		.setWaitSemaphores(iGPU_.graphicPipeline.presentAvaliableSemaphore);

	if (iGPU_.presentQueue.presentKHR(present_info) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Render present failed");
	}

	if (iGPU_.device.waitForFences(iGPU_.graphicPipeline.fence, true, std::numeric_limits<uint64_t>::max()) !=
		vk::Result::eSuccess)
	{
		throw std::runtime_error("Render wait fence failed");
	}

	//-------------------------------------------- dGPU Render-----------------------------------------------------

	dGPU_.device.resetFences(dGPU_.graphicPipeline.fence);
	result = dGPU_.device.acquireNextImageKHR(dGPU_.swapchain.swapchain,	// TODO update
		std::numeric_limits<uint64_t>::max(),
		dGPU_.graphicPipeline.imageAvaliableSemaphore);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("acquire image failed");
	}

	image_index = result.value;

	// draw
	dGPU_.graphicPipeline.commandBuffer.reset();
	recordCommand(dGPU_, dGPU_.graphicPipeline.commandBuffer, dGPU_.frameBuffer[image_index]);

	flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	//submit_info;
	submit_info.setCommandBuffers(dGPU_.graphicPipeline.commandBuffer)
		.setSignalSemaphores(dGPU_.graphicPipeline.presentAvaliableSemaphore)
		.setWaitSemaphores(dGPU_.graphicPipeline.imageAvaliableSemaphore)
		.setWaitDstStageMask(flags);
	dGPU_.graphicsQueue.submit(submit_info, dGPU_.graphicPipeline.fence);
	// present
	//vk::PresentInfoKHR present_info;
	present_info.setImageIndices(image_index)	// TODO update
		.setSwapchains(dGPU_.swapchain.swapchain)
		.setWaitSemaphores(dGPU_.graphicPipeline.presentAvaliableSemaphore);

	if (dGPU_.presentQueue.presentKHR(present_info) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Render present failed");
	}

	if (dGPU_.device.waitForFences(dGPU_.graphicPipeline.fence, true, std::numeric_limits<uint64_t>::max()) !=
		vk::Result::eSuccess)
	{
		throw std::runtime_error("Render wait fence failed");
	}
}

void MultiRender::waitIdle()
{
	iGPU_.device.waitIdle();
	dGPU_.device.waitIdle();
}

vk::ShaderModule MultiRender::createShaderModule(const char* filename, int device_index)
{
	std::ifstream file(filename, std::ios::binary | std::ios::in);
	std::vector<char> content((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
	file.close();

	vk::ShaderModuleCreateInfo info;
	info.pCode = (uint32_t*)(content.data());
	info.codeSize = content.size();
	if (device_index == 0)
	{
		iGPU_.graphicPipeline.shaders.push_back(iGPU_.device.createShaderModule(info));
		return iGPU_.graphicPipeline.shaders.back();
	}else
	{
		dGPU_.graphicPipeline.shaders.push_back(dGPU_.device.createShaderModule(info));
		return dGPU_.graphicPipeline.shaders.back();
	}

}

void MultiRender::createComputerPipeline(vk::ShaderModule computerShader)
{
	// TODO
}

void MultiRender::createCommonPipeline(vk::ShaderModule vertex_shader, vk::ShaderModule frag_shader, int device_index)
{
	vk::GraphicsPipelineCreateInfo info;

	std::array<vk::PipelineShaderStageCreateInfo, 2> stage_infos;
	stage_infos[0].setModule(vertex_shader)
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName("main");
	stage_infos[1].setModule(frag_shader)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPName("main");
	info.setStages(stage_infos);

	// Vertex Input
	vk::PipelineVertexInputStateCreateInfo vertex_input;
	auto binding_desc = Vertex::getBindingDescription();
	auto attr_desc = Vertex::getAttrDescription();
	vertex_input.setVertexAttributeDescriptions(attr_desc)
		.setVertexBindingDescriptions(binding_desc);
	info.setPVertexInputState(&vertex_input);

	// Input Assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList);
	info.setPInputAssemblyState(&input_assembly_info);

	// Rasterization
	vk::PipelineRasterizationStateCreateInfo rast_info;
	rast_info.setRasterizerDiscardEnable(false)
		.setDepthClampEnable(false)
		.setDepthBiasEnable(false)
		.setLineWidth(1)
		.setCullMode(vk::CullModeFlagBits::eNone)
		.setPolygonMode(vk::PolygonMode::eFill);
	info.setPRasterizationState(&rast_info);

	// Multisample
	vk::PipelineMultisampleStateCreateInfo multisample;
	multisample.setSampleShadingEnable(false)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	info.setPMultisampleState(&multisample);

	// DepthStemcil
	info.setPDepthStencilState(nullptr);

	//Color blend
	vk::PipelineColorBlendStateCreateInfo color_blend;
	vk::PipelineColorBlendAttachmentState attachment_blend_state;
	attachment_blend_state.setColorWriteMask(
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eA);
	color_blend.setLogicOpEnable(false)
		.setAttachments(attachment_blend_state);
	info.setPColorBlendState(&color_blend);

	if(device_index==0)
	{
		// TODO Layout
		info.setLayout(iGPU_.graphicPipeline.pipelineLayout);
		// TODO viewport and scissor
		vk::PipelineViewportStateCreateInfo viewport_state;
		vk::Viewport viewport(0, 0,
			swapchainRequiredInfo_.extent.width,
			swapchainRequiredInfo_.extent.height,
			0, 1);
		vk::Rect2D scissor({ 0,0 },
			{ swapchainRequiredInfo_.extent.width,
				swapchainRequiredInfo_.extent.height });
		viewport_state.setViewports(viewport)
			.setScissors(scissor);
		info.setPViewportState(&viewport_state);
		// TODO Render Pass
		info.setRenderPass(iGPU_.renderPass);


		auto result = iGPU_.device.createGraphicsPipeline(nullptr, info);
		if (result.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create pipline");
		}
		iGPU_.graphicPipeline.pipeline = result.value;
	}
	else
	{
		// TODO Layout
		info.setLayout(dGPU_.graphicPipeline.pipelineLayout);
		// TODO viewport and scissor
		vk::PipelineViewportStateCreateInfo viewport_state;
		vk::Viewport viewport(0, 0,
			swapchainRequiredInfo_.extent.width,
			swapchainRequiredInfo_.extent.height,
			0, 1);
		vk::Rect2D scissor({ 0,0 },
			{ swapchainRequiredInfo_.extent.width,
				swapchainRequiredInfo_.extent.height });
		viewport_state.setViewports(viewport)
			.setScissors(scissor);
		info.setPViewportState(&viewport_state);

		// TODO Render Pass
		info.setRenderPass(dGPU_.renderPass);
		auto result = dGPU_.device.createGraphicsPipeline(nullptr, info);
		if (result.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create pipline");
		}
		dGPU_.graphicPipeline.pipeline = result.value;
	}
	

	
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
	dGPU_.presentQueue = dGPU_.device.getQueue(dGPU_.queueIndices.presentIndices.value(),0);
}

vk::SwapchainKHR MultiRender::createSwapchain(vk::Device device, RAII::QueueFamilyIndices indies)
{
	vk::SwapchainCreateInfoKHR info;
	
	info.setImageColorSpace(swapchainRequiredInfo_.format.colorSpace)
		.setImageFormat(swapchainRequiredInfo_.format.format)
		.setMinImageCount(swapchainRequiredInfo_.imageCount)
		.setImageExtent(swapchainRequiredInfo_.extent)
		.setPresentMode(swapchainRequiredInfo_.presentMode)
		.setPreTransform(swapchainRequiredInfo_.capabilities.currentTransform);

	if (indies.graphicsIndices.value() == indies.presentIndices.value())
	{
		std::array<uint32_t, 1> indices{
			indies.graphicsIndices.value(),
		};
		info.setQueueFamilyIndices(indices);
		info.setImageSharingMode(vk::SharingMode::eExclusive);	// span different device
	}
	else
	{
		std::array<uint32_t, 2> indices{
			indies.graphicsIndices.value(),
			indies.presentIndices.value(),
		};
		info.setQueueFamilyIndices(indices);
		info.setImageSharingMode(vk::SharingMode::eConcurrent);	// 队列不同，并行存储
	}
	info.setClipped(false);
	info.setSurface(surface_);
	info.setImageArrayLayers(1);	// 图层
	info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);	// 不透明
	info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	
	return device.createSwapchainKHR(info);
}

std::vector<vk::ImageView> MultiRender::createSwapchainImageViews(vk::Device device, RAII::SwapChain swapchain)
{
	std::vector<vk::ImageView> views(swapchain.images.size());
	for (int i = 0; i < views.size(); i++)
	{
		vk::ImageViewCreateInfo info;
		info.setImage(swapchain.images[i])
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

		views[i] = device.createImageView(info);
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

std::vector<vk::Framebuffer> MultiRender::createFrameBuffers(vk::Device device, vk::RenderPass render_pass,RAII::SwapChain swapchain)
{
	// 对于每个imageview 创建framebuffer
	std::vector<vk::Framebuffer> result;

	for (int i = 0; i < swapchain.imageViews.size(); i++)
	{
		vk::FramebufferCreateInfo info;
		info.setRenderPass(render_pass);
		info.setLayers(1);
		info.setWidth(swapchainRequiredInfo_.extent.width);
		info.setHeight(swapchainRequiredInfo_.extent.height);
		info.setAttachments(swapchain.imageViews[i]);

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

void MultiRender::recordCommand(RAII::Device device,vk::CommandBuffer buffer, vk::Framebuffer frame_buffer)
{
	vk::CommandBufferBeginInfo begin_info;
	begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	if (buffer.begin(&begin_info) != vk::Result::eSuccess)
	{
		throw std::runtime_error("command buffer record failed");
	}
	vk::RenderPassBeginInfo render_pass_begin_info;
	vk::ClearColorValue clear_color(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.f});
	vk::ClearValue value(clear_color);
	if(device.index ==0)
	{
		render_pass_begin_info.setRenderPass(device.renderPass)
			.setRenderArea(vk::Rect2D(
				{ 0,0 },
				swapchainRequiredInfo_.extent
				//{ 400,600 }
			))
			.setClearValues(value)
			.setFramebuffer(frame_buffer);
	}else
	{
		render_pass_begin_info.setRenderPass(device.renderPass)
			.setRenderArea(vk::Rect2D(
				{ 0,0 },
				swapchainRequiredInfo_.extent
				//{ 400,600 }
			))
			.setClearValues(value)
			.setFramebuffer(frame_buffer);
	}
	

	buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

	// TODO binding descriptor Image with pipeline
	//buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, device.graphicPipeline.pipelineLayout, 0,
	//	1, devi.descriptorSet.data(),
	//	0, 0);


	buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, device.graphicPipeline.pipeline);

	vk::DeviceSize size = 0;
	buffer.bindVertexBuffers(0, device.vertex.buffer, size);

	buffer.draw(nodes.size(), 1, 0, 0);

	buffer.endRenderPass();

	buffer.end();
}

vk::PipelineLayout MultiRender::createPipelineLayout(RAII::Device device)
{
	vk::PipelineLayoutCreateInfo info;
	return device.device.createPipelineLayout(info);
}

void MultiRender::createRenderDescriptor(RAII::Device device)
{
	//device.graphicPipeline.pipelineLayout = createPipelineLayout(device);
}


void MultiRender::createVertexBuffer(RAII::Device &device)
{
	device.vertex.buffer = createBuffer(device,vk::BufferUsageFlagBits::eVertexBuffer);
	device.vertex.memory = allocateMemory(device, device.vertex.buffer);

	CHECK_NULL(device.vertex.buffer)
	CHECK_NULL(device.vertex.memory)

	device.device.bindBufferMemory(device.vertex.buffer, device.vertex.memory, 0);

	void* data = device.device.mapMemory(device.vertex.memory, 0, sizeof(nodes));
	memcpy(data, nodes.data(), sizeof(nodes));
	device.device.unmapMemory(device.vertex.memory);
}

vk::DeviceMemory MultiRender::allocateMemory(RAII::Device device, vk::Buffer buffer)
{
	auto requirement = queryBufferMemRequiredInfo(device,buffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::MemoryAllocateInfo info;
	info.setAllocationSize(requirement.size)
		.setMemoryTypeIndex(requirement.index);
	return device.device.allocateMemory(info);
}

vk::Buffer MultiRender::createBuffer(RAII::Device device, vk::BufferUsageFlags flags)
{
	vk::BufferCreateInfo info;
	info.setSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndices(device.queueIndices.graphicsIndices.value())
		.setSize(sizeof(nodes))
		.setUsage(flags);

	return device.device.createBuffer(info);
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
		if (format.format == vk::Format::eB8G8R8A8Srgb)
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

RAII::MemRequiredInfo MultiRender::queryBufferMemRequiredInfo(RAII::Device device, vk::Buffer buffer, vk::MemoryPropertyFlags flags)
{
	RAII::MemRequiredInfo info;
	auto properties = device.physicalDevice.getMemoryProperties();
	auto requirement = device.device.getBufferMemoryRequirements(buffer);
	info.size = requirement.size;

	for (int i = 0; i < properties.memoryTypeCount; i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) &&
			(properties.memoryTypes[i].propertyFlags & (flags)))
		{
			info.index = i;
		}
	}
	return info;
}


