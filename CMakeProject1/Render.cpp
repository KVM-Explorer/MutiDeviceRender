#pragma	once
#include "Render.hpp"


Render::QueueFamilyIndices Render::queueIndices_ ;
Render::SwapChainRequiredInfo Render::requiredInfo_;


vk::Instance Render::instance_ = nullptr;
vk::SurfaceKHR Render::surface_ = nullptr;
vk::PhysicalDevice Render::physicalDevice_ = nullptr;
vk::Device	Render::device_ = nullptr;
vk::Queue	Render::graphicQueue_ = nullptr;
vk::Queue	Render::presentQueue_ = nullptr;
vk::SwapchainKHR Render::swapchain_ = nullptr;
std::vector<vk::Image> Render::images_;
std::vector<vk::ImageView> Render::imageViews_;
std::vector<vk::ShaderModule> Render::shaderModules_;
vk::Pipeline	Render::pipeline_ = nullptr;
vk::PipelineLayout Render::layout_ = nullptr;
vk::RenderPass Render::renderPass_ = nullptr;
vk::CommandPool Render::commandPool_ = nullptr;
vk::CommandBuffer Render::commandBuffer_ = nullptr;
std::vector<vk::Framebuffer> Render::frameBuffer_;
vk::Semaphore Render::imageAvaliableSemaphore_ = nullptr;
vk::Semaphore Render::renderFinishSemaphore_ = nullptr;
vk::Fence Render::fence_ = nullptr;
vk::Buffer Render::vertexBuffer_ = nullptr;
vk::DeviceMemory Render::vertexMemory_ = nullptr;
Render::Texture Render::texture_;

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

std::array vertices{
	Vertex{{-0.5,-0.5},{1,0,0}},
	Vertex{{-0.5,0.5},{0,1,0} },
	Vertex{{0.5,0.5},{0,0,1}},


	Vertex{{0.5,-0.5},{1,0,0}},
	Vertex{{-0.5,-0.5},{0,1,0} },
	Vertex{{0.5,0.5},{0,0,1}},
};

#define CHECK_NULL(expr) \
	if(!(expr))			\
		throw std::runtime_error(#expr "is nullptr");

//std::array vertices{
//	Vertex
//}

void Render::init(GLFWwindow* window)
{

	unsigned int count;

	std::cout << "Vulkan Extensions" << std::endl;

	glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions(count);
	auto ext_array =  glfwGetRequiredInstanceExtensions(&count);

	for(unsigned int i=0;i < count;i++)
	{
		std::cout << "\t" << ext_array[i] << std::endl;
		extensions[i] = ext_array[i];
	}

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	instance_ = createInstance(extensions);
	CHECK_NULL(instance_)

	surface_ = createSurface(window);
	CHECK_NULL(surface_)

	physicalDevice_ = pickupPhysicalDevice();
	CHECK_NULL(physicalDevice_)
	std::cout << "pick up " << physicalDevice_.getProperties().deviceName << std::endl;

	queueIndices_ = queryPhysicalDevice();


	device_ = createDevice();
	CHECK_NULL(device_)

	graphicQueue_ = device_.getQueue(queueIndices_.graphicsIndices.value(),0);
	presentQueue_ = device_.getQueue(queueIndices_.presentIndices.value(),0);

	CHECK_NULL(graphicQueue_)
	CHECK_NULL(physicalDevice_)

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	requiredInfo_ = querySwapChainRequiredInfo(w, h);

	swapchain_ = createSwapchain();
	CHECK_NULL(swapchain_)
	images_ = device_.getSwapchainImagesKHR(swapchain_);
	imageViews_ = createImageViews();

	

	layout_ = createLayout();
	CHECK_NULL(layout_)

	renderPass_ = createRenderPass();
	CHECK_NULL(renderPass_)
	
	frameBuffer_ = createFreamebuffers();
	for(auto &framebuffer:frameBuffer_)
	{
		CHECK_NULL(framebuffer);
	}

	commandPool_ = createCommandPool();
	CHECK_NULL(commandPool_)

	commandBuffer_ = createCommandBuffer();
	CHECK_NULL(commandBuffer_)

	imageAvaliableSemaphore_ = createSemaphore();
	renderFinishSemaphore_ =createSemaphore();

	CHECK_NULL(imageAvaliableSemaphore_)
	CHECK_NULL(renderFinishSemaphore_)

	fence_ = createFence();
	CHECK_NULL(fence_)

	createBuffer();
	createTexture();
}

vk::Instance Render::createInstance(std::vector<const char*>& extensions)
{

	std::cout << "Layer Name:" << std::endl;

	std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};
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

vk::SurfaceKHR Render::createSurface(GLFWwindow * window)
{
	VkSurfaceKHR surface;
	auto result = glfwCreateWindowSurface(instance_, window, nullptr, &surface);
	if(result!=VkResult::VK_SUCCESS)
	{
		throw std::runtime_error("surface create failed");
	}
	
	return surface;
}

vk::PhysicalDevice Render::pickupPhysicalDevice()
{
	
	auto phycial_device = instance_.enumeratePhysicalDevices();
	 for (auto &device :phycial_device)
	 {
		 auto dproperty = device.getProperties();
		 auto features = device.getFeatures();
	 }
	// TODO 对多个显卡进行配置

	return phycial_device[1];
}

vk::Device Render::createDevice()
{
	std::vector<vk::DeviceQueueCreateInfo> queue_infos;

	if(queueIndices_.graphicsIndices.value()==queueIndices_.presentIndices.value())
	{
		vk::DeviceQueueCreateInfo info;
		float priority = 1.0f;
		info.setQueuePriorities(priority);
		info.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());
		queue_infos.push_back(info);
	}else
	{
		vk::DeviceQueueCreateInfo info1;
		float priority = 1.0f;
		info1.setQueuePriorities(priority);
		info1.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());
		

		vk::DeviceQueueCreateInfo info2;
		info2.setQueuePriorities(priority);
		info2.setQueueFamilyIndex(queueIndices_.presentIndices.value());

		queue_infos.push_back(info1);
		queue_infos.push_back(info2);
	}

	std::array<const char*, 1> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo info;

	info.setQueueCreateInfos(queue_infos);
	info.setPEnabledExtensionNames(extensions);
	
	
	return physicalDevice_.createDevice(info);
}

vk::SwapchainKHR Render::createSwapchain()
{
	vk::SwapchainCreateInfoKHR info;
	info.setImageColorSpace(requiredInfo_.format.colorSpace)
		.setImageFormat(requiredInfo_.format.format)
		.setMinImageCount(requiredInfo_.imageCount)
		.setImageExtent(requiredInfo_.extent)
		.setPresentMode(requiredInfo_.presentMode)
		.setPreTransform(requiredInfo_.capabilities.currentTransform);

	if (queueIndices_.graphicsIndices.value()==queueIndices_.presentIndices.value())
	{
		info.setQueueFamilyIndices(queueIndices_.graphicsIndices.value());
		info.setImageSharingMode(vk::SharingMode::eExclusive);
		
	}else
	{
		std::array<uint32_t, 2> indices{ queueIndices_.graphicsIndices.value(),
									queueIndices_.presentIndices.value() };
		info.setQueueFamilyIndices(indices);
		info.setImageSharingMode(vk::SharingMode::eConcurrent);	// 队列不同，并行存储
	}
	info.setClipped(false);
	info.setSurface(surface_);
	info.setImageArrayLayers(1);	// 图层
	info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);	// 不透明
	info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	return device_.createSwapchainKHR(info);
}

std::vector<vk::ImageView> Render::createImageViews()
{
	std::vector<vk::ImageView> views(images_.size());
	for (int i=0;i<views.size();i++)
	{
		vk::ImageViewCreateInfo info;
		info.setImage(images_[i])
			.setFormat(requiredInfo_.format.format)
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

		views[i] = device_.createImageView(info); 
	}

	return views;
}

vk::PipelineLayout Render::createLayout()
{
	vk::PipelineLayoutCreateInfo info;
	return device_.createPipelineLayout(info);
}

vk::RenderPass Render::createRenderPass()
{
	vk::RenderPassCreateInfo info;
	vk::AttachmentDescription attachment_desc;
	attachment_desc.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFormat(requiredInfo_.format.format)
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

	return device_.createRenderPass(info);
}

std::vector<vk::Framebuffer> Render::createFreamebuffers()
{
	// 对于每个imageview 创建framebuffer
	std::vector<vk::Framebuffer> result;

	for (int i=0;i<imageViews_.size();i++)
	{
		vk::FramebufferCreateInfo info;
		info.setRenderPass(renderPass_);
		info.setLayers(1);
		info.setWidth(requiredInfo_.extent.width);
		info.setHeight(requiredInfo_.extent.height);
		info.setAttachments(imageViews_[i]);

		result.push_back(device_.createFramebuffer(info));
	}
	return result;
}

vk::CommandPool Render::createCommandPool()
{
	vk::CommandPoolCreateInfo info;
	info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	info.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());

	return device_.createCommandPool(info);
}

vk::CommandBuffer Render::createCommandBuffer()
{
	vk::CommandBufferAllocateInfo info;
	info.setCommandPool(commandPool_);
	info.setCommandBufferCount(1);
	info.setLevel(vk::CommandBufferLevel::ePrimary);

	//TODO 注意command buffer
	return device_.allocateCommandBuffers(info)[0];
}

void Render::recordCommand(vk::CommandBuffer buffer,vk::Framebuffer framebuffer)
{
	vk::CommandBufferBeginInfo begin_info;
	begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	if(buffer.begin(&begin_info)!=vk::Result::eSuccess)
	{
		throw std::runtime_error("command buffer record failed");
	}
	vk::RenderPassBeginInfo render_pass_begin_info;
	vk::ClearColorValue clear_color(std::array<float,4>{0.1f,0.1f,0.1f,1.f});
	vk::ClearValue value(clear_color);
	render_pass_begin_info.setRenderPass(renderPass_)
		.setRenderArea(vk::Rect2D({ 0,0 }, requiredInfo_.extent))
		.setClearValues(value)
		.setFramebuffer(framebuffer);

	buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

	buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

	vk::DeviceSize size = 0;
	buffer.bindVertexBuffers(0, vertexBuffer_, size);

	buffer.draw(vertices.size(), 1, 0, 0);
	
	buffer.endRenderPass();

	buffer.end();
}

vk::Semaphore Render::createSemaphore()
{
	vk::SemaphoreCreateInfo info;

	return device_.createSemaphore(info);
}

vk::Fence Render::createFence()
{
	vk::FenceCreateInfo info;
	
	return device_.createFence(info);
}

vk::Buffer Render::createBufferDefine(vk::BufferUsageFlags flag)
{
	vk::BufferCreateInfo info;
	info.setSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndices(queueIndices_.graphicsIndices.value())
		.setSize(sizeof(vertices))
		.setUsage(flag);

	return device_.createBuffer(info);
}

vk::DeviceMemory Render::allocateMem(vk::Buffer buffer)
{
	auto requirement = queryBufferMemRequiredInfo(buffer, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::MemoryAllocateInfo info;
	info.setAllocationSize(requirement.size)
		.setMemoryTypeIndex(requirement.index);
	return device_.allocateMemory(info);
}

void Render::render()
{
	device_.resetFences(fence_);

	// acquire a image from swapchain
	auto result = device_.acquireNextImageKHR(swapchain_, std::numeric_limits<uint64_t>::max(),
		imageAvaliableSemaphore_, nullptr);	//Semaphore GPU-GPU fence CPU-GPU
	if(result.result!=vk::Result::eSuccess)
	{
		throw std::runtime_error("acquire image failed");
	}
	uint32_t image_index = result.value;
	commandBuffer_.reset();
	recordCommand(commandBuffer_,frameBuffer_[image_index]);

	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submit_info;
	submit_info.setCommandBuffers(commandBuffer_)
		.setSignalSemaphores(renderFinishSemaphore_)
		.setWaitSemaphores(imageAvaliableSemaphore_)
		.setWaitDstStageMask(flags);
	graphicQueue_.submit(submit_info,fence_);

	vk::PresentInfoKHR present_info;
	present_info.setImageIndices(image_index)
		.setSwapchains(swapchain_)
		.setWaitSemaphores(renderFinishSemaphore_);

	if(presentQueue_.presentKHR(present_info)!=vk::Result::eSuccess)
	{
		throw std::runtime_error("Render present failed");
	};

	if(device_.waitForFences(fence_, true, std::numeric_limits<uint64_t>::max())!=
		vk::Result::eSuccess)
	{
		throw std::runtime_error("Render wait fence failed");
	}
	// sync

	// 
}

void Render::waitIdle()
{
	device_.waitIdle();
}

Render::QueueFamilyIndices Render::queryPhysicalDevice()
{
	auto familes = physicalDevice_.getQueueFamilyProperties();
	Render::QueueFamilyIndices indices;
	uint32_t idx = 0;
	for (auto &family:familes)
	{
		if(family.queueFlags | vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsIndices = idx;
		}

		if(physicalDevice_.getSurfaceSupportKHR(idx,surface_))
		{
			indices.presentIndices = idx;
		}
		if (indices.graphicsIndices && indices.presentIndices) break;
		idx++;
	}
	return indices;
}

void Render::createBuffer()
{
	vertexBuffer_ = createBufferDefine(vk::BufferUsageFlagBits::eVertexBuffer);
	vertexMemory_ = allocateMem(vertexBuffer_);

	CHECK_NULL(vertexBuffer_)
	CHECK_NULL(vertexMemory_)

	device_.bindBufferMemory(vertexBuffer_, vertexMemory_, 0);

	void* data = device_.mapMemory(vertexMemory_, 0, sizeof(vertices));
	memcpy(data, vertices.data(), sizeof(vertices));
	device_.unmapMemory(vertexMemory_);
}

Render::SwapChainRequiredInfo Render::querySwapChainRequiredInfo(int w, int h)
{
	SwapChainRequiredInfo info;
	info.capabilities = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
	auto formats = physicalDevice_.getSurfaceFormatsKHR(surface_);
	info.format = formats[0];
	for (auto &format:formats)
	{
		if (format.format==vk::Format::eR8G8B8A8Srgb ||
			format.format==vk::Format::eB8G8R8A8Srgb)
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
	auto present_mode = physicalDevice_.getSurfacePresentModesKHR(surface_);
	info.presentMode = vk::PresentModeKHR::eFifo;
	for(auto & present:present_mode)
	{
		if (present==vk::PresentModeKHR::eMailbox)
		{
			info.presentMode = present;
		}
	}

	return info;
}

void Render::createTexture()
{
	textureImage_ = createImageDefine(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
	textureMemory_ = allocateMem(textureImage_);

	CHECK_NULL(textureImage_);
	CHECK_NULL(textureMemory_);

	device_.bindImageMemory(textureImage_, textureMemory_,0);
	



}

Render::MemRequiredInfo Render::queryImageMemRequiredInfo(vk::Image image, vk::MemoryPropertyFlags flag)
{
	MemRequiredInfo info;
	auto properties = physicalDevice_.getMemoryProperties();
	auto requirement = device_.getImageMemoryRequirements(image);
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

Render::MemRequiredInfo Render::queryBufferMemRequiredInfo(vk::Buffer buffer, vk::MemoryPropertyFlags flag)
{
	MemRequiredInfo info;
	auto properties = physicalDevice_.getMemoryProperties();
	auto requirement = device_.getBufferMemoryRequirements(buffer);
	info.size = requirement.size;

	for(int i=0;i<properties.memoryTypeCount;i++)
	{
		if((requirement.memoryTypeBits&(1<<i))&&
			(properties.memoryTypes[i].propertyFlags & (flag)))
		{
			info.index = i;
		}
	}
	return info;
}

void Render::quit()
{
	// TODO 按顺序释放成员
	device_.freeMemory(textureMemory_);
	device_.destroyImage(textureImage_);

	device_.freeMemory(vertexMemory_);
	device_.destroyBuffer(vertexBuffer_);

	device_.destroyFence(fence_);
	device_.destroySemaphore(imageAvaliableSemaphore_);
	device_.destroySemaphore(renderFinishSemaphore_);
	device_.freeCommandBuffers(commandPool_,commandBuffer_);
	device_.destroyCommandPool(commandPool_);
	for(auto &framebuffer:frameBuffer_)
	{
		device_.destroyFramebuffer(framebuffer);
	}
	device_.destroyRenderPass(renderPass_);
	device_.destroyPipelineLayout(layout_);
	device_.destroyPipeline(pipeline_);
	for(auto &shader:shaderModules_)
	{
		device_.destroyShaderModule(shader);
	}

	for(auto &view:imageViews_)
	{
		device_.destroyImageView(view);
	}
	device_.destroySwapchainKHR(swapchain_);
	device_.destroy();
	instance_.destroySurfaceKHR(surface_);
	instance_.destroy();
}

void Render::createPipeline(vk::ShaderModule vertex_shader, vk::ShaderModule frag_shader)
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

	//Todo  Pipeline all stage setting

	// vertex input
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

	// layout 
	info.setLayout(layout_);

	//viewport and scissor
	vk::PipelineViewportStateCreateInfo viewport_state;
	vk::Viewport viewport(0, 0,
		requiredInfo_.extent.width, 
		requiredInfo_.extent.height,
		0, 1);
	vk::Rect2D scissor({ 0,0 }, requiredInfo_.extent);
	viewport_state.setViewports(viewport)
		.setScissors(scissor);
	info.setPViewportState(&viewport_state);

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

	// RenderPass
	info.setRenderPass(renderPass_);

	// createPipeline
	auto result = device_.createGraphicsPipeline(nullptr, info);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create pipline");
	}
	pipeline_ = result.value;
	return;
}

vk::ShaderModule Render::createShaderModule(const char* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::in);
	std::vector<char> content((	std::istreambuf_iterator<char>(file)),
								std::istreambuf_iterator<char>());
	file.close();

	vk::ShaderModuleCreateInfo info;
	info.pCode = (uint32_t*)(content.data());
	info.codeSize = content.size();

	shaderModules_.push_back(device_.createShaderModule(info));
	return shaderModules_.back();
}

vk::Image Render::createImageDefine(vk::ImageUsageFlags flag)
{
	vk::Image texture;
	vk::ImageCreateInfo info;
	vk::Extent3D texture_extent;
	vk::DeviceMemory memory;
	texture_extent.setHeight(textureHeight);
	texture_extent.setWidth(textureWidth);
	texture_extent.setDepth(1);

	info.setImageType(vk::ImageType::e2D)
		.setExtent(texture_extent)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(vk::Format::eR8G8B8A8Srgb)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(flag);
		

	return device_.createImage(info);
}

vk::DeviceMemory Render::allocateMem(vk::Image image)
{
	
	auto requirement = queryImageMemRequiredInfo(image, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::MemoryAllocateInfo info;
	info.setAllocationSize(requirement.size)
		.setMemoryTypeIndex(requirement.index);
	return device_.allocateMemory(info);

}
