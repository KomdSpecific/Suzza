#include "VulkanFunctionLibrary.h"
#include "MainWindow.h"
#include "macros/BuildMacro.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	uint32 VulkanFunctionLibrary::FindMemoryType(const VkPhysicalDevice& PhysicalDevice, uint32 TypeFilter, const VkMemoryPropertyFlags& Property)
	{
		/* 查询相关可用内存的信息 */
		VkPhysicalDeviceMemoryProperties properties{};
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &properties);

		/* 查询合适的内存索引 可能存在多个期望的属性因此按位与运算比较 */
		for (usize typeIndex = 0; typeIndex < properties.memoryTypeCount; typeIndex++)
		{
			/* 通过按位比较得知支持全部功能 */
			if (TypeFilter & (1 << typeIndex) && (properties.memoryTypes[typeIndex].propertyFlags & Property) == Property)
			{
				return static_cast<uint32>(typeIndex);
			}
		}

		SUZZA_LOG_ERROR("无法查找到合适的内存类型");
		return UINT32_MAX;
	}

	VkFormat VulkanFunctionLibrary::FindDepthFormat(const VkPhysicalDevice& PhysicalDevice)
	{
		VkFormat outFormat{};
		bool success = FindSupportFormat(PhysicalDevice,
			{ VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,											// 图像像素块按照最优平铺模式 GPU访问性能最高
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
			outFormat
		);

		if (!success)
			SUZZA_LOG_ERROR("未找到合适的深度格式!");

		return outFormat;
	}

	bool VulkanFunctionLibrary::FindSupportFormat(
		const VkPhysicalDevice& PhysicalDevice,
		const Vector<VkFormat>& InFormats,
		const VkImageTiling& Tiling,
		const VkFormatFeatureFlags& Features,
		VkFormat& OutFormat
	)
	{
		for (const VkFormat& format : InFormats)
		{
			/* 获取格式的支持情况 */
			VkFormatProperties formatProperties{};
			vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &formatProperties);

			/* 查找设备是否包含全部所需的功能和期望的目标特性 */
			if (Tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & Features) == Features)
			{
				OutFormat = format;
				return true;
			}
			else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & Features) == Features)
			{
				OutFormat = format;
				return true;
			}
		}

		OutFormat = VK_FORMAT_UNDEFINED;
		return false;
	}

	bool VulkanFunctionLibrary::HasStencilComponent(const VkFormat& DepthFormat)
	{
		return DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkSurfaceFormatKHR VulkanFunctionLibrary::ChooseSuitableSurfaceFormatKHR(const Vector<VkSurfaceFormatKHR>& AvailableFormats)
	{
		for (const VkSurfaceFormatKHR& format : AvailableFormats)
		{
			/* 选择支持按顺序存储的RGB的format颜色通道类型 */
			/* 选择支持SRGB色彩空间的colorSpace */
			if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return AvailableFormats[0];
	}

	VkPresentModeKHR VulkanFunctionLibrary::ChooseSuitablePresentModeKHR(const Vector<VkPresentModeKHR>& AvailableModes)
	{
		for (const VkPresentModeKHR& mode : AvailableModes)
		{
			/* 最好选择在队列已满时程序不会停止运行而是将队列中的图像替换成更新的图像 */
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		/* 默认情况下程序在队列后面插入绘制好的图像, 显示器刷新时从队列前面获取图像 */
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanFunctionLibrary::GetSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities, MainWindow* Window)
	{
		/* 某些情况下窗口实际分辨率不一定等于窗口系统的分辨率 */
		/* 窗口系统给出了明确固定的分辨率, 直接返回 */
		if (Capabilities.currentExtent.width != std::numeric_limits<uint32>::max())
		{
			return Capabilities.currentExtent;
		}
		else
		{
			Window->ResetWindowExtent();
			return Window->GetWindowExtent();
		}
	}

	Vector<const char*> VulkanFunctionLibrary::GetRequiredExtensions()
	{
		/* 获取GLFW窗口所需要的Vulkan实例扩展数量 */
		uint32 extensionsCount = 0;
		/* glfw自动返回当前平台渲染窗口必须的扩展列表 */
		const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

		Vector<const char*> extensions(requiredExtensions, requiredExtensions + extensionsCount);

		/* 启用验证层时, 追加调试扩展 */
#ifdef SUZZA_DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#endif
		return extensions;
	}
	
	bool VulkanFunctionLibrary::CheckValidationSupport(const Vector<const char*>& ValidationLayers)
	{
		/* 获取可用层数量 */
		uint32 layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		/* 获取所有可用层详情 */
		Vector<VkLayerProperties> layerProperties(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

		for (const char* layer : ValidationLayers)
		{
			/* 遍历所有系统可用层 */
			bool found = false;
			for (const VkLayerProperties& property : layerProperties)
			{
				/* 如果两字符串相等直接跳出内层循环 */
				if (strcmp(layer, property.layerName) == 0)
				{
					found = true;
					break;
				}
			}

			/* 但凡有一个验证层要求不满足直接返回 */
			if (!found) return false;
		}

		return true;
	}

	VkResult VulkanFunctionLibrary::CreateDebugUtilsMessengerEXT(
		VkInstance Instance,
		const VkDebugUtilsMessengerCreateInfoEXT* MessengerCreateInfo,
		const VkAllocationCallbacks* Allocator,
		VkDebugUtilsMessengerEXT* DebugMessenger
	)
	{
		PFN_vkCreateDebugUtilsMessengerEXT messenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
		if (messenger)
		{
			return messenger(Instance, MessengerCreateInfo, Allocator, DebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VulkanFunctionLibrary::DestroyDebugUtilsMessengerEXT(
		VkInstance Instance,
		VkDebugUtilsMessengerEXT DebugMessenger,
		const VkAllocationCallbacks* Allocator
	)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT messenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (messenger)
		{
			messenger(Instance, DebugMessenger, Allocator);
		}
	}
}