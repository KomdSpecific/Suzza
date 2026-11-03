#include "DeviceFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/functionLibrary/DeviceFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	namespace suz_device
	{
		DeviceSpec CreateDefaultDeviceSpec()
		{
			const VkPhysicalDevice& physicalDevice = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetPhysicalDevice();
			const VkSurfaceKHR& surface = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetSurface();

			DeviceSpec spec(physicalDevice);
			spec.validationLayers = { "VK_LAYER_KHRONOS_validation" };
			spec.deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
			};

			QueueFamilyCollector collector = DeviceFunctionLibrary::FindQueueFamilies(physicalDevice, surface);
			if (!collector.IsValid())
			{
				SUZZA_LOG_ERROR("无法找到合适的队列族!");
				return DeviceSpec(physicalDevice);
			}

			spec.queueFamilieIndices = collector;

			Set<uint32> indices = { collector.graphicsFamliy.value(),collector.presentFamily.value() };
			for (const uint32& index : indices)
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueCount = 1;
				queueInfo.queueFamilyIndex = index;
				spec.queueCreateInfos.push_back(queueInfo);
			}

			/* 指定要使用的设备特性 */
			spec.physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;						// 开启采样器各项异性过滤

			/* 将设备开启能力链表设置给设备创建信息 */
			if (DeviceFunctionLibrary::CheckPhysicalDeviceFeatureSupport(physicalDevice))
			{
				spec.deviceFeaturesChain = DeviceFunctionLibrary::CreateDeviceFeaturesChain();
			}
			else
			{
				SUZZA_LOG_ERROR("设备不支持所需特性, 构建DeviceSpec失败!");
				return DeviceSpec(physicalDevice);
			}

			return spec;
		}
	}
}