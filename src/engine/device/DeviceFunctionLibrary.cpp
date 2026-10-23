#include "DeviceFunctionLibrary.h"

namespace suz
{
	uint32 DeviceFunctionLibrary::GetDeviceMaxBindlessImageNum(const VkPhysicalDevice& PhysicalDevice)
	{
		VkPhysicalDeviceProperties deviceProp{};
		vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProp);
		return deviceProp.limits.maxDescriptorSetSampledImages;
	}

	VkPhysicalDeviceProperties DeviceFunctionLibrary::GetPhysicalDeviceProperties(const VkPhysicalDevice& PhysicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(PhysicalDevice, &physicalDeviceProperties);
		return physicalDeviceProperties;
	}
}