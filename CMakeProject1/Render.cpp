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
vk::Queue	Render::computeQueue_ = nullptr;
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
vk::DescriptorSetLayout Render::descriptorSetLayout_ = nullptr;
vk::PipelineLayout Render::pipelineLayout_ = nullptr;
vk::DescriptorPool Render::descriptorPool_ = nullptr;
vk::Pipeline Render::computerPipeline_ = nullptr;
std::vector<vk::DescriptorSet> Render::descriptorSet_;
Render::Computer Render::computer_;

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

	commandPool_ = createCommandPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,queueIndices_.graphicsIndices.value());
	CHECK_NULL(commandPool_)

	commandBuffer_ = createCommandBuffer(commandPool_);
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

vk::CommandPool Render::createCommandPool(vk::CommandPoolCreateFlagBits flags,uint32_t index)
{
	vk::CommandPoolCreateInfo info;
	info.setFlags(flags);
	info.setQueueFamilyIndex(index);

	return device_.createCommandPool(info);
}

vk::CommandBuffer Render::createCommandBuffer(vk::CommandPool command_pool)
{
	vk::CommandBufferAllocateInfo info;
	info.setCommandPool(command_pool);
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

vk::Buffer Render::createVertexBufferDefine(vk::BufferUsageFlags flag)
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
	// Computer Shader
	//vk::Result status
	auto status = device_.waitForFences(1, &computer_.fence, true, 100);
	if (status != vk::Result::eSuccess) throw std::runtime_error("failed to wait computer shader");
	device_.resetFences(computer_.fence);

	vk::SubmitInfo computer_submit_info;
	computer_submit_info.setCommandBuffers(computer_.commandBuffer)
		.setCommandBufferCount(1);
	status = computeQueue_.submit(1, &computer_submit_info, computer_.fence);
	if (status != vk::Result::eSuccess) throw std::runtime_error("failed to submit command");

	// draw 
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
		if(family.queueFlags | vk::QueueFlagBits::eCompute)
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

void Render::createBuffer()
{
	vertexBuffer_ = createVertexBufferDefine(vk::BufferUsageFlagBits::eVertexBuffer);
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
	texture_.image = createImageDefine(vk::ImageUsageFlagBits::eTransferDst | 
		vk::ImageUsageFlagBits::eSampled |
		vk::ImageUsageFlagBits::eStorage);
	texture_.memory = allocateMem(texture_.image);
	
	CHECK_NULL(texture_.image);
	CHECK_NULL(texture_.memory);
	
	device_.bindImageMemory(texture_.image, texture_.memory,0);
	
	
	vk::CommandBuffer layout_cmd = createCommandBuffer(commandPool_);

	vk::CommandBufferBeginInfo begin_info;
	begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	
	if (layout_cmd.begin(&begin_info) != vk::Result::eSuccess)
	{
		throw std::runtime_error("command buffer record failed");
	}
	texture_.layout = vk::ImageLayout::eGeneral;
	setImageLayout(layout_cmd,texture_.image,vk::ImageAspectFlagBits::eColor,
		vk::ImageLayout::eUndefined,
		texture_.layout);
	
	flushCommandBuffer(layout_cmd);



	// sampler
	texture_.sampler = createTextureSampler();
	// image view
	texture_.imageView = createTextureViewImage(texture_.image);

	texture_.descriptor.setImageLayout(texture_.layout);
	texture_.descriptor.setImageView(texture_.imageView);
	texture_.descriptor.setSampler(texture_.sampler);

}

vk::Sampler Render::createTextureSampler()
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

	return device_.createSampler(info);
}

vk::ImageView Render::createTextureViewImage(vk::Image image)
{
	vk::ImageViewCreateInfo info;
	info.setImage(image)
		.setFormat(vk::Format::eR32G32B32A32Sfloat)
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

	return device_.createImageView(info);
}

void Render::setImageLayout(vk::CommandBuffer cmd_buffer, vk::Image image,
	vk::ImageAspectFlags flags, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
{
	auto stage_mask = vk::PipelineStageFlagBits::eAllCommands;
	vk::ImageSubresourceRange subsource_range{};
	subsource_range.setAspectMask(flags)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setLayerCount(1);

	// TODO set image layput
	auto image_memory_barrier = createImageMemoryBarrier();
	image_memory_barrier.setOldLayout(old_layout);
	image_memory_barrier.setNewLayout(new_layout);
	image_memory_barrier.setImage(image);
	image_memory_barrier.setSubresourceRange(subsource_range);

	image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);

	//TODO update dependency Flag
	cmd_buffer.pipelineBarrier(stage_mask,stage_mask ,vk::DependencyFlagBits::eByRegion,
		0,nullptr,0,nullptr,1,&image_memory_barrier);
	return;
}

vk::ImageMemoryBarrier Render::createImageMemoryBarrier()
{
	vk::ImageMemoryBarrier image_memory_barrier;
	image_memory_barrier.setSrcQueueFamilyIndex(queueIndices_.graphicsIndices.value());
	image_memory_barrier.setDstQueueFamilyIndex(queueIndices_.graphicsIndices.value());
	return image_memory_barrier;
}

void Render::flushCommandBuffer(vk::CommandBuffer cmd_buffer)
{
	cmd_buffer.end();

	vk::SubmitInfo submit_info;
	submit_info.setCommandBufferCount(1)
		.setPCommandBuffers(&cmd_buffer);

	vk::FenceCreateInfo fence_info;
	
	vk::Fence fence = device_.createFence(fence_info);
	CHECK_NULL(fence)

	
	auto result = graphicQueue_.submit(1,&submit_info, fence);
	
	if (result != vk::Result::eSuccess) {
		throw std::runtime_error("faied to commit command in texture");
	}
	result = device_.waitForFences(fence, true, 100000000000);
	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait texture fence");
	}
	device_.destroyFence(fence);
	device_.freeCommandBuffers(commandPool_,cmd_buffer);
	return;
}

void Render::createComputerPipeline(vk::ShaderModule computerShader)
{

	vk::PipelineShaderStageCreateInfo shader_stage;
	shader_stage.setModule(computerShader)
		.setStage(vk::ShaderStageFlagBits::eCompute)
		.setPName("main");

	descriptorPool_ = createDescriptorPool();

	// set pipeline layout 
	computeQueue_ = device_.getQueue(queueIndices_.computerIndices.value(), 0);
	CHECK_NULL(computeQueue_)
	descriptorSetLayout_ = createDescriptorSetLayout();
	CHECK_NULL(descriptorSetLayout_)

	pipelineLayout_ = createPipelineLayout();
	CHECK_NULL(pipelineLayout_)

	descriptorSet_ = createDescriptorSet();

	// fill with Shader
	vk::ComputePipelineCreateInfo info;
	info.setLayout(pipelineLayout_)
		.setStage(shader_stage);

	auto result = device_.createComputePipeline(nullptr, info);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed create computer pipeline");
	}
	computerPipeline_ = result.value;

	computer_.commandPool = createCommandPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueIndices_.computerIndices.value());
	CHECK_NULL(computer_.commandPool)

	computer_.commandBuffer = createCommandBuffer(computer_.commandPool);
	CHECK_NULL(computer_.commandBuffer)

	computer_.fence = createFence();
	CHECK_NULL(computer_.fence)


}

vk::DescriptorSetLayout Render::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutCreateInfo info;
	
	std::array<vk::DescriptorSetLayoutBinding, 1> layout_bindings = {
		setLayoutBinding(vk::DescriptorType::eStorageImage,vk::ShaderStageFlagBits::eCompute,0)	// binding=0 image
	};

	info.setPBindings(layout_bindings.data())
		.setBindingCount(layout_bindings.size());
	return device_.createDescriptorSetLayout(info);
}

vk::DescriptorSetLayoutBinding Render::setLayoutBinding(vk::DescriptorType type,vk::ShaderStageFlagBits flags,
	uint32_t binding,uint32_t descriptorCount)
{
	vk::DescriptorSetLayoutBinding layout_set;
	layout_set.setBinding(binding)
		.setDescriptorType(type)
		.setDescriptorCount(descriptorCount)
		.setStageFlags(flags);

	return layout_set;
}
// texture
vk::PipelineLayout Render::createPipelineLayout()
{
	vk::PipelineLayoutCreateInfo  info;
	info.setPSetLayouts(&descriptorSetLayout_)
		.setSetLayoutCount(1);
	return device_.createPipelineLayout(info);
}

vk::DescriptorPool Render::createDescriptorPool()
{
	auto createDescriptorSize = [](vk::DescriptorType type, uint32_t num)
	{
		vk::DescriptorPoolSize pool_size;
		pool_size.setType(type)
			.setDescriptorCount(num);
		return pool_size;
	};
	std::array<vk::DescriptorPoolSize,1> pool_size{
		createDescriptorSize(vk::DescriptorType::eStorageImage,1)
	};
	vk::DescriptorPoolCreateInfo descriptor_pool_info;
	// TODO update max sets 
	descriptor_pool_info.setPoolSizeCount(pool_size.size())
		.setPPoolSizes(pool_size.data())
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(1);

	return device_.createDescriptorPool(descriptor_pool_info);
}

std::vector<vk::DescriptorSet> Render::createDescriptorSet()
{
	
	vk::DescriptorSetAllocateInfo info;
	info.setDescriptorPool(descriptorPool_)
		.setSetLayouts(descriptorSetLayout_)
		.setDescriptorSetCount(1);


	auto result = device_.allocateDescriptorSets(info);

	std::array<vk::WriteDescriptorSet, 1> write_descriptorset{
		createWriteDescriptorSet(result[0],vk::DescriptorType::eStorageImage ,0,texture_.descriptor)
	};

	device_.updateDescriptorSets(write_descriptorset.size(), 
		write_descriptorset.data(), 0, nullptr);
	return result;
}

vk::WriteDescriptorSet Render::createWriteDescriptorSet(vk::DescriptorSet descriptor_set, vk::DescriptorType type, uint32_t binding, vk::DescriptorImageInfo image_info)
{
	vk::WriteDescriptorSet info;
	info.setDstSet(descriptor_set)
		.setDescriptorType(type)
		.setDstBinding(binding)
		.setPImageInfo(&image_info)
		.setDescriptorCount(1);

	return info;

}

void Render::recordRayTraceCommand()
{
	vk::CommandBufferBeginInfo info;
	info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	if(computer_.commandBuffer.begin(&info)!=vk::Result::eSuccess)
	{
		throw std::runtime_error("failed record Ray Trace command");
	}
	// bind commmand buffer with pipeline
	computer_.commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computerPipeline_);
	computer_.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout_, 
		0, descriptorSet_.size(), descriptorSet_.data(),0,0);
	// TODO update 
	computer_.commandBuffer.dispatch(textureWidth / 16, textureHeight / 16, 1);

	computer_.commandBuffer.end();
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
	// TODO release resource
	
	device_.freeCommandBuffers(computer_.commandPool,computer_.commandBuffer);
	device_.destroyCommandPool(computer_.commandPool);
	device_.destroyFence(computer_.fence);
	device_.destroy(computerPipeline_);
	for (auto &descriptor_set:descriptorSet_)
	{
		device_.freeDescriptorSets(descriptorPool_, descriptor_set);
	}
	device_.destroyDescriptorPool(descriptorPool_);
	device_.destroyPipelineLayout(pipelineLayout_);
	device_.destroyDescriptorSetLayout(descriptorSetLayout_);
	device_.destroySampler(texture_.sampler);
	device_.destroyImageView(texture_.imageView);
	device_.freeMemory(texture_.memory);
	device_.destroyImage(texture_.image);

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
		.setFormat(vk::Format::eR32G32B32A32Sfloat)
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
