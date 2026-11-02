#pragma once

#include "SuzzaTypes.h"
#include "VulkanResourceBase.h"

#include <vulkan/vulkan.hpp>
#include <optional>

namespace suz
{
	struct QueueFamilyCollector
	{
		std::optional<uint32> graphicsFamliy;
		std::optional<uint32> presentFamily;

		const bool IsValid() const
		{
			return graphicsFamliy.has_value() && presentFamily.has_value();
		}
	};

	struct DeviceSpec
	{
		/* 验证层启用控制 */
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif // DEBUG模式自动开启验证层

		DeviceSpec(const VkPhysicalDevice& PhysicalDevice)
			: physicalDevice(PhysicalDevice) {
		};

		const VkPhysicalDevice& physicalDevice;

		/* 指定队列分配优先级 */
		const float queuePriority = 1.0f;

		Vector<const char*> validationLayers{};
		Vector<const char*> deviceExtensions{};

		/* 逻辑设备支持的队列族 */
		QueueFamilyCollector queueFamilieIndices{};
		Vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		/* 将开启的物理设备特性 */
		VkPhysicalDeviceFeatures physicalDeviceFeatures{};
		/* 缓存设备硬件能力的链表 */
		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT> deviceFeaturesChain{};

		static VkDeviceCreateInfo DefaultDeviceCreateInfo(const DeviceSpec& Spec);
	};

	class Device : public VulkanResourceBase
	{
	public:
		Device() = default;
		~Device();

		bool CreateDevice(DeviceSpec& Spec);

		void Destroy();

	private:
		VkDevice m_device = {};

		/* 设备的队列族索引 */
		uint32 m_graphicsFamilyIndex = 0;
		uint32 m_presentFamilyIndex = 0;

		/* 设备的队列句柄 */
		VkQueue m_graphicsQueue = {};
		VkQueue m_presentQueue = {};

	public:
		inline const VkDevice& GetDevice() const { return m_device; }

		inline uint32 GetGraphicsFamily() const { return m_graphicsFamilyIndex; }
		inline uint32 GetPresentFamily() const { return m_presentFamilyIndex; }
		inline const VkQueue& GetGraphicsQueue() const { return m_graphicsQueue; }
		inline const VkQueue& GetPresentQueue() const { return m_presentQueue; }
	};
}