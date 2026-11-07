#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.hpp>

namespace suz
{
	struct QueueFamilyCollector;
	struct SwapChainSupportDetails;

	class DeviceFunctionLibrary
	{
	public:
		static bool IsPhysicalDeviceSuitable(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface, const Vector<const char*>& DeviceExtensions);

		/* 查询设备是否支持所需的扩展 */
		static bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice& PhysicalDevice, const Vector<const char*>& DeviceExtensions);

		/* 查询设备对硬件特性的支持 */
		static bool CheckPhysicalDeviceFeatureSupport(const VkPhysicalDevice& PhysicalDevice);
		/* 创建设备硬件能力的链表 */
		static vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT> CreateDeviceFeaturesChain();

		static uint32 GetDeviceMaxBindlessImageNum(const VkPhysicalDevice& PhysicalDevice);

		static VkPhysicalDeviceProperties GetPhysicalDeviceProperties(const VkPhysicalDevice& PhysicalDevice);

		static QueueFamilyCollector FindQueueFamilies(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface);

		static SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface);
	};
}