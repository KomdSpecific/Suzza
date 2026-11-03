#include "VulkanFunctionLibrary.h"

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
}