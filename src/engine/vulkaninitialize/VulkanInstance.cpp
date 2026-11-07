#include "VulkanInstance.h"
#include "MainWindow.h"
#include "functionLibrary/VulkanFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VulkanInstanceSpec::VulkanInstanceSpec(MainWindow* Window, const Vector<const char*>& ValidationLayers)
		: mainWindow(Window), validationLayers(ValidationLayers)
	{
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstanceSpec::DefaultDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
		VkDebugUtilsMessageTypeFlagsEXT Type,
		const VkDebugUtilsMessengerCallbackDataEXT* Data,
		void* UserData)
	{
		/* 只打印警告以上级别的调试信息 */
		if (Severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::cerr << "[Vulkan Message]: " << Data->pMessage << std::endl << std::endl;
		}

		/* 调试必须返回VK_FALSE, 否则会终止API调用 */
		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT VulkanInstanceSpec::DefaultDebugMessengerCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		/* 设置监听的消息级别 */
		debugCreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		/* 设置监听的消息类型 */
		debugCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		/* 绑定回调 */
		debugCreateInfo.pfnUserCallback = DefaultDebugCallback;
		debugCreateInfo.pUserData = nullptr;							// 可传入自定义数据指针

		return debugCreateInfo;
	}

	VkInstanceCreateInfo VulkanInstanceSpec::DefaultVulkanInstanceCreateInfo(const VulkanInstanceSpec& Spec)
	{
		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &Spec.appInfo;

#ifdef SUZZA_DEBUG
		/* 注入验证层与调试配置 */
		if (!VulkanFunctionLibrary::CheckValidationSupport(Spec.validationLayers))
		{
			SUZZA_LOG_ERROR("系统不支持请求的验证层!");
			return instanceInfo;
		}

		/* 注入验证层列表 */
		instanceInfo.enabledLayerCount = static_cast<uint32>(Spec.validationLayers.size());
		instanceInfo.ppEnabledLayerNames = Spec.validationLayers.data();

		/* 填充调试信使参数 */
		/* 挂载到pNext扩展链, 捕获实例创建/销毁过程的错误 */
		instanceInfo.pNext = &Spec.debugMessengerCreateInfo;

#endif
		/* 设置需要开启的扩展 */
		instanceInfo.enabledExtensionCount = static_cast<uint32>(Spec.requiredExtensions.size());
		instanceInfo.ppEnabledExtensionNames = Spec.requiredExtensions.data();

		return instanceInfo;
	}

	VulkanInstance::~VulkanInstance()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class VulkanInstance Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool VulkanInstance::Create(VulkanInstanceSpec& Spec)
	{
		return CreateVulkanInstance(Spec);
	}

	bool VulkanInstance::CreateVulkanInstance(VulkanInstanceSpec& Spec)
	{
		/* 创建Vulkan实例 */
		VkInstanceCreateInfo instanceInfo = VulkanInstanceSpec::DefaultVulkanInstanceCreateInfo(Spec);
		if (vkCreateInstance(&instanceInfo, nullptr, &m_vulkanInstance) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建Vulkan实例失败!");
			return false;
		}

		/* 创建调试信使 */
#ifdef SUZZA_DEBUG
		if (VulkanFunctionLibrary::CreateDebugUtilsMessengerEXT(m_vulkanInstance, &Spec.debugMessengerCreateInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("调试信使创建失败!");
			return false;
		}

#endif
		/* 创建窗口表面 */
		if (Spec.mainWindow)
		{
			bool surfaceResult = Spec.mainWindow->CreateWindowSurface(m_vulkanInstance, &m_surface);
			if (!surfaceResult)
			{
				SUZZA_LOG_ERROR("创建窗口表面失败!");
				return false;
			}
		}

		return true;
	}

	void VulkanInstance::Destroy()
	{
		if (!(*m_isAlive)) return;

#ifdef SUZZA_DEBUG
		VulkanFunctionLibrary::DestroyDebugUtilsMessengerEXT(m_vulkanInstance, m_debugMessenger, nullptr);

#endif
		/* 销毁窗口表面 */
		vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
		/* 销毁实例 */
		/* 参数1: 输出实例句柄; 参数2: 自定义内存分配器, 不为空代表需要手动销毁 */
		vkDestroyInstance(m_vulkanInstance, nullptr);

		*m_isAlive = false;
	}
}