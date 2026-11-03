#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class VulkanFunctionLibrary
	{
	public:
		/* 找到正确的内存类型 */
		static uint32 FindMemoryType(const VkPhysicalDevice& PhysicalDevice, uint32 TypeFilter, const VkMemoryPropertyFlags& Property);

		/* 找到硬件支持的深度格式 */
		static VkFormat FindDepthFormat(const VkPhysicalDevice& PhysicalDevice);

		/* 从格式列表中找到支持的格式
		*  参数1: 查找的目标格式列表
		*  参数2: 图像像素在显存内的排列方式
		*  参数3: 图像像素格式希望支持的用途
		*/
		static bool FindSupportFormat(
			const VkPhysicalDevice& PhysicalDevice,
			const Vector<VkFormat>& InFormats,
			const VkImageTiling& Tiling,
			const VkFormatFeatureFlags& Features,
			VkFormat& OutFormat
		);

		/* 判断选择的深度格式是否包含模板分量 */
		static bool HasStencilComponent(const VkFormat& DepthFormat);
	};
}