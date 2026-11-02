#include "Device.h"
#include "functionLibrary/DeviceFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkDeviceCreateInfo DeviceSpec::DefaultDeviceCreateInfo(const DeviceSpec& Spec)
	{
		/* 创建逻辑设备 */
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32>(Spec.queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = Spec.queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &Spec.physicalDeviceFeatures;

		/* 启用设备(如交换链等)扩展 */
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32>(Spec.deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = Spec.deviceExtensions.data();

		if (DeviceFunctionLibrary::CheckPhysicalDeviceFeatureSupport(Spec.physicalDevice))
		{
			deviceCreateInfo.pNext = &Spec.deviceFeaturesChain;
		}
		else
		{
			SUZZA_LOG_INFO("设备不支持所需的特性!");
			return {};
		}

		/* 再次设置设备验证层, 防止被忽略 */
		if (Spec.enableValidationLayers)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32>(Spec.validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = Spec.validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		return deviceCreateInfo;
	}


	Device::~Device()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class Device Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool Device::CreateDevice(DeviceSpec& Spec)
	{
		if (Spec.physicalDevice == VK_NULL_HANDLE)
		{
			SUZZA_LOG_ERROR("无法创建逻辑设备: 物理设备physicalDevice不合法!");
			return false;
		}

		for (VkDeviceQueueCreateInfo& queueInfo : Spec.queueCreateInfos)
		{
			queueInfo.pQueuePriorities = &Spec.queuePriority;
		}

		VkDeviceCreateInfo deviceInfo = DeviceSpec::DefaultDeviceCreateInfo(Spec);
		if (vkCreateDevice(Spec.physicalDevice, &deviceInfo, nullptr, &m_device) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建逻辑设备失败!");
			return false;
		}

		/* 缓存队列族索引 */
		m_graphicsFamilyIndex = Spec.queueFamilieIndices.graphicsFamliy.value();
		m_presentFamilyIndex = Spec.queueFamilieIndices.presentFamily.value();

		/* 检索目标设备的队列句柄并进行缓存 */
		vkGetDeviceQueue(m_device, Spec.queueFamilieIndices.graphicsFamliy.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, Spec.queueFamilieIndices.presentFamily.value(), 0, &m_presentQueue);

		return true;
	}

	void Device::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyDevice(m_device, nullptr);

		*m_isAlive = false;
	}
}