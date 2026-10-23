#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class DeviceFunctionLibrary
	{
	public:
		static uint32 GetDeviceMaxBindlessImageNum(const VkPhysicalDevice& PhysicalDevice);

		static VkPhysicalDeviceProperties GetPhysicalDeviceProperties(const VkPhysicalDevice& PhysicalDevice);
	};
}