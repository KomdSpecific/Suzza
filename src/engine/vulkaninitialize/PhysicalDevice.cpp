#include "PhysicalDevice.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/functionLibrary/DeviceFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	PhysicalDevice::PhysicalDevice(const VkInstance& Instance, const VkSurfaceKHR& Surface, const Vector<const char*>& DeviceExtensions)
		: m_vulkanInstance(Instance), m_surface(Surface), m_deviceExtensions(DeviceExtensions)
	{
	}

	bool PhysicalDevice::FindPhyscialDevice()
	{
		/* 获取所有GPU设备数量 */
		uint32 physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(m_vulkanInstance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			SUZZA_LOG_ERROR("无法找到支持Vulkan的GPU!");
			return false;
		}

		/* 该数组保存所有的设备句柄 */
		Vector<VkPhysicalDevice> physicalDeviceList(physicalDeviceCount);
		vkEnumeratePhysicalDevices(m_vulkanInstance, &physicalDeviceCount, physicalDeviceList.data());

		for (const VkPhysicalDevice& physicalDevice : physicalDeviceList)
		{
			if (DeviceFunctionLibrary::IsPhysicalDeviceSuitable(physicalDevice, m_surface, m_deviceExtensions))
			{
				/* 只要找到合适的设备即终止 */
				m_physicalDevice = physicalDevice;
				break;
			}
		}

		if (m_physicalDevice != VK_NULL_HANDLE)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}