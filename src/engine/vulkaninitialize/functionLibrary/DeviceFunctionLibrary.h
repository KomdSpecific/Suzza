#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.hpp>

namespace suz
{
	struct QueueFamilyCollector;

	class DeviceFunctionLibrary
	{
	public:
		/* 查询设备对硬件能力的支持 */
		static bool CheckPhysicalDeviceFeatureSupport(const VkPhysicalDevice& PhysicalDevice);
		/* 创建设备硬件能力的链表 */
		static vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT> CreateDeviceFeaturesChain();

		static uint32 GetDeviceMaxBindlessImageNum(const VkPhysicalDevice& PhysicalDevice);

		static VkPhysicalDeviceProperties GetPhysicalDeviceProperties(const VkPhysicalDevice& PhysicalDevice);

		static QueueFamilyCollector FindQueueFamilies(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface);
	};
}