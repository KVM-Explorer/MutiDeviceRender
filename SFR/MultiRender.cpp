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
			.setStride(sizeof(Vertex)); // �����size

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
	iGPU_.swapchain.swapchain = createSwapchain(iGPU_.device, iGPU_.queueIndices);
	iGPU_.swapchain.images = iGPU_.device.getSwapchainImagesKHR(iGPU_.swapchain.swapchain);
	iGPU_.swapchain.imageViews = createSwapchainImageViews(iGPU_.device, iGPU_.swapchain.images);
	CHECK_NULL(iGPU_.swapchain.swapchain)

	//dGPU_.swapchain.swapchain = createSwapchain(dGPU_.device, dGPU_.queueIndices);
	//dGPU_.swapchain.images = dGPU_.device.getSwapchainImagesKHR(dGPU_.swapchain.swapchain);
	//dGPU_.swapchain.imageViews = createSwapchainImageViews(dGPU_.device, dGPU_.swapchain.images);
	//CHECK_NULL(dGPU_.swapchain.swapchain)

	// Render Pass
	iGPU_.renderPass = createRenderPass(iGPU_.device,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,vk::ImageLayout::eColorAttachmentOptimal);
	//dGPU_.renderPass = createRenderPass(dGPU_.device,vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

	// Frame Buffer TODO Only iGPU
	iGPU_.frameBuffer = createFrameBuffers(iGPU_.device, iGPU_.renderPass, iGPU_.swapchain.imageViews);

	// CommandPool Command Buffer

	iGPU_.graphicPipeline.commandPool = createCommandPool(iGPU_.device, vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
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

	// PipelineLayout TODO  Add SetLayout
	//iGPU_.graphicPipeline.pipelineLayout = createPipelineLayout(iGPU_.device);
	//CHECK_NULL(iGPU_.graphicPipeline.pipelineLayout)
	//dGPU_.graphicPipeline.pipelineLayout = createPipelineLayout(dGPU_);
	//CHECK_NULL(dGPU_.graphicPipeline.pipelineLayout)


		// Vertex
	createVertexBuffer(iGPU_);
	createVertexBuffer(dGPU_);

	iGPU_.index = 0;
	dGPU_.index = 1;

	initiGPUResource();
	initdGPUResource();
	

	//// query support copy method
	//std::cout << "iGPU" << "\t"<<std::endl;
	//querySupportBlit(iGPU_);
	//std::cout << "dGPU" << "\t"<<std::endl;
	//querySupportBlit(dGPU_);
}

void MultiRender::release()
{
	// Offscreen
	dGPU_.device.destroyFramebuffer(dGPU_.offscreen.framebuffer);
	dGPU_.device.destroySampler(dGPU_.offscreen.sampler);
	dGPU_.device.freeMemory(dGPU_.offscreen.memory);
	dGPU_.device.destroyImageView(dGPU_.offscreen.view);
	dGPU_.device.destroyImage(dGPU_.offscreen.image);

	// Mapping Image and Memory
	iGPU_.device.freeMemory(iGPU_.mappingMemory);
	iGPU_.device.destroyImage(iGPU_.mappingImage);
	dGPU_.device.freeMemory(dGPU_.mappingMemory);
	dGPU_.device.destroyImage(dGPU_.mappingImage);

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
	auto igpu_index = commonPrepare();
	renderByiGPU(igpu_index);
	renderBydGPU();
	copyOffscreenToMapping(dGPU_,0);
	copyMappingToMapping(dGPU_, iGPU_);
	updatePresentImage(igpu_index);
	presentImage(igpu_index);
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
			swapchainRequiredInfo_.extent.width/2,
			swapchainRequiredInfo_.extent.height,
			0, 1);
		vk::Rect2D scissor({ 0,0 },
			{ swapchainRequiredInfo_.extent.width/2,
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

	std::array<const char*, 1> layers{
		"VK_LAYER_KHRONOS_validation"
	};
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
		info.setImageSharingMode(vk::SharingMode::eConcurrent);	// ���в�ͬ�����д洢
	}
	info.setClipped(false);
	info.setSurface(surface_);
	info.setImageArrayLayers(1);	// ͼ��
	info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);	// ��͸��
	info.setImageUsage(
		vk::ImageUsageFlagBits::eColorAttachment|
		vk::ImageUsageFlagBits::eTransferSrc|
		vk::ImageUsageFlagBits::eTransferDst);
	return device.createSwapchainKHR(info);
}

std::vector<vk::ImageView> MultiRender::createSwapchainImageViews(vk::Device device, std::vector<vk::Image>& images)
{
	std::vector<vk::ImageView> views(images.size());
	for (int i = 0; i < views.size(); i++)
	{
		views[i] = createImageView(device,images[i]);
	}

	return views;
}

vk::ImageView MultiRender::createImageView(vk::Device device, vk::Image image)
{
	vk::ImageViewCreateInfo info;
	info.setImage(image)
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

	return device.createImageView(info);
}

vk::Sampler MultiRender::createSampler(vk::Device device)
{
	vk::SamplerCreateInfo info;
	info.setMagFilter(vk::Filter::eLinear)
		.setMinFilter(vk::Filter::eLinear)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
		.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
		.setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
		.setMinLod(0)
		.setMaxAnisotropy(1.f)
		.setCompareOp(vk::CompareOp::eNever)
		.setMinLod(0)
		.setMaxLod(0)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	return device.createSampler(info);
}

vk::DescriptorSetLayoutBinding MultiRender::setLayoutBinding(vk::DescriptorType type, vk::ShaderStageFlagBits flags, uint32_t binding, uint32_t descriptorCount)
{
	vk::DescriptorSetLayoutBinding layout_set;
	layout_set.setBinding(binding)
		.setDescriptorType(type)
		.setDescriptorCount(descriptorCount)
		.setStageFlags(flags);

	return layout_set;
}

vk::WriteDescriptorSet MultiRender::createWriteDescriptorSet(vk::DescriptorSet descriptor_set, vk::DescriptorType type, uint32_t binding, vk::DescriptorImageInfo image_info)
{
	vk::WriteDescriptorSet info;
	info.setDstSet(descriptor_set)
		.setDescriptorType(type)
		.setDstBinding(binding)
		.setPImageInfo(&image_info)
		.setDescriptorCount(1);

	return info;
}

vk::DescriptorSetLayout MultiRender::createDescriptorSetLayout(vk::Device device)
{
	vk::DescriptorSetLayoutCreateInfo info;

	std::array<vk::DescriptorSetLayoutBinding, 1> layout_bindings = {
		setLayoutBinding(vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,0)	// binding=0 image
	};

	info.setPBindings(layout_bindings.data())
		.setBindingCount(layout_bindings.size());
	return device.createDescriptorSetLayout(info);
}

vk::DescriptorPool MultiRender::createDescriptorPool(vk::Device device)
{
	auto createDescriptorSize = [](vk::DescriptorType type, uint32_t num)
	{
		vk::DescriptorPoolSize pool_size;
		pool_size.setType(type)
			.setDescriptorCount(num);
		return pool_size;
	};
	std::array<vk::DescriptorPoolSize, 1> pool_size{
		createDescriptorSize(vk::DescriptorType::eCombinedImageSampler,1)
	};
	vk::DescriptorPoolCreateInfo descriptor_pool_info;
	// TODO update max sets 
	descriptor_pool_info.setPoolSizeCount(pool_size.size())
		.setPPoolSizes(pool_size.data())
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(3);

	return device.createDescriptorPool(descriptor_pool_info);
}

std::vector<vk::DescriptorSet> MultiRender::createDescriptorSet(vk::Device device, vk::DescriptorPool descriptor_pool, vk::DescriptorSetLayout set_layout, vk::DescriptorImageInfo descriptor)
{
	vk::DescriptorSetAllocateInfo info;
	info.setDescriptorPool(descriptor_pool)
		.setSetLayouts(set_layout)
		.setDescriptorSetCount(1);

	// TODO Update
	auto result = device.allocateDescriptorSets(info);

	std::array<vk::WriteDescriptorSet, 1> write_descriptorset{
		createWriteDescriptorSet(result[0],vk::DescriptorType::eCombinedImageSampler ,0,descriptor)
	};

	device.updateDescriptorSets(write_descriptorset.size(),
		write_descriptorset.data(), 0, nullptr);
	return result;
}

void MultiRender::convertImageLayout(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
{
	auto stage_mask = vk::PipelineStageFlagBits::eAllCommands;

	auto image_memory_barrier = insertImageMemoryBarrier(cmd,image,
		vk::AccessFlagBits::eMemoryRead,
		vk::AccessFlagBits::eMemoryRead,
		old_layout,
		new_layout,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);

	cmd.pipelineBarrier(stage_mask, stage_mask, vk::DependencyFlagBits::eByRegion,
		0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void MultiRender::savePPMImage(const char* data, vk::Extent2D extent,vk::SubresourceLayout layout)
{
	// Formt BGR
	std::ofstream file("image.ppm", std::ios::out | std::ios::binary);

	//header
	file << "P6\n" << extent.width << "\n" << extent.height << "\n" << 255 << "\n";
	for(uint32_t y = 0; y<extent.height;y++)
	{
		unsigned int* row = (unsigned int*)data;
		for(uint32_t x=0;x<extent.width;x++)
		{
			file.write((char*)row + 2, 1);
			file.write((char*)row + 1, 1);
			file.write((char*)row, 1);
			row++;
		}
		data += layout.rowPitch;
	}
	file.close();
}

vk::RenderPass MultiRender::createRenderPass(vk::Device device, vk::AttachmentLoadOp load_op, 
	vk::AttachmentStoreOp store_op, 
	vk::ImageLayout init_layout,
	vk::ImageLayout final_layout)
{

	vk::RenderPassCreateInfo info;
	vk::AttachmentDescription attachment_desc;
	attachment_desc.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(load_op)
		.setStoreOp(store_op)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFormat(swapchainRequiredInfo_.format.format)
		.setInitialLayout(init_layout)
		.setFinalLayout(final_layout);
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

std::vector<vk::Framebuffer> MultiRender::createFrameBuffers(vk::Device device, vk::RenderPass render_pass, std::vector<vk::ImageView> image_views)
{
	// ����ÿ��imageview ����framebuffer
	std::vector<vk::Framebuffer> result;

	for (int i = 0; i < image_views.size(); i++)
	{
		result.push_back(createFrameBuffer(device,render_pass,image_views[i]));
	}
	return result;
}

vk::Framebuffer MultiRender::createFrameBuffer(vk::Device device, vk::RenderPass render_pass, vk::ImageView view)
{
	vk::FramebufferCreateInfo info;
	info.setRenderPass(render_pass);
	info.setLayers(1);
	info.setWidth(swapchainRequiredInfo_.extent.width);
	info.setHeight(swapchainRequiredInfo_.extent.height);
	info.setAttachments(view);
	return device.createFramebuffer(info);
}

void MultiRender::initiGPUResource()
{

	// Tmp Image to swap GPU-GPU
	vk::Extent3D extent = {
		swapchainRequiredInfo_.extent.width,
		swapchainRequiredInfo_.extent.height,
		1
	};
	iGPU_.mappingImage = createImage(iGPU_.device, extent,
		vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc,vk::ImageTiling::eLinear);
	iGPU_.mappingMemory = allocateImageMemory(iGPU_,
		vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent,
		iGPU_.mappingImage);
	iGPU_.device.bindImageMemory(iGPU_.mappingImage, iGPU_.mappingMemory, 0);

	iGPU_.graphicPipeline.pipelineLayout = createPipelineLayout(iGPU_.device);
	CHECK_NULL(iGPU_.graphicPipeline.pipelineLayout);

}

void MultiRender::initdGPUResource()
{

	vk::Extent3D extent = {
		swapchainRequiredInfo_.extent.width,
		swapchainRequiredInfo_.extent.height,
		1
	};
	// Image
	dGPU_.offscreen.image = createImage(dGPU_.device, extent,
		vk::Format::eB8G8R8A8Srgb,
		vk::ImageUsageFlagBits::eTransferSrc|
		vk::ImageUsageFlagBits::eColorAttachment,vk::ImageTiling::eOptimal);
	// Memroy
	dGPU_.offscreen.memory = allocateImageMemory(dGPU_,
		vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent,
		dGPU_.offscreen.image);
	// binding image and memory
	dGPU_.device.bindImageMemory(dGPU_.offscreen.image, dGPU_.offscreen.memory, 0);
	// image view
	dGPU_.offscreen.view = createImageView(dGPU_.device, dGPU_.offscreen.image);
	// Render Pass
	dGPU_.renderPass = createRenderPass(dGPU_.device, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
	// Frame Buffer
	dGPU_.offscreen.framebuffer = createFrameBuffer(dGPU_.device, dGPU_.renderPass, dGPU_.offscreen.view);

	// Mapping Image
	dGPU_.mappingImage = createImage(dGPU_.device, extent,
		vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eLinear);
	dGPU_.mappingMemory = allocateImageMemory(dGPU_,
		vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent,
		dGPU_.mappingImage);
	dGPU_.device.bindImageMemory(dGPU_.mappingImage, dGPU_.mappingMemory, 0);
/*
	// Descriptor
	dGPU_.offscreen.descriptor.imageLayout = vk::ImageLayout::eReadOnlyOptimal;
	dGPU_.offscreen.descriptor.imageView = dGPU_.offscreen.view;
	dGPU_.offscreen.descriptor.sampler = dGPU_.offscreen.sampler;
	// DescriptorSetLayout
	dGPU_.offscreen.descriptorSetLayout = createDescriptorSetLayout(dGPU_.device);
	// DescriptorPool
	dGPU_.offscreen.descriptorPool = createDescriptorPool(dGPU_.device);
	// DescriptorSet
	dGPU_.offscreen.descriptorSet = createDescriptorSet(dGPU_.device,dGPU_.offscreen.descriptorPool,dGPU_.offscreen.descriptorSetLayout,dGPU_.offscreen.descriptor);
*/
	// PipelineLayout
	dGPU_.graphicPipeline.pipelineLayout = createPipelineLayout(dGPU_.device/*, dGPU_.offscreen.descriptorSetLayout */ );

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

	//TODO ע��command buffer
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

void MultiRender::recordPresentCommand(RAII::Device device,vk::CommandBuffer buffer, vk::Framebuffer frame_buffer)
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

	render_pass_begin_info.setRenderPass(device.renderPass)
		.setRenderArea(vk::Rect2D(
			{ 0,0 },
			swapchainRequiredInfo_.extent
		))
		.setClearValues(value)
		.setFramebuffer(frame_buffer);

	

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

void MultiRender::recordOffScreenCommand(RAII::Device device, vk::CommandBuffer buffer, vk::Framebuffer frame)
{
	vk::CommandBufferBeginInfo begin_info;
	begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	if (buffer.begin(&begin_info)!=vk::Result::eSuccess)
	{
		throw std::runtime_error("dGPU command buffer record failed");
	}
	vk::RenderPassBeginInfo render_pass_info;
	vk::ClearColorValue clear_color(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.f});
	vk::ClearValue value(clear_color);

	render_pass_info.setRenderPass(device.renderPass)
		.setRenderArea(vk::Rect2D(
			{ 0,0 },
			swapchainRequiredInfo_.extent
		))
		.setClearValues(value)
		.setFramebuffer(frame);
	
	buffer.beginRenderPass(render_pass_info,vk::SubpassContents::eInline);

	buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,device.graphicPipeline.pipeline);
	vk::DeviceSize size = 0;
	buffer.bindVertexBuffers(0, device.vertex.buffer,size);

	buffer.draw(nodes.size(), 1, 0, 0);

	buffer.endRenderPass();

	buffer.end();

}

vk::PipelineLayout MultiRender::createPipelineLayout(vk::Device device/*, vk::DescriptorSetLayout set_layout*/)
{
	vk::PipelineLayoutCreateInfo info;
	//info.setSetLayoutCount(1);
	//info.setSetLayouts(set_layout);
	return device.createPipelineLayout(info);
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

vk::Image MultiRender::createImage(vk::Device device,vk::Extent3D extent , vk::Format format, vk::ImageUsageFlags flags,vk::ImageTiling tiling)
{
	vk::ImageCreateInfo info;

	info.setFormat(format)
		.setImageType(vk::ImageType::e2D)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setTiling(tiling)
		.setExtent(extent)
		.setArrayLayers(1)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setUsage(flags);

	return device.createImage(info);
}

vk::DeviceMemory MultiRender::allocateImageMemory(RAII::Device& device, vk::MemoryPropertyFlags flags, vk::Image image)
{
	vk::MemoryAllocateInfo info;
	auto requirements = queryImageMemRequiredInfo(device, image, flags);
	info.setMemoryTypeIndex(requirements.index)
		.setAllocationSize(requirements.size);
	return device.device.allocateMemory(info);
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
	auto support_usage = info.capabilities.supportedUsageFlags;
	
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
			(properties.memoryTypes[i].propertyFlags & (flags))==flags)
		{
			info.index = i;
		}
	}
	return info;
}

RAII::MemRequiredInfo MultiRender::queryImageMemRequiredInfo(RAII::Device device, vk::Image image, vk::MemoryPropertyFlags flags)
{
	RAII::MemRequiredInfo info;
	auto properties = device.physicalDevice.getMemoryProperties();
	auto requirement = device.device.getImageMemoryRequirements(image);
	info.size = requirement.size;
	info.index = 0;
	for (int i = 0; i < properties.memoryTypeCount; i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) &&
			(properties.memoryTypes[i].propertyFlags & (flags))==flags)
		{
			info.index = i;
		}
	}
	return info;
}

void MultiRender::querySupportBlit(RAII::Device& device)
{

	vk::FormatProperties format_properties;
	// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
	device.physicalDevice.getFormatProperties(vk::Format::eB8G8R8A8Srgb, &format_properties);
	if(!(format_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc))
	{
		std::cout << "Device doesn't support blitting from optimal tiled images, using copy instead of blit" << std::endl;
		device.isSupportsBlit = false;
	}
	if(!(format_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst))
	{
		std::cout << "Device doesn't support blitting from optimal tiled images, using copy instead of blit" << std::endl;
		device.isSupportsBlit = false;
	}
}


void MultiRender::copyPresentImage(RAII::Device& src, RAII::Device& dst, uint32_t src_index, uint32_t dt)
{
	vk::CommandBuffer copy_command;
	copyOffscreenToMapping(src, src_index);
	copyMappingToMapping(src, dst);
	copyMappingToPresent(src,src_index);
	

}

void MultiRender::copyOffscreenToMapping(RAII::Device& src, uint32_t src_index)
{
	vk::CommandBuffer copy_command = startSingleCommand(src, src.graphicPipeline.commandPool);
	// Add Image Memory Barrier

	insertImageMemoryBarrier(		// mapping image
		copy_command,
		src.mappingImage,
		vk::AccessFlagBits::eNone,
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer);
	insertImageMemoryBarrier(		// offscren image
		copy_command,
		src.offscreen.image,
		vk::AccessFlagBits::eMemoryRead,
		vk::AccessFlagBits::eTransferRead,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eTransferSrcOptimal,
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer);

	// Step 1: Copy Image src Device to src host visable

	vk::ImageCopy image_copy_region;

	image_copy_region.setDstSubresource({ vk::ImageAspectFlagBits::eColor,0,0,1 })
		.setSrcSubresource({ vk::ImageAspectFlagBits::eColor,0,0,1 })
		.setExtent({ swapchainRequiredInfo_.extent.width,swapchainRequiredInfo_.extent.height,1 });
	copy_command.copyImage(src.offscreen.image,
		vk::ImageLayout::eTransferSrcOptimal,
		src.mappingImage,
		vk::ImageLayout::eTransferDstOptimal,
		1, &image_copy_region);

	insertImageMemoryBarrier( copy_command, src.mappingImage,	// mapping image
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eTransferRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);
	insertImageMemoryBarrier( copy_command, src.offscreen.image,	// present image
		vk::AccessFlagBits::eTransferRead,
		vk::AccessFlagBits::eMemoryRead,
		vk::ImageLayout::eTransferSrcOptimal,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);
	endSingleCommand(src, copy_command, src.graphicPipeline.commandPool, src.graphicsQueue);
}

void MultiRender::copyMappingToMapping(RAII::Device& src, RAII::Device& dst)
{
	// Step 2: Copy Image src host to dst host by mapping memory
	auto copy_command = startSingleCommand(dst, dst.graphicPipeline.commandPool);
	insertImageMemoryBarrier( copy_command, dst.mappingImage,
		vk::AccessFlagBits::eNone,
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);
	endSingleCommand(dst, copy_command, dst.graphicPipeline.commandPool, dst.graphicsQueue);

	//vk::ImageSubresource subresource{ vk::ImageAspectFlagBits::eColor,0,0 };
	//auto subresource_layout = src.device.getImageSubresourceLayout(src.mappingImage, subresource);

	auto requirements = queryImageMemRequiredInfo(src, src.mappingImage,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);


	void* dst_data = dst.device.mapMemory(dst.mappingMemory, 0, VK_WHOLE_SIZE);
	void* src_data = src.device.mapMemory(src.mappingMemory, 0, VK_WHOLE_SIZE);
	
	memcpy(dst_data, src_data, requirements.size);
	//savePPMImage((const char*)src_data, swapchainRequiredInfo_.extent, subresource_layout);
	src.device.unmapMemory(src.mappingMemory);
	dst.device.unmapMemory(dst.mappingMemory);
}

void MultiRender::copyMappingToPresent(RAII::Device& src, uint32_t src_index)
{
	// Step 3: Copy Image dst host to dst Device
	vk::CommandBuffer copy_command;
	copy_command = startSingleCommand(src, src.graphicPipeline.commandPool);
	insertImageMemoryBarrier( copy_command, src.mappingImage,		// mapping image
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eTransferRead,
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eTransferSrcOptimal,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);

	insertImageMemoryBarrier( copy_command, src.swapchain.images[src_index],	// present image
		vk::AccessFlagBits::eMemoryRead,
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);

	vk::ImageCopy image_copy_region;
	image_copy_region.setExtent({ swapchainRequiredInfo_.extent.width,swapchainRequiredInfo_.extent.height,1 })
		.setSrcSubresource({ vk::ImageAspectFlagBits::eColor,0,0 ,1 })
		.setDstSubresource({ vk::ImageAspectFlagBits::eColor,0,0,1 });

	copy_command.copyImage(src.mappingImage,		// mapping image
		vk::ImageLayout::eTransferSrcOptimal,
		src.swapchain.images[src_index],
		vk::ImageLayout::eTransferDstOptimal,
		1, &image_copy_region);

	insertImageMemoryBarrier( copy_command, src.swapchain.images[src_index],  //present image
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eMemoryRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);
	insertImageMemoryBarrier( copy_command, src.mappingImage,	 // mapping Image
		vk::AccessFlagBits::eTransferRead,
		vk::AccessFlagBits::eNone,
		vk::ImageLayout::eTransferSrcOptimal,
		vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);

	endSingleCommand(src, copy_command, src.graphicPipeline.commandPool, src.graphicsQueue);
}

vk::ImageMemoryBarrier MultiRender::insertImageMemoryBarrier(vk::CommandBuffer command_buffer,
                                                             vk::Image image,
                                                             vk::AccessFlags src_access,
                                                             vk::AccessFlags dst_access,
                                                             vk::ImageLayout old_layout,
                                                             vk::ImageLayout new_layout,
                                                             vk::PipelineStageFlags src_mask, vk::PipelineStageFlags dst_mask)
															 
{
	vk::ImageMemoryBarrier barrier;
	barrier.setOldLayout(old_layout)
		.setNewLayout(new_layout)
		.setSrcAccessMask(src_access)
		.setDstAccessMask(dst_access)
		.setSubresourceRange({ vk::ImageAspectFlagBits::eColor,0,1,0,1 })
		.setImage(image);


	command_buffer.pipelineBarrier(src_mask, dst_mask,vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &barrier);
	return barrier;
}

uint32_t MultiRender::commonPrepare()
{
	// acquire image index
	auto result = iGPU_.device.acquireNextImageKHR(iGPU_.swapchain.swapchain,	// TODO update
		std::numeric_limits<uint64_t>::max(),
		iGPU_.graphicPipeline.imageAvaliableSemaphore, 
		nullptr);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to acquire iGPU image failed");
	}

	uint32_t igpu_index = result.value;
	return igpu_index;
}

void MultiRender::renderBydGPU()
{
	// reset fence
	dGPU_.device.resetFences(dGPU_.graphicPipeline.fence);
	// Step1 render Image
	// draw
	dGPU_.graphicPipeline.commandBuffer.reset();
	recordOffScreenCommand(dGPU_, dGPU_.graphicPipeline.commandBuffer, dGPU_.offscreen.framebuffer);
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submit_info;
	submit_info.setCommandBuffers(dGPU_.graphicPipeline.commandBuffer);
	dGPU_.graphicsQueue.submit(submit_info, dGPU_.graphicPipeline.fence);

	// Step2 wait for fence
	auto result = dGPU_.device.waitForFences(dGPU_.graphicPipeline.fence, true, std::numeric_limits<uint64_t>::max());
	if(result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to render by dGPU");
	}
	
}

void MultiRender::renderByiGPU(uint32_t igpu_index)
{
	// reset fence
	iGPU_.device.resetFences(iGPU_.graphicPipeline.fence);
	//Step 1 render Image
	// draw
	iGPU_.graphicPipeline.commandBuffer.reset();
	recordPresentCommand(iGPU_, iGPU_.graphicPipeline.commandBuffer, iGPU_.frameBuffer[igpu_index]);
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submit_info;
	submit_info.setCommandBuffers(iGPU_.graphicPipeline.commandBuffer)
		.setSignalSemaphores(iGPU_.graphicPipeline.presentAvaliableSemaphore)
		.setWaitSemaphores(iGPU_.graphicPipeline.imageAvaliableSemaphore)
		.setWaitDstStageMask(flags);
	iGPU_.graphicsQueue.submit(submit_info, iGPU_.graphicPipeline.fence);
}

void MultiRender::presentImage(uint32_t igpu_index)
{
	// Convert Image Layoyut
	auto cmd = startSingleCommand(iGPU_, iGPU_.graphicPipeline.commandPool);
	convertImageLayout(cmd, iGPU_.swapchain.images[igpu_index], vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
	endSingleCommand(iGPU_, cmd, iGPU_.graphicPipeline.commandPool, iGPU_.graphicsQueue);

	// present iGPU swapchain Image
	vk::PresentInfoKHR present_info;
	present_info.setImageIndices(igpu_index)
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
}

void MultiRender::updatePresentImage(uint32_t igpu_index)
{
	// TODO merge iGPU and dGPU render result
	vk::CommandBuffer cmd;
	cmd = startSingleCommand(iGPU_, iGPU_.graphicPipeline.commandPool);

	// mapping image as src
	insertImageMemoryBarrier(cmd, iGPU_.mappingImage,
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eTransferRead,
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eTransferSrcOptimal,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);
	// swapchain image as dst
	insertImageMemoryBarrier(cmd, iGPU_.swapchain.images[igpu_index],
		vk::AccessFlagBits::eNone,
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eTransferDstOptimal,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);

	vk::ImageCopy copy_region;
	copy_region.setSrcSubresource({ vk::ImageAspectFlagBits::eColor,0,0,1 })
		.setDstSubresource({ vk::ImageAspectFlagBits::eColor,0,0,1 })
		.setExtent({ swapchainRequiredInfo_.extent.width/2 ,swapchainRequiredInfo_.extent.height,1 });

	cmd.copyImage(iGPU_.mappingImage, vk::ImageLayout::eTransferSrcOptimal,
		iGPU_.swapchain.images[igpu_index], vk::ImageLayout::eTransferDstOptimal,
		1, &copy_region);
	// reset swapchain image layout
	insertImageMemoryBarrier(cmd, iGPU_.swapchain.images[igpu_index],
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eMemoryRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);
	// reset mapping image layout
	insertImageMemoryBarrier(cmd, iGPU_.mappingImage,
		vk::AccessFlagBits::eTransferRead,
		vk::AccessFlagBits::eNone,
		vk::ImageLayout::eTransferSrcOptimal,
		vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer);

	endSingleCommand(iGPU_, cmd, iGPU_.graphicPipeline.commandPool, iGPU_.graphicsQueue);
}

void MultiRender::endSingleCommand(RAII::Device& device, vk::CommandBuffer& command, vk::CommandPool pool, vk::Queue& queue)
{
	command.end();

	vk::SubmitInfo submit_info;
	submit_info.setCommandBufferCount(1)
		.setCommandBuffers(command);

	vk::Fence fence = createFence(device.device);
	queue.submit(submit_info, fence);

	auto result = device.device.waitForFences(1,&fence,true,std::numeric_limits<uint64_t>::max());

	device.device.destroyFence(fence);
	device.device.freeCommandBuffers(pool,command);
}

vk::CommandBuffer MultiRender::startSingleCommand(RAII::Device& device, vk::CommandPool command_pool)
{
	vk::CommandBuffer  command = createCommandBuffer(device.device, command_pool);
	vk::CommandBufferBeginInfo begin_info;
	begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	command.begin(begin_info);
	return command;
}


