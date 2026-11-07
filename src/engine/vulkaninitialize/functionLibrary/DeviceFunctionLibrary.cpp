#include "DeviceFunctionLibrary.h"
#include "macros/VkMacro.h"
#include "vulkaninitialize/Device.h"
#include "vulkaninitialize/SwapChain.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	bool DeviceFunctionLibrary::IsPhysicalDeviceSuitable(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface, const Vector<const char*>& DeviceExtensions)
	{
		/* 查询目标设备基本属性 */
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceProperties(PhysicalDevice, &properties);
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &features);

		SUZZA_LOG_INFO("当前设备: ", properties.deviceName);
		
		if (FORCE_DISCRETE_GPU)
		{
			/* 显卡要求为独显 */
			if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				return false;
		}

		if (FORCE_GEOMETRY_SHADER)
		{
			/* 程序应适用于支持几何着色器的独立显卡 */
			if (features.geometryShader != VK_TRUE)
				return false;
		}

		/* 查找设备队列族索引 */
		QueueFamilyCollector familyIndices = FindQueueFamilies(PhysicalDevice, Surface);

		/* 查询设备是否支持扩展 (如交换链) */
		bool requiredExtensionSupported = CheckPhysicalDeviceExtensionSupport(PhysicalDevice, DeviceExtensions);

		/* 查询设备是否支持图像格式和呈现模式 */
		bool swapChainAdequate = false;
		if (requiredExtensionSupported)
		{
			SwapChainSupportDetails details = QuerySwapChainSupport(PhysicalDevice, Surface);
			swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
		}

		return familyIndices.IsValid() && requiredExtensionSupported && swapChainAdequate && features.samplerAnisotropy;
	}

	bool DeviceFunctionLibrary::CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice& PhysicalDevice, const Vector<const char*>& DeviceExtensions)
	{
		/* 获取设备支持的所有扩展数量 */
		uint32 extensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionsCount, nullptr);

		/* 获取设备支持的所有扩展 */
		Vector<VkExtensionProperties> extensionProperties(extensionsCount);
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionsCount, extensionProperties.data());

		Set<String> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

		for (const VkExtensionProperties& property : extensionProperties)
		{
			/* 每找到目标数据即从扩展列表排除 */
			requiredExtensions.erase(property.extensionName);
		}

		return requiredExtensions.empty();
	}

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

		uint8 i = 0;
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

	SwapChainSupportDetails DeviceFunctionLibrary::QuerySwapChainSupport(const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface)
	{
		SwapChainSupportDetails details{};

		/* 获取设备的基本表面功能 */
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &details.capabilities);

		/* 查询设备支持的表面格式 */
		uint32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, details.formats.data());
		}

		/* 查询设备支持的呈现格式 */
		uint32 presentCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			details.presentModes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &presentCount, details.presentModes.data());
		}

		return details;
	}
}