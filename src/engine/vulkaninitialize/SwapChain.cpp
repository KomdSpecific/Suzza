#include "SwapChain.h"
#include "Engine.h"
#include "VulkanResourceManager.h"
#include "MainWindow.h"
#include "graphics/Texture.h"
#include "factory/SwapChainFactories.h"
#include "functionLibrary/VulkanFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkSwapchainCreateInfoKHR SwapChainSpec::DefaultSwapChainCreateInfoKHR(const SwapChainSpec& Spec)
	{
		uint32 minImageCount = Spec.swapChainDetails.capabilities.minImageCount + 1;
		if (Spec.swapChainDetails.capabilities.maxImageCount > 0)
		{
			/* 保证不超过图像的最大数量 */
			minImageCount = std::min(minImageCount, Spec.swapChainDetails.capabilities.maxImageCount);
		}
		/* 保证未来数据改动导致 max < min */
		minImageCount = std::max(minImageCount, Spec.swapChainDetails.capabilities.minImageCount);

		/* 创建交换链对象 */
		VkSwapchainCreateInfoKHR swapChainInfo{};
		swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainInfo.surface = Spec.surface;
		/* 指定交换链图像的详细信息 */
		swapChainInfo.minImageCount = minImageCount;
		swapChainInfo.imageFormat = Spec.surfaceFormat.format;
		swapChainInfo.imageColorSpace = Spec.surfaceFormat.colorSpace;
		swapChainInfo.imageExtent = Spec.swapChainExtent;
		swapChainInfo.imageArrayLayers = 1;										// 指定图像由多少层组成
		swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;			// 指定交换链中图像的操作类型

		swapChainInfo.preTransform = Spec.swapChainDetails.capabilities.currentTransform;
		/* 指定alpha通道是否与窗口系统中的其他窗口混合 */
		swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainInfo.presentMode = Spec.presentMode;							// 设置交换链呈现模式
		swapChainInfo.clipped = VK_TRUE;
		swapChainInfo.oldSwapchain = VK_NULL_HANDLE;							// 如果需重建交换链可在旧交换链未销毁时创建新的交换链

		if (Spec.familyIndices.graphicsFamliy != Spec.familyIndices.presentFamily)
		{
			/* 图像可在多个队列族之间使用 */
			swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainInfo.queueFamilyIndexCount = 2;
			swapChainInfo.pQueueFamilyIndices = Spec.queueIndices.data();
		}
		else
		{
			/* 图像由同一个队列族拥有 必须显式传输所有权 */
			swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainInfo.queueFamilyIndexCount = 0;
			swapChainInfo.pQueueFamilyIndices = nullptr;
		}

		return swapChainInfo;
	}

	SwapChain::SwapChain(CommandContext* InCommandContext, const VkRenderPass& RenderPass)
		: m_commandContext(InCommandContext), m_renderPass(RenderPass)
	{
		m_mainWindow = Engine::GetEngineInstance().GetMainWindow();
	}

	SwapChain::~SwapChain()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class SwapChain Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void SwapChain::InitSwapChain(SwapChainSpec& Spec)
	{
		if (!Create(Spec))
		{
			SUZZA_LOG_ERROR("创建交换链失败!");
			return;
		}

		CreateImageViews();
		CreateDepthResources();
		CreateFrameBuffers();
	}

	bool SwapChain::Create(SwapChainSpec& Spec)
	{
		return CreateSwapChain(Spec);
	}

	bool SwapChain::CreateSwapChain(SwapChainSpec& Spec)
	{
		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建交换链: 逻辑设备device不合法!");
			return false;
		}
		
		m_deviceContext = Spec.deviceContext;

		Spec.queueIndices.resize(2);
		Spec.queueIndices[0] = Spec.familyIndices.graphicsFamliy.value();
		Spec.queueIndices[1] = Spec.familyIndices.presentFamily.value();

		VkSwapchainCreateInfoKHR swapChainInfo = SwapChainSpec::DefaultSwapChainCreateInfoKHR(Spec);
		if (vkCreateSwapchainKHR(Spec.deviceContext.device, &swapChainInfo, nullptr, &m_swapChain) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建交换链失败!");
			return false;
		}

		/* 需要先查询才能够知道创建之后的交换链图像数量 */
		uint32 imageCount = 0;
		vkGetSwapchainImagesKHR(m_deviceContext.device, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_deviceContext.device, m_swapChain, &imageCount, m_swapChainImages.data());

		/* 保存交换链宽高和图像格式 */
		m_swapChainExtent = swapChainInfo.imageExtent;
		m_swapChainImageFormat = swapChainInfo.imageFormat;

		return true;
	}

	void SwapChain::Destroy()
	{
		if (!(*m_isAlive)) return;

		CleanupSwapChain();

		*m_isAlive = false;
	}

	void SwapChain::ReCreateSwapChain()
	{
		/* 窗口最小化处理 */
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_mainWindow->GetMainWindow(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_mainWindow->GetMainWindow(), &width, &height);
			glfwWaitEvents();
		}

		/* 等待设备完成工作 */
		vkDeviceWaitIdle(m_deviceContext.device);

		/* 清理旧的缓冲区和交换链 */
		//BufferArea->CleanupBufferArea();
		CleanupSwapChain();

		/* 创建新的交换链和新的帧缓冲区 */
		SwapChainSpec spec = suz_swapchain::CreateDefaultSwapChain();
		InitSwapChain(spec);
		//BufferArea->ReCreateBufferArea(m_swapChainExtent);
	}

	void SwapChain::CleanupSwapChain()
	{
		/* 1. 销毁帧缓冲 */
		for (UniquePtr<FrameBuffer>& frameBuffer : m_frameBuffers)
		{
			frameBuffer->Destroy();
			frameBuffer = nullptr;
		}

		/* 2. 销毁深度图像资源 */
		vkDestroyImageView(m_deviceContext.device, m_depthImageView, nullptr);
		vkFreeMemory(m_deviceContext.device, m_depthImageMemory, nullptr);
		vkDestroyImage(m_deviceContext.device, m_depthImage, nullptr);

		/* 3. 销毁图像视图 */
		for (const VkImageView& imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_deviceContext.device, imageView, nullptr);
		}
		m_swapChainImageViews.clear();
		m_swapChainImages.clear();

		/* 4. 销毁缓冲区 */
		vkDestroySwapchainKHR(m_deviceContext.device, m_swapChain, nullptr);
	}

	void SwapChain::CreateImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		/* 迭代所有交换链图像 */
		for (usize currentImage = 0; currentImage < m_swapChainImages.size(); currentImage++)
		{
			VkImageViewCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageCreateInfo.image = m_swapChainImages[currentImage];
			imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// 指定用2D纹理解释图像
			imageCreateInfo.format = m_swapChainImageFormat;						// 设置图像格式
			/* 默认映射颜色通道 */
			imageCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			/* 指定访问图像的用途以及访问图像哪一部分 */
			imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCreateInfo.subresourceRange.baseMipLevel = 0;
			imageCreateInfo.subresourceRange.levelCount = 1;
			imageCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_deviceContext.device, &imageCreateInfo, nullptr, &m_swapChainImageViews[currentImage]) != VK_SUCCESS)
			{
				SUZZA_LOG_ERROR("创建交换链图像失败!, ", std::to_string(currentImage));
			}
		}
	}

	void SwapChain::CreateDepthResources()
	{
		/* 查询硬件支持的最佳深度格式 */
		VkFormat depthFormat = VulkanFunctionLibrary::FindDepthFormat(m_deviceContext.physicalDevice);

		/* 创建深度缓冲图像并分配设备本地显存 */
		Texture::CreateImage(
			m_swapChainExtent.width, m_swapChainExtent.height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,							// 最优平铺 GPU访问性能最高
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,		// 用途: 作为深度/模板渲染附件
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,				// 类型: GPU本地显存
			m_depthImage,
			m_depthImageMemory
		);

		/* 创建深度图像视图, 只可见深度面 */
		m_depthImageView = Texture::CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		/* 将图像从未定义转换为 "深度模板附件最优" 布局 */
		/* 未定义布局: 新建图像无有效数据, 转换时不保留旧内容, 性能最优 */
		m_commandContext->TransitionImageLayout(m_depthImage, depthFormat,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
	}

	void SwapChain::CreateFrameBuffers()
	{
		/* 创建图像视图的帧缓冲 */
		m_frameBuffers.resize(m_swapChainImageViews.size());

		for (usize frameIndex = 0; frameIndex < m_swapChainImageViews.size(); frameIndex++)
		{
			FrameBufferSpec spec(m_deviceContext, m_renderPass);
			spec.swapChainExtent = m_swapChainExtent;
			spec.attachments = { m_swapChainImageViews[frameIndex],m_depthImageView };

			VkFramebufferCreateInfo framebufferInfo = FrameBufferSpec::DefaultFrameBufferCreateInfo(spec);
			
			m_frameBuffers[frameIndex] = std::make_unique<FrameBuffer>();
			if (!m_frameBuffers[frameIndex]->Create(spec))
			{
				SUZZA_LOG_ERROR("创建帧缓冲失败!, 索引号: ", frameIndex);
				continue;
			}
		}
	}
}