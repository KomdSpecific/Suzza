#include "DeviceFunctionLibrary.h"

namespace suz
{
	uint32 DeviceFunctionLibrary::GetDeviceMaxBindlessImageNum(const VkPhysicalDevice& PhysicalDevice)
	{
		VkPhysicalDeviceProperties deviceProp{};
		vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProp);
		return deviceProp.limits.maxDescriptorSetSampledImages;
	}
}