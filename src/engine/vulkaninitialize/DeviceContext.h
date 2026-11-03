#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>

namespace suz
{
	struct DeviceContext
	{
		VkDevice device = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		uint32 graphicsFamilyIndex = 0;
		uint32 presentFamilyIndex = 0;

		const bool DeviceValid() const
		{
			return device != VK_NULL_HANDLE;
		}

		const bool PhysicalDeviceValid() const
		{
			return physicalDevice != VK_NULL_HANDLE;
		}

		const bool GraphicsQueueValid() const
		{
			return graphicsQueue != VK_NULL_HANDLE;
		}

		const bool PresentQueueValid() const
		{
			return presentQueue != VK_NULL_HANDLE;
		}
	};
}