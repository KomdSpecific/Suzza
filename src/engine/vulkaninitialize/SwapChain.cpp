#include "SwapChain.h"
#include "Engine.h"
#include "VulkanInitialize.h"
#include "VulkanResourceManager.h"
#include "BufferArea.h"
#include "MainWindow.h"

#include "debug/SuzzaLog.h"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace suz
{
	SwapChain::SwapChain(const VkDevice& Device)
		: m_device(Device)
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

	void SwapChain::InitSwapChain()
	{
		CreateSwapChain();
		CreateImageViews();
	}

	void SwapChain::Destroy()
	{
		if (!(*m_isAlive)) return;

		CleanupSwapChain();

		*m_isAlive = false;
	}

	SwapChainSupportDetails SwapChain::QuerySupportChainSupport(const VkPhysicalDevice& Device, const VkSurfaceKHR& Surface)
	{
		SwapChainSupportDetails details = {};

		/* 获取设备的基本表面功能 */
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &details.capabilities);
		
		/* 查询设备支持的表面格式 */
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &formatCount, details.formats.data());
		}

		/* 查询设备支持的呈现格式 */
		uint32_t presentCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			details.presentModes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
	{
		for (const VkSurfaceFormatKHR& surfaceFormat : AvailableFormats)
		{
			/* 选择支持按顺序存储的RGB的format颜色通道类型 */
			/* 选择支持SRGB色彩空间的colorSpace */
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return surfaceFormat;
			}
		}

		return AvailableFormats[0];
	}

	VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes)
	{
		for (const VkPresentModeKHR& presentMode : AvailableModes)
		{
			/* 最好选择在队列已满时程序不会停止运行而是将队列中的图像替换成更新的图像 */
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return presentMode;
			}
		}

		/* 默认情况下程序在队列后面插入绘制好的图像, 显示器刷新时从队列前面获取图像 */
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void SwapChain::ReCreateSwapChain(BufferArea* BufferArea)
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
		vkDeviceWaitIdle(m_device);

		/* 清理旧的缓冲区和交换链 */
		BufferArea->CleanupBufferArea();
		CleanupSwapChain();

		/* 创建新的交换链和新的帧缓冲区 */
		CreateSwapChain();
		CreateImageViews();
		BufferArea->ReCreateBufferArea(m_swapChainExtent);
	}

	void SwapChain::CleanupSwapChain()
	{
		for (const VkImageView& imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	}

	void SwapChain::CreateSwapChain()
	{
		VulkanInitialize* vulkanInitialize = Engine::GetVulkanResourceManager().GetVulkanInitialize();
		const VkPhysicalDevice& physicalDevice = vulkanInitialize->GetPhysicalDevice();
		const VkSurfaceKHR& surface = vulkanInitialize->GetSurface();

		SwapChainSupportDetails swapChainSupport = QuerySupportChainSupport(physicalDevice, surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);

		VkExtent2D currentExtent = GetSwapChainExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0)
		{
			/* 保证不超过图像的最大数量 */
			imageCount = std::min(imageCount, swapChainSupport.capabilities.maxImageCount);
		}
		/* 保证未来数据改动导致 max < min */
		imageCount = std::max(imageCount, swapChainSupport.capabilities.minImageCount);

		/* 创建交换链对象 */
		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = surface;
		/* 指定交换链图像的详细信息 */
		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = surfaceFormat.format;
		swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapChainCreateInfo.imageExtent = currentExtent;
		swapChainCreateInfo.imageArrayLayers = 1;									// 指定图像由多少层组成
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;		// 指定交换链中图像的操作类型

		QueueFamilyIndices indices = vulkanInitialize->FindQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			/* 图像可在多个队列族之间使用 */
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			/* 图像一次由一个队列族拥有 必须显式传输所有权 */
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainCreateInfo.queueFamilyIndexCount = 0;
			swapChainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		/* 指定alpha通道是否与窗口系统中的其他窗口混合 */
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;		// 此刻设置忽略alpha通道
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;							// 如果需重建交换链可在旧交换链未销毁时创建新的交换链

		if (vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("创建交换链失败!");
		}

		/* 检索交换链所有图像 */
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = currentExtent;
	}

	void SwapChain::CreateImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		/* 迭代所有交换链图像 */
		for (size_t currentImage = 0; currentImage < m_swapChainImages.size(); currentImage++)
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

			if (vkCreateImageView(m_device, &imageCreateInfo, nullptr, &m_swapChainImageViews[currentImage]) != VK_SUCCESS)
			{
				throw std::runtime_error("创建交换链图像失败!" + std::to_string(currentImage));
			}
		}
	}

	VkExtent2D SwapChain::GetSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
	{
		/* 某些情况下窗口实际分辨率大于屏幕坐标分辨率, 因此必须返回像素坐标 */
		if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return Capabilities.currentExtent;
		}
		else
		{
			m_mainWindow->ResetWindowExtent();
			return m_mainWindow->GetWindowExtent();
		}
	}
}