#include "BufferArea.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "VulkanInitialize.h"
#include "Pipeline.h"
#include "macros/VkMacro.h"
#include "graphics/Texture.h"

#include "debug/SuzzaLog.h"

#include <stdexcept>

namespace suz
{
	BufferArea::BufferArea(
		Pipeline& Pipeline,
		const VkPhysicalDevice& PhysicalDevice,
		const VkDevice& Device,
		const VkSurfaceKHR& Surface,
		const VkExtent2D& SwapChainExtent,
		const VkRenderPass& RenderPass
	) : m_pipeline(Pipeline), m_physicalDevice(PhysicalDevice), m_device(Device), m_surface(Surface), m_swapChainExtent(SwapChainExtent), m_renderPass(RenderPass)
	{
	}

	BufferArea::~BufferArea()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class BufferArea Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void BufferArea::InitBufferArea()
	{
		CreateCommandPool();
		CreateDepthResources();
		CreateFrameBuffers();
		CreateCommandBuffer();
		InitViewportConfig();
		CreateUniformBuffers();
	}

	void BufferArea::Destroy()
	{
		if (!(*m_isAlive)) return;

		CleanupBufferArea();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
			vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
		}

		vkDestroyCommandPool(m_device, m_commandPool, nullptr);

		*m_isAlive = false;
	}

	void BufferArea::RegisterBindlessDescriptorSet(const VkDescriptorSet& TargetDescriptorSet)
	{
		m_bindlessDescriptorSet = TargetDescriptorSet;
	}

	void BufferArea::CreateBuffer(const VkDeviceSize& Buffersize, const VkBufferUsageFlags& Usage, const VkMemoryPropertyFlags& Properties, VkBuffer& BufferToCreate, VkDeviceMemory& AllocatedMemory)
	{
		const VkDevice& device = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetDevice();

		/* 1. 定义顶点缓冲区信息 */
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = Buffersize;
		bufferInfo.usage = Usage;											// 定义缓冲的数据将要(顶点缓冲或片段缓冲等)
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;					// 只有特定队列族独占

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &BufferToCreate) != VK_SUCCESS)
		{
			throw std::runtime_error("创建顶点缓冲区失败!");
		}

		/* 查询内存需求 */
		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(device, BufferToCreate, &memoryRequirements);

		/* 2. 定义内存分配信息 */
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, Properties);

		if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &AllocatedMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("分配顶点缓冲区内存失败!");
		}

		/* 3. 将分配的内存与顶点缓冲关联起来 */
		vkBindBufferMemory(device, BufferToCreate, AllocatedMemory, 0);
	}

	void BufferArea::CreateVertexBuffer(const std::vector<Vertex>& Vertices, VkBuffer& VertexBuffer, VkDeviceMemory& VertexBufferMemory)
	{
		const VkDeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();

		/* 创建主机可见的缓冲作为临时缓冲 */
		VkBuffer stagingBuffer = {};
		VkDeviceMemory stagingMemory = {};
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

		/* 将缓冲内存映射到CPU可访问的内存 */
		void* data;
		vkMapMemory(m_device, stagingMemory, 0, bufferSize, 0, &data);
		/* 拷贝数据到映射的内存 */
		memcpy(data, Vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device, stagingMemory);

		/* 创建设备本地内存的缓冲区(内存类型最佳且无法直接访问) */
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

		/* 复制缓冲区 */
		CopyBuffer(stagingBuffer, VertexBuffer, bufferSize);

		/* 销毁旧的临时缓冲区 */
		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingMemory, nullptr);
	}

	uint32 BufferArea::CreateIndexBuffer(const std::vector<uint32>& Indices, VkBuffer& IndexBuffer, VkDeviceMemory& IndexBufferMemory)
	{
		const VkDeviceSize bufferSize = sizeof(Indices[0]) * Indices.size();
		uint32 indexBufferSize = static_cast<uint32>(Indices.size());

		/* 创建主机可见的缓冲作为临时缓冲 */
		VkBuffer stagingBuffer = {};
		VkDeviceMemory stagingMemory = {};
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

		/* 将缓冲内存映射到CPU可访问的内存 */
		/* 缓冲内存作为桥梁链接显存和CPU内存 */
		void* data;
		vkMapMemory(m_device, stagingMemory, 0, bufferSize, 0, &data);
		/* 拷贝数据到映射的内存 */
		memcpy(data, Indices.data(), (usize)bufferSize);
		vkUnmapMemory(m_device, stagingMemory);

		/* 创建设备本地内存的缓冲区(内存类型最佳且无法直接访问) */
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

		/* 复制缓冲区 */
		CopyBuffer(stagingBuffer, IndexBuffer, bufferSize);

		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingMemory, nullptr);

		return indexBufferSize;
	}

	void BufferArea::CopyBufferToImage(const VkBuffer& SrcBuffer, const VkImage& DstImage, uint32 Width, uint32 Height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy copyRegion{};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;

		/* 指定目标图像需要复制的部分 */
		copyRegion.imageOffset = { 0,0,0 };
		copyRegion.imageExtent = { Width,Height,1 };

		vkCmdCopyBufferToImage(commandBuffer, SrcBuffer, DstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void BufferArea::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(CameraViewportInfo);

		/* 只创建缓冲区并关联内存, 不正式更改顶点属性 */
		m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[frameIndex], m_uniformBuffersMemory[frameIndex]);
			/* 只进行映射而不释放, 称为持久映射 */
			/* 缓冲内存作为桥梁链接显存和CPU内存 */
			vkMapMemory(m_device, m_uniformBuffersMemory[frameIndex], 0, bufferSize, 0, &m_uniformBuffersMapped[frameIndex]);
		}
	}

	void BufferArea::RecordCommandBuffer(const VkCommandBuffer& CommandBuffer, const VkDescriptorSet& CameraDescriptor, const std::unordered_map<Mesh*, std::vector<ModelConstant>>& MeshToDraw, uint32_t ImageIndex)
	{
		VkCommandBufferBeginInfo bufferBeginInfo{};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.flags = 0;
		bufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(CommandBuffer, &bufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("开始记录命令缓冲区失败!");
		}

		/* 配置渲染通道信息 */
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_frameBuffers[ImageIndex];				// 绑定帧缓冲

		renderPassInfo.renderArea.offset = { 0,0 };								// 定义加载原点 (加载矩形左上角)
		renderPassInfo.renderArea.extent = m_swapChainExtent;
		/* 配置背景颜色 */
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f,1.0f,1.0f,1.0f} };						// 设置颜色清除值 (背景颜色)
		clearValues[1].depthStencil = { 1.0f,0 };								// 深度 + 模板清除值
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		/* 将命令缓冲区和渲染通道绑定 */
		vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		/* 绑定图形管线 */
		vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.GetGraphicsPipeline());

		m_viewConfigInfo.viewport.width = static_cast<float>(m_swapChainExtent.width);
		m_viewConfigInfo.viewport.height = static_cast<float>(m_swapChainExtent.height);

		m_viewConfigInfo.scissor.extent = { m_swapChainExtent.width,m_swapChainExtent.height };

		/* 设置图形视口和裁剪矩形区域 */
		vkCmdSetViewport(CommandBuffer, 0, 1, &m_viewConfigInfo.viewport);
		vkCmdSetScissor(CommandBuffer, 0, 1, &m_viewConfigInfo.scissor);

		std::array<VkDescriptorSet, 2> descriptorSets = { CameraDescriptor,m_bindlessDescriptorSet };

		/* 绑定描述符集合, 并从映射内存读取数据 */
		/* 绑定摄像机描述符和Bindless描述符 */
		vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.GetPipelineLayout(),
			0, static_cast<uint32>(descriptorSets.size()), &descriptorSets[0],
			0, nullptr
		);

		for (const auto& [mesh, modelConstants] : MeshToDraw)
		{
			/* 绑定顶点缓冲 */
			VkDeviceSize offsets[] = { 0 };											// 定义读取顶点数据的字节偏移量
			vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &mesh->GetMeshVertexBuffer(), offsets);
			if (mesh->GetMeshIndexBuffer() != VK_NULL_HANDLE)
			{
				vkCmdBindIndexBuffer(CommandBuffer, mesh->GetMeshIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			}

			for (const ModelConstant& modelConstant : modelConstants)
			{
				vkCmdPushConstants(CommandBuffer, m_pipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelConstant), &modelConstant);

				/* 发出绘制命令 */
				/* 告诉Vulkan使用索引缓冲区 */
				vkCmdDrawIndexed(CommandBuffer, mesh->GetIndexBufferSize(), 1, 0, 0, 0);
			}
		}

		/* 结束绘制命令 */
		vkCmdEndRenderPass(CommandBuffer);

		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("结束记录命令缓冲区失败!");
		}
	}

	void BufferArea::ReCreateBufferArea(const VkExtent2D& NewSwapChainExtent)
	{
		m_swapChainExtent = NewSwapChainExtent;								// 重置分辨率
		CreateDepthResources();
		CreateFrameBuffers();
	}

	void BufferArea::CleanupBufferArea()
	{
		vkDestroyImageView(m_device, m_depthImageView, nullptr);
		vkFreeMemory(m_device, m_depthImageMemory, nullptr);
		vkDestroyImage(m_device, m_depthImage, nullptr);

		for (const VkFramebuffer& framebuffer : m_frameBuffers)
		{
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}
	}

	VkFormat BufferArea::FindDepthFormat()
	{
		return FindSupportFormat(
			{ VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,								// 最优平铺 GPU访问性能最高
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	uint32 BufferArea::FindMemoryType(uint32 TypeFilter, const VkMemoryPropertyFlags& Properties)
	{
		const VkPhysicalDevice& physicalDevice = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetPhysicalDevice();
		/* 查询相关可用内存的信息 */
		VkPhysicalDeviceMemoryProperties physicalDeviceMemory{};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemory);

		/* 查询合适的内存索引 可能存在多个期望的属性因此按位与运算比较 */
		for (usize typeIndex = 0; typeIndex < physicalDeviceMemory.memoryTypeCount; typeIndex++)
		{
			if (TypeFilter & (1 << typeIndex) && (physicalDeviceMemory.memoryTypes[typeIndex].propertyFlags & Properties) == Properties)
			{
				return static_cast<uint32_t>(typeIndex);
			}
		}

		throw std::runtime_error("无法查找到合适的内存类型!");
	}

	VkFormat BufferArea::FindSupportFormat(const std::vector<VkFormat>& Candidates, const VkImageTiling& Tiling, const VkFormatFeatureFlags& Features)
	{
		const VkPhysicalDevice& physicalDevice = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetPhysicalDevice();

		for (const VkFormat& format : Candidates)
		{
			/* 获取格式的支持情况 */
			VkFormatProperties formatProps{};
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

			/* 查找设备是否包含全部所需的功能 */
			if (Tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures & Features) == Features)
			{
				return format;
			}
			else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & Features) == Features)
			{
				return format;
			}
		}

		throw std::runtime_error("无法找到受支持的格式!");
	}

	void BufferArea::TransitionImageLayout(const VkImage& Image, const VkFormat& Format, const VkImageLayout& OldLayout, const VkImageLayout& NewLayout)
	{
		/* 开启一次性命令缓冲 */
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		/* 图像内存屏障 */
		/* 1. 执行布局转换 2. 保证前后管线阶段的内存可见性 3. 同步资源访问 */
		VkImageMemoryBarrier memoryBarrier{};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memoryBarrier.oldLayout = OldLayout;
		memoryBarrier.newLayout = NewLayout;
		memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;	// 不做队列族转移
		memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.image = Image;									// 目标图像

		/* 子资源范围: 指定要转换的图像区域 */
		memoryBarrier.subresourceRange.baseMipLevel = 0;
		memoryBarrier.subresourceRange.levelCount = 1;
		memoryBarrier.subresourceRange.baseArrayLayer = 0;
		memoryBarrier.subresourceRange.layerCount = 1;

		/* 源/目标阶段: 访问掩码 */
		VkPipelineStageFlags sourceStage{};								// 屏障前的管线阶段
		VkPipelineStageFlags destinationStage{};						// 屏障后的管线阶段

		/* 场景: 从未定义布局 -> 深度/模板附件最优布局 */
		if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			memoryBarrier.srcAccessMask = 0;
			memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// 屏障提交的命令在管线最开始完成
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			memoryBarrier.srcAccessMask = 0;							// 源端无访问权限, 旧数据直接丢弃
			memoryBarrier.dstAccessMask =								// 目标端 深度附件设置读和写权限
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// 屏障提交的命令在管线最开始完成
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;// 管线到达早期片段测试阶段时调用屏障
		}
		else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("不受支持的图像布局转换!");
		}

		/* 设置aspect掩码 */
		/* 深度布局: 可见面为深度面, 如果格式含模板, 额外加上模板面 */
		if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;	// 可见面为深度面
			if (HasStencilComponent(Format))
			{
				memoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;// 额外加上模板面
			}
		}
		else
		{
			memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		/* 提交管线屏障 */
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,							// 无内存屏障
			0, nullptr,							// 无缓冲屏障
			1, &memoryBarrier					// 1个图像屏障
		);

		/* 结束命令缓冲并提交执行 */
		EndSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer BufferArea::BeginSingleTimeCommands()
	{
		/* 命令缓冲区分配 */
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = m_commandPool;
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer{};
		vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer);

		/* 定义命令缓冲信息 */
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		/* 开始记录命令缓冲 */
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void BufferArea::EndSingleTimeCommands(const VkCommandBuffer& CommandBuffer)
	{
		vkEndCommandBuffer(CommandBuffer);

		/* 执行命令缓冲提交 */
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		VkQueue graphicsQueue = vkManager.GetVulkanInitialize()->GetGraphicsQueue();
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		/* 清理临时命令缓冲 */
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &CommandBuffer);
	}

	void BufferArea::CreateFrameBuffers()
	{
		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		const std::vector<VkImageView>& swapChainImageViews = vkManager.GetSwapChain()->GetSwapChainImageViews();
		/* 创建图像视图的帧缓冲 */
		m_frameBuffers.resize(swapChainImageViews.size());

		for (size_t currentImageView = 0; currentImageView < swapChainImageViews.size(); currentImageView++)
		{
			std::array<VkImageView, 2> attachments = { swapChainImageViews[currentImageView],m_depthImageView };

			/* 遍历图像视图创建帧缓冲 */
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();								// 每个帧缓冲绑定一个交换链图像
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_frameBuffers[currentImageView]) != VK_SUCCESS)
			{
				throw std::runtime_error("创建帧缓冲失败!");
			}
		}
	}

	void BufferArea::CreateCommandPool()
	{
		QueueFamilyIndices indices = VulkanInitialize::FindQueueFamilies(m_physicalDevice, m_surface);

		/* 创建命令缓冲区用于存储缓冲区的内存并用于分配缓冲区 */
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;			// 允许单独重新记录命令缓冲区
		commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();

		if (vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("创建命令池失败!");
		}
	}

	void BufferArea::CreateDepthResources()
	{
		/* 查询硬件支持的最佳深度格式 */
		VkFormat depthFormat = FindDepthFormat();

		/* 创建深度缓冲图像并分配设备本地显存 */
		Texture::CreateImage(
			m_swapChainExtent.width, m_swapChainExtent.height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,						// 最优平铺 GPU访问性能最高
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,	// 用途: 作为深度/模板渲染附件
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,			// 类型: GPU本地显存
			m_depthImage,
			m_depthImageMemory
		);

		/* 创建深度图像视图, 只可见深度面 */
		m_depthImageView = Texture::CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		/* 将图像从未定义转换为 "深度附件最优" 布局 */
		/* 未定义布局: 新建图像无有效数据, 转换时不保留旧内容, 性能最优 */
		TransitionImageLayout(m_depthImage, depthFormat,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
	}

	void BufferArea::CreateCommandBuffer()
	{
		m_commandBuffer.resize(MAX_FRAMES_IN_FLIGHT);

		/* 命令缓冲区分配 */
		VkCommandBufferAllocateInfo bufferAllocateInfo{};
		bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocateInfo.commandPool = m_commandPool;										// 指定命令池
		bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;							// 设为主缓冲区
		bufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffer.size());

		if (vkAllocateCommandBuffers(m_device, &bufferAllocateInfo, m_commandBuffer.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("分配命令缓冲区失败!");
		}
	}

	void BufferArea::InitViewportConfig()
	{
		/* 配置视口信息, 定义管线坐标映射到图像的输出坐标 */
		m_viewConfigInfo.viewport.x = 0.0f;
		m_viewConfigInfo.viewport.y = 0.0f;
		/* 定义视口宽度和高度 */
		m_viewConfigInfo.viewport.width = static_cast<float>(m_swapChainExtent.width);
		m_viewConfigInfo.viewport.height = static_cast<float>(m_swapChainExtent.height);
		m_viewConfigInfo.viewport.minDepth = 0.0f;
		m_viewConfigInfo.viewport.maxDepth = 1.0f;

		/* 定义裁剪矩形, 在窗口裁剪出extent大小 */
		m_viewConfigInfo.scissor.offset = { 0,0 };
		m_viewConfigInfo.scissor.extent = { m_swapChainExtent.width,m_swapChainExtent.height };
	}

	void BufferArea::CopyBuffer(const VkBuffer& SrcBuffer, const VkBuffer& DstBuffer, const VkDeviceSize& BufferSize)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy bufferCopy{};
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = BufferSize;
		vkCmdCopyBuffer(commandBuffer, SrcBuffer, DstBuffer, 1, &bufferCopy);

		EndSingleTimeCommands(commandBuffer);
	}

	bool BufferArea::HasStencilComponent(const VkFormat& DepthFormat)
	{
		return DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}