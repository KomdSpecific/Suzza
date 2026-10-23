#include "VulkanInitialize.h"
#include "Engine.h"
#include "MainWindow.h"
#include "SwapChain.h"
#include "device/DeviceFunctionLibrary.h"
#include "macros/VkMacro.h"

#include "debug/SuzzaLog.h"

#include <stdexcept>
#include <set>

namespace suz
{
	/* 请求的验证层列表 */
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
	};

	VulkanInitialize::VulkanInitialize()
	{
		m_mainWindow = Engine::GetEngineInstance().GetMainWindow();
		InitVulkanInfo();
		SetupDebugMessenger();											// 开启验证层, 创建Vulkan实例包含在内
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}
	
	VulkanInitialize::~VulkanInitialize()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class VulkanInitialize Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void VulkanInitialize::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyDevice(m_device, nullptr);

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
		}

		/* 销毁窗口表面 */
		vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
		/* 销毁实例 */
		/* 参数1: 输出实例句柄; 参数2: 自定义内存分配器, 不为空代表需要手动销毁 */
		vkDestroyInstance(m_vkInstance, nullptr);

		*m_isAlive = false;
	}

#pragma region HelperFunction

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInitialize::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* data,
		void* userData
	)
	{
		/* 只打印警告以上级别的调试信息 */
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::cerr << "[Vulkan Message]: " << data->pMessage << std::endl;
		}

		/* 调试必须返回VK_FALSE, 否则会终止API调用 */
		return VK_FALSE;
	}

	VkResult VulkanInitialize::CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
		const VkAllocationCallbacks* allocator,
		VkDebugUtilsMessengerEXT* outMessenger
	)
	{
		PFN_vkCreateDebugUtilsMessengerEXT messengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (messengerEXT)
		{
			return messengerEXT(instance, createInfo, allocator, outMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VulkanInitialize::DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* allocator
	)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT messengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (messengerEXT)
		{
			messengerEXT(instance, messenger, allocator);
		}
	}

	void VulkanInitialize::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo)
	{
		CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		/* 设置监听的消息级别 */
		CreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		/* 设置监听的消息类型 */
		CreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		/* 绑定回调 */
		CreateInfo.pfnUserCallback = DebugCallback;
		CreateInfo.pUserData = nullptr;									// 可传入自定义数据指针
	}

	bool VulkanInitialize::CheckValidationLayerSupport()
	{
		/* 获取可用层数量 */
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		/* 获取所有可用层详情 */
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			/* 遍历所有系统可用层 */
			bool found = false;
			for (const VkLayerProperties& props : availableLayers)
			{
				if (strcmp(layerName, props.layerName) == 0)
				{
					found = true;
					break;
				}
			}

			/* 未找到指定的验证层名称返回false */
			if (!found) return false;
		}

		return true;
	}

	std::vector<const char*> VulkanInitialize::GetRequiredExtensions()
	{
		/* 获取GLFW窗口所需要的Vulkan实例扩展 */
		uint32_t glfwExtensionCount = 0;
		/* glfw自动返回当前平台渲染窗口必须的扩展列表 */
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> exts(glfwExtensions, glfwExtensions + glfwExtensionCount);

		/* 启用验证层时, 追加调试扩展 */
		if (enableValidationLayers)
		{
			exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return exts;
	}

	bool VulkanInitialize::IsDeviceSuitable(const VkPhysicalDevice& physicalDevice)
	{
		/* 查询目标设备基本属性 */
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		std::cout << "当前设备: " << deviceProperties.deviceName << std::endl;

		if (m_shouldRequiredDiscreteGPU)
		{
			/* 程序应适用于支持几何着色器的独立显卡 */
			if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
			{
				return false;
			}
		}

		/* 确保设备可以处理指定的命令 */
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		/* 查询设备是否支持扩展 (如交换链) */
		bool extensionsSuppored = CheckDeviceExtensionSupport(physicalDevice);

		/* 查询设备是否支持图像格式和呈现模式 */
		bool swapChainAdequate = false;
		if (extensionsSuppored)
		{
			SwapChainSupportDetails swapChainSupport = SwapChain::QuerySupportChainSupport(physicalDevice, m_surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.IsValid() && extensionsSuppored && swapChainAdequate && deviceFeatures.samplerAnisotropy;
	}

	VkPhysicalDeviceProperties VulkanInitialize::GetPhysicalDeviceProperties()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &physicalDeviceProperties);
		return physicalDeviceProperties;
	}

	bool VulkanInitialize::CheckDeviceExtensionSupport(const VkPhysicalDevice& physicalDevice)
	{
		/* 获取设备支持的所有扩展数量 */
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		/* 获取设备支持的所有扩展 */
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const VkExtensionProperties& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanInitialize::FindQueueFamilies(const VkPhysicalDevice& physicalDevice)
	{
		QueueFamilyIndices indices;

		/* 获取设备中队列族成员数量 */
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		/* 获取设备所有队列族信息 */
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			/* 查找支持绘制到窗口命令的队列族 */
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}
			
			/* 只要找到可处理的队列族, 跳出循环*/
			if (indices.IsValid())
			{
				break;
			}

			i++;
		}

		m_queueFamilyIndices = indices;

		return indices;
	}

	QueueFamilyIndices VulkanInitialize::FindQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
	{
		QueueFamilyIndices indices;

		/* 获取设备中队列族成员数量 */
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		/* 获取设备所有队列族信息 */
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			/* 查找支持绘制到窗口命令的队列族 */
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			/* 只要找到可处理的队列族, 跳出循环*/
			if (indices.IsValid())
			{
				break;
			}

			i++;
		}

		return indices;
	}

#pragma endregion

	void VulkanInitialize::InitVulkanInfo()
	{
		/* 填充应用程序信息, (给驱动做优化提示) */
		m_vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// 设置结构体类型
		m_vkAppInfo.pNext = nullptr;									// 扩展链, 默认空
		m_vkAppInfo.pApplicationName = "Suzza Engine";					// 应用名字
		m_vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);		// 应用版本 主. 次. 补丁
		m_vkAppInfo.pEngineName = "Suzza Engine";						// 使用的引擎名字
		m_vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		m_vkAppInfo.apiVersion = VK_API_VERSION_1_2;					// 使用的Vulkan API版本


		/* 创建实例的核心配置 */
		m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		m_vkCreateInfo.pApplicationInfo = &m_vkAppInfo;					// 绑定上面的应用信息
	}

	void VulkanInitialize::SetupDebugMessenger()
	{
		/* 注入验证层与调试配置 */
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			if (!CheckValidationLayerSupport())
			{
				throw std::runtime_error("系统不支持请求的验证层!");
			}

			/* 注入验证层列表 */
			m_vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			m_vkCreateInfo.ppEnabledLayerNames = validationLayers.data();

			/* 填充调试信使参数 */
			/* 挂载到pNext扩展链, 捕获实例创建/销毁过程的错误 */
			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			m_vkCreateInfo.pNext = &debugCreateInfo;
		}

		std::vector<const char*> exts = GetRequiredExtensions();
		/* 设置需要开启的扩展 */
		m_vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(exts.size());
		m_vkCreateInfo.ppEnabledExtensionNames = exts.data();

		/* 创建Vulkan实例 */
		CreateVulkanInstance();

		/* 创建调试信使 */
		if (enableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
			PopulateDebugMessengerCreateInfo(messengerInfo);
			if (CreateDebugUtilsMessengerEXT(m_vkInstance, &messengerInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
			{
				throw std::runtime_error("调试信使创建失败!");
			}
		}
	}

	void VulkanInitialize::CreateVulkanInstance()
	{
		/* 真正创建实例的过程 */
		/* 参数1: 创建配置; 参数2: 自定义内存分配器, 为空代表交给Vulkan管理; 参数3: 输出实例句柄 */
		VkResult result = vkCreateInstance(&m_vkCreateInfo, nullptr, &m_vkInstance);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("创建Vulkan实例失败!");
		}
	}

	void VulkanInitialize::CreateSurface()
	{
		if (m_mainWindow)
		{
			m_mainWindow->CreateWindowSurface(m_vkInstance, &m_surface);
		}
	}

	void VulkanInitialize::PickPhysicalDevice()
	{
		/* 将最终选择的显卡设备存储在该句柄中 */
		m_vkPhysicalDevice = VK_NULL_HANDLE;

		/* 获取所有GPU设备数量 */
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("无法找到支持Vulkan的GPU!");
		}

		/* 该数组保存所有的设备句柄 */
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

		for (const VkPhysicalDevice& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				/* 只要找到合适的设备即终止 */
				m_vkPhysicalDevice = device;
				break;
			}
		}

		if (m_vkPhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("无法找到合适的GPU!");
		}
	}

	void VulkanInitialize::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_vkPhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value() };

		/* 指定队列分配优先级 */
		float queuePriority = 1.0f;
		for (const uint32_t& queueFamily : uniqueQueueFamilies)
		{
			/* 指定要创建的队列 */
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		/* 指定要使用的设备特性 */
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;						// 开启采样器各项异性过滤
		
		/* 创建逻辑设备 */
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		/* 启用交换链扩展 */
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		/* 将设备开启能力链表设置给设备创建信息 */
		if (CheckDeviceFeatureSupport(m_vkPhysicalDevice))
		{
			m_deviceFeaturesChain = CreateDeviceFeaturesChain();
			createInfo.pNext = &m_deviceFeaturesChain;
		}

		/* 再次设置设备验证层, 防止被忽略 */
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		/* 创建逻辑设备 */
		if (vkCreateDevice(m_vkPhysicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("创建逻辑设备失败!");
		}

		/* 检索目标设备的队列句柄并进行缓存 */
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	bool VulkanInitialize::CheckDeviceFeatureSupport(const VkPhysicalDevice& physicalDevice)
	{
		/* 查询设备是否支持描述符索引 */
		auto queryStructureChain = vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT
		>{};

		auto& deviceFeatures2 = queryStructureChain.get<vk::PhysicalDeviceFeatures2>();
		auto& descIndexFeatureQuery = queryStructureChain.get<vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

		/* 调用接口, 驱动填充全部Features */
		vkGetPhysicalDeviceFeatures2(physicalDevice, reinterpret_cast<VkPhysicalDeviceFeatures2*>(&deviceFeatures2));

		bool featSupport = descIndexFeatureQuery.shaderSampledImageArrayNonUniformIndexing == VK_TRUE &&
			descIndexFeatureQuery.descriptorBindingVariableDescriptorCount == VK_TRUE &&
			descIndexFeatureQuery.descriptorBindingUpdateUnusedWhilePending == VK_TRUE;

		/* 至少预留256个纹理槽位 */
		bool propSupport = DeviceFunctionLibrary::GetDeviceMaxBindlessImageNum(physicalDevice) >= MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;

		/* shader里数组下标可以是变量 */
		/* 允许描述符集合更新之后, 不重新分配set; 运行时可修改数组中元素 */
		/* 允许提交后更新元素 */
		if (featSupport && propSupport)
		{
			return true;
		}
		else
		{
			throw std::runtime_error("设备无法支持所需功能!");
		}
	}

	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceDescriptorIndexingFeaturesEXT
	> VulkanInitialize::CreateDeviceFeaturesChain()
	{
		auto featureChain = vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT
		>{};

		auto& deviceFeatures2 = featureChain.get<vk::PhysicalDeviceFeatures2>();
		auto& descIndexFeature = featureChain.get<vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

		descIndexFeature.runtimeDescriptorArray = VK_TRUE;
		descIndexFeature.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		descIndexFeature.descriptorBindingVariableDescriptorCount = VK_TRUE;
		descIndexFeature.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
		descIndexFeature.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		descIndexFeature.descriptorBindingPartiallyBound = VK_TRUE;

		return featureChain;
	}
}