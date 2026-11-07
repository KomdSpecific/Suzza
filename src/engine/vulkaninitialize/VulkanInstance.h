#pragma once

#include "SuzzaTypes.h"
#include "macros/BuildMacro.h"
#include "VulkanResourceBase.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class MainWindow;

	struct VulkanInstanceSpec
	{
		VulkanInstanceSpec(MainWindow* Window, const Vector<const char*>& ValidationLayers);

		MainWindow* mainWindow;

		const Vector<const char*> validationLayers;

		Vector<const char*> requiredExtensions{};

		VkApplicationInfo appInfo{};
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};

		/* 调试回调函数 */
		/* 固定签名, 由Vulkan验证层调用, 可自定义过滤和输出逻辑 */
		static VKAPI_ATTR VkBool32 VKAPI_CALL DefaultDebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
			VkDebugUtilsMessageTypeFlagsEXT Type,
			const VkDebugUtilsMessengerCallbackDataEXT* Data,
			void* UserData
		);

		static VkDebugUtilsMessengerCreateInfoEXT DefaultDebugMessengerCreateInfo();
		static VkInstanceCreateInfo DefaultVulkanInstanceCreateInfo(const VulkanInstanceSpec& Spec);
	};

	class VulkanInstance : public VulkanResourceBase
	{
	public:
		VulkanInstance() = default;
		~VulkanInstance();

		bool Create(VulkanInstanceSpec& Spec);
		bool CreateVulkanInstance(VulkanInstanceSpec& Spec);

		void Destroy();

	private:
		VkInstance m_vulkanInstance = {};								// Vulkan实例
#ifdef SUZZA_DEBUG
		VkDebugUtilsMessengerEXT m_debugMessenger = {};					// 调式信使

#endif
		VkSurfaceKHR m_surface = {};									// 窗口表面

	public:
		inline const VkInstance& GetVulkanInstance() const { return m_vulkanInstance; }
		inline const VkSurfaceKHR& GetSurface() const { return m_surface; }
	};
}