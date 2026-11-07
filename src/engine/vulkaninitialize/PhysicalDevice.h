#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class PhysicalDevice
	{
	public:
		PhysicalDevice(const VkInstance& Instance, const VkSurfaceKHR& Surface, const Vector<const char*>& DeviceExtensions);

		bool FindPhyscialDevice();

	private:
		VkInstance m_vulkanInstance = {};
		VkSurfaceKHR m_surface = {};

		Vector<const char*> m_deviceExtensions = {};

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

	public:
		inline const VkPhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }
	};
}