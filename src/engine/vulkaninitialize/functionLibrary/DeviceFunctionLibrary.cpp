#include "DeviceFunctionLibrary.h"
#include "macros/VkMacro.h"
#include "vulkaninitialize/Device.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	bool DeviceFunctionLibrary::CheckPhysicalDeviceFeatureSupport(const VkPhysicalDevice& PhysicalDevice)
	{
		/* 查询设备是否支持描述符索引 */
		auto queryStructureChain = vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT
		>{};

		auto& deviceFeatures2 = queryStructureChain.get<vk::PhysicalDeviceFeatures2>();
		auto& descIndexFeaturesEXT = queryStructureChain.get<vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

		/* 调用接口, 驱动填充全部Features */
		vkGetPhysicalDeviceFeatures2(PhysicalDevice, reinterpret_cast<VkPhysicalDeviceFeatures2*>(&deviceFeatures2));

		/* shader里数组下标可以是变量 */
		/* 允许描述符集合更新之后, 不重新分配set; 运行时可修改数组中元素 */
		/* 允许提交后更新元素 */
		bool featSupport = descIndexFeaturesEXT.shaderSampledImageArrayNonUniformIndexing == VK_TRUE &&
			descIndexFeaturesEXT.descriptorBindingVariableDescriptorCount == VK_TRUE &&
			descIndexFeaturesEXT.descriptorBindingUpdateUnusedWhilePending == VK_TRUE;

		/* 至少预留256个纹理槽位 */
		bool bindlessSupport = DeviceFunctionLibrary::GetDeviceMaxBindlessImageNum(PhysicalDevice) >= MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;

		if (featSupport && bindlessSupport)
		{
			return true;
		}
		else
		{
			SUZZA_LOG_ERROR("设备无法支持所需功能!");
			return false;
		}
	}

	vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceDescriptorIndexingFeaturesEXT> DeviceFunctionLibrary::CreateDeviceFeaturesChain()
	{
		auto featuresChain = vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>{};

		auto& deviceFeatures2 = featuresChain.get<vk::PhysicalDeviceFeatures2>();
		auto& descIndexFeaturesEXT = featuresChain.get<vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

		/* shader里数组下标可以是变量 */
		/* 允许描述符集合更新之后, 不重新分配set; 运行时可修改数组中元素 */
		/* 允许提交后更新元素 */
		descIndexFeaturesEXT.runtimeDescriptorArray = VK_TRUE;
		descIndexFeaturesEXT.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		descIndexFeaturesEXT.descriptorBindingVariableDescriptorCount = VK_TRUE;
		descIndexFeaturesEXT.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
		descIndexFeaturesEXT.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		descIndexFeaturesEXT.descriptorBindingPartiallyBound = VK_TRUE;

		return featuresChain;
	}

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

	QueueFamilyCollector DeviceFunctionLibrary::FindQueueFamilies(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface)
	{
		QueueFamilyCollector collector{};

		/* 获取设备中队列族成员数量 */
		uint32 familiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &familiesCount, nullptr);

		/* 获取设备所有队列族信息 */
		Vector<VkQueueFamilyProperties> familyProperties(familiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &familiesCount, familyProperties.data());

		int i = 0;
		for (const VkQueueFamilyProperties& property : familyProperties)
		{
			if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				collector.graphicsFamliy = i;
			}

			/* 查找支持绘制到窗口命令的队列族 */
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &presentSupport);
			if (presentSupport)
			{
				collector.presentFamily = i;
			}

			/* 只要找到可处理的队列族, 跳出循环*/
			if (collector.IsValid())
			{
				break;
			}

			i++;
		}

		return collector;
	}
}