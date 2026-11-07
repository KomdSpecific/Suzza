#include "VulkanInstanceFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/functionLibrary/VulkanFunctionLibrary.h"

namespace suz
{
	namespace suz_instance
	{
		VulkanInstanceSpec CreateDefaultVulkanInstance()
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();
			
			VulkanInstanceSpec spec(Engine::GetEngineInstance().GetMainWindow(), manager.VALIDATION_LAYERS);
			spec.appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// 设置结构体类型
			spec.appInfo.pApplicationName = "Suzza Engine";						// 应用名字
			spec.appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);			// 应用版本 主. 次. 补丁
			spec.appInfo.pEngineName = "Suzza Engine";							// 使用的引擎名字
			spec.appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			spec.appInfo.apiVersion = VK_API_VERSION_1_2;						// 使用的Vulkan API版本
			spec.appInfo.pNext = nullptr;										// 扩展链, 默认空

			spec.requiredExtensions = VulkanFunctionLibrary::GetRequiredExtensions();

			spec.debugMessengerCreateInfo = VulkanInstanceSpec::DefaultDebugMessengerCreateInfo();

			return spec;
		}
	}
}