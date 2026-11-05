#include "BufferArea.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "VulkanInitialize.h"
#include "Pipeline.h"
#include "macros/VkMacro.h"
#include "graphics/Texture.h"
#include "functionLibrary/VulkanFunctionLibrary.h"

#include "debug/SuzzaLog.h"

#include <stdexcept>

namespace suz
{
	BufferArea::BufferArea(
		Pipeline& Pipeline,
		const VkDevice& Device,
		const VkExtent2D& SwapChainExtent,
		const VkRenderPass& RenderPass
	) : m_engineUIDelegate(Engine::GetEngineInstance().GetEngineUIRenderEventDelegate()),
		m_commandContext(Engine::GetVulkanResourceManager().GetCommandContext()),
		m_bufferContext(Engine::GetVulkanResourceManager().GetBufferContext()),
		m_swapChain(Engine::GetVulkanResourceManager().GetSwapChain()),
		m_pipeline(Pipeline), m_device(Device), m_renderPass(RenderPass)
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
		//CreateCommandPool();
		//CreateDepthResources();
		//CreateFrameBuffers();
		//CreateCommandBuffer();
		InitViewportConfig();
		//CreateUniformBuffers();
	}

	void BufferArea::Destroy()
	{
		if (!(*m_isAlive)) return;

		//CleanupBufferArea();

		/*for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
			vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
		}*/

		//vkDestroyCommandPool(m_device, m_commandPool, nullptr);

		*m_isAlive = false;
	}

	void BufferArea::RegisterBindlessDescriptorSet(const VkDescriptorSet& TargetDescriptorSet)
	{
		m_bindlessDescriptorSet = TargetDescriptorSet;
	}

	//void BufferArea::CreateUniformBuffers()
	//{
	//	VkDeviceSize bufferSize = sizeof(CameraViewportInfo);

	//	/* 只创建缓冲区并关联内存, 不正式更改顶点属性 */
	//	m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	//	m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	//	m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	//	for (size_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
	//	{
	//		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[frameIndex], m_uniformBuffersMemory[frameIndex]);
	//		/* 只进行映射而不释放, 称为持久映射 */
	//		/* 缓冲内存作为桥梁链接显存和CPU内存 */
	//		vkMapMemory(m_device, m_uniformBuffersMemory[frameIndex], 0, bufferSize, 0, &m_uniformBuffersMapped[frameIndex]);
	//	}
	//}

	void BufferArea::RecordCommandBuffer(const VkCommandBuffer& CommandBuffer, const VkDescriptorSet& CameraDescriptor, const DrawList& DrawQueue, uint32_t ImageIndex)
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
		renderPassInfo.framebuffer = m_swapChain->GetFrameBuffer(ImageIndex).GetFrameBuffer();	// 绑定帧缓冲

		VkExtent2D extent = m_swapChain->GetSwapChainExtent();
		renderPassInfo.renderArea.offset = { 0,0 };										// 定义加载原点 (加载矩形左上角)
		renderPassInfo.renderArea.extent = extent;
		/* 配置背景颜色 */
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };								// 设置颜色清除值 (背景颜色)
		clearValues[1].depthStencil = { 1.0f,0 };										// 深度 + 模板清除值
		renderPassInfo.clearValueCount = static_cast<uint32>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		/* 将命令缓冲区和渲染通道绑定 */
		vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		/* 绑定图形管线 */
		vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.GetGraphicsPipeline());

		m_viewConfigInfo.viewport.width = static_cast<float>(extent.width);
		m_viewConfigInfo.viewport.height = static_cast<float>(extent.height);

		m_viewConfigInfo.scissor.extent = { extent.width,extent.height };

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

		for (const auto& [mesh, modelConstants] : DrawQueue.GetDrawQueue())
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

		/* 执行引擎全局广播委托 */
		m_engineUIDelegate.Execute(CommandBuffer);

		/* 结束绘制命令 */
		vkCmdEndRenderPass(CommandBuffer);

		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("结束记录命令缓冲区失败!");
		}
	}

	//void BufferArea::ReCreateBufferArea(const VkExtent2D& NewSwapChainExtent)
	//{
	//	m_swapChainExtent = NewSwapChainExtent;								// 重置分辨率
	//	CreateDepthResources();
	//	CreateFrameBuffers();
	//}

	//void BufferArea::CleanupBufferArea()
	//{
	//	vkDestroyImageView(m_device, m_depthImageView, nullptr);
	//	vkFreeMemory(m_device, m_depthImageMemory, nullptr);
	//	vkDestroyImage(m_device, m_depthImage, nullptr);

	//	for (const VkFramebuffer& framebuffer : m_frameBuffers)
	//	{
	//		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	//	}
	//}

	//void BufferArea::CreateFrameBuffers()
	//{
	//	VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
	//	const std::vector<VkImageView>& swapChainImageViews = vkManager.GetSwapChain()->GetSwapChainImageViews();
	//	/* 创建图像视图的帧缓冲 */
	//	m_frameBuffers.resize(swapChainImageViews.size());

	//	for (size_t currentImageView = 0; currentImageView < swapChainImageViews.size(); currentImageView++)
	//	{
	//		std::array<VkImageView, 2> attachments = { swapChainImageViews[currentImageView],m_depthImageView };

	//		/* 遍历图像视图创建帧缓冲 */
	//		VkFramebufferCreateInfo framebufferInfo{};
	//		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//		framebufferInfo.renderPass = m_renderPass;
	//		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	//		framebufferInfo.pAttachments = attachments.data();								// 每个帧缓冲绑定一个交换链图像
	//		framebufferInfo.width = m_swapChainExtent.width;
	//		framebufferInfo.height = m_swapChainExtent.height;
	//		framebufferInfo.layers = 1;

	//		if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_frameBuffers[currentImageView]) != VK_SUCCESS)
	//		{
	//			throw std::runtime_error("创建帧缓冲失败!");
	//		}
	//	}
	//}

	//void BufferArea::CreateDepthResources()
	//{
	//	/* 查询硬件支持的最佳深度格式 */
	//	VkFormat depthFormat = VulkanFunctionLibrary::FindDepthFormat(
	//		Engine::GetVulkanResourceManager().GetDeviceContext().physicalDevice
	//	);

	//	/* 创建深度缓冲图像并分配设备本地显存 */
	//	Texture::CreateImage(
	//		m_swapChainExtent.width, m_swapChainExtent.height,
	//		depthFormat,
	//		VK_IMAGE_TILING_OPTIMAL,						// 最优平铺 GPU访问性能最高
	//		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,	// 用途: 作为深度/模板渲染附件
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,			// 类型: GPU本地显存
	//		m_depthImage,
	//		m_depthImageMemory
	//	);

	//	/* 创建深度图像视图, 只可见深度面 */
	//	m_depthImageView = Texture::CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	//	/* 将图像从未定义转换为 "深度附件最优" 布局 */
	//	/* 未定义布局: 新建图像无有效数据, 转换时不保留旧内容, 性能最优 */
	//	m_commandContext->TransitionImageLayout(m_depthImage, depthFormat,
	//		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//	);
	//}

	void BufferArea::InitViewportConfig()
	{
		VkExtent2D extent = m_swapChain->GetSwapChainExtent();

		/* 配置视口信息, 定义管线坐标映射到图像的输出坐标 */
		m_viewConfigInfo.viewport.x = 0.0f;
		m_viewConfigInfo.viewport.y = 0.0f;
		/* 定义视口宽度和高度 */
		m_viewConfigInfo.viewport.width = static_cast<float>(extent.width);
		m_viewConfigInfo.viewport.height = static_cast<float>(extent.height);
		m_viewConfigInfo.viewport.minDepth = 0.0f;
		m_viewConfigInfo.viewport.maxDepth = 1.0f;

		/* 定义裁剪矩形, 在窗口裁剪出extent大小 */
		m_viewConfigInfo.scissor.offset = { 0,0 };
		m_viewConfigInfo.scissor.extent = { extent.width,extent.height };
	}
}