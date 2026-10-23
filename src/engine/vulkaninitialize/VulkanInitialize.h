#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <optional>

namespace suz
{
	class MainWindow;

	struct QueueFamilyIndices
	{
		std::optional<uint32> graphicsFamily;
		std::optional<uint32> presentFamily;

		bool IsValid() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class VulkanInitialize : public BaseObject
	{
		/* 验证层启用控制 */
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif // DEBUG模式自动开启验证层

	public:
		VulkanInitialize();
		~VulkanInitialize();

		virtual void Destroy() override;

#pragma region HelperFunction
		/* 调试回调函数 */
		/* 固定签名, 由Vulkan验证层调用, 可自定义过滤和输出逻辑 */
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* data,
			void* userData);

		/* 创建调试信使 */
		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
			const VkAllocationCallbacks* allocator,
			VkDebugUtilsMessengerEXT* outMessenger
		);
		/* 销毁调试信使 */
		void DestroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT messenger,
			const VkAllocationCallbacks* allocator
		);
		/* 填充调试信使创建参数 */
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo);

		/* 工具: 检查验证层是否在系统中可用 */
		bool CheckValidationLayerSupport();
		/* 工具: 生成实例所需的扩展列表 */
		std::vector<const char*> GetRequiredExtensions();

		/* 评估每个设备是否适合 */
		bool IsDeviceSuitable(const VkPhysicalDevice& physicalDevice);
		/* 检索物理设备信息 */
		VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
		/* 评估设备是否支持扩展 */
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& physicalDevice);
		/* 再设备中查找所需的所有队列族 */
		QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& physicalDevice);
		static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);
#pragma endregion

	private:
		void InitVulkanInfo();
		void SetupDebugMessenger();
		void CreateVulkanInstance();								// 创建Vulkan实例
		void CreateSurface();										// 创建窗口表面
		void PickPhysicalDevice();									// 获取物理设备
		void CreateLogicalDevice();									// 创建逻辑设备

		/* 查询并添加设备对硬件能力的支持 */
		bool CheckDeviceFeatureSupport(const VkPhysicalDevice& physicalDevice);
		/* 创建设备硬件能力的链表 */
		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT
		> CreateDeviceFeaturesChain();

		MainWindow* m_mainWindow;

		VkInstance m_vkInstance = {};
		VkApplicationInfo m_vkAppInfo = {};
		VkInstanceCreateInfo m_vkCreateInfo = {};
		VkDebugUtilsMessengerEXT m_debugMessenger = {};
		VkSurfaceKHR m_surface = {};

		VkPhysicalDevice m_vkPhysicalDevice = {};
		VkDevice m_device = {};
		VkQueue m_graphicsQueue = {};
		VkQueue m_presentQueue = {};

		/* 程序是否要求独显开关 */
		bool m_shouldRequiredDiscreteGPU = false;
		/* 缓存设备硬件能力的链表 */
		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT
		> m_deviceFeaturesChain = {};

	public:
		inline const VkSurfaceKHR& GetSurface() const { return m_surface; }
		inline const VkPhysicalDevice& GetPhysicalDevice() const { return m_vkPhysicalDevice; }
		inline const VkDevice& GetDevice() const { return m_device; }
		inline const VkQueue& GetGraphicsQueue() const { return m_graphicsQueue; }
		inline const VkQueue& GetPresentQueue() const { return m_presentQueue; }
	};
}