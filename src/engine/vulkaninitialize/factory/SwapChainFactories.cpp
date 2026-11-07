#include "SwapChainFactories.h"
#include "Engine.h"
#include "MainWindow.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/functionLibrary/DeviceFunctionLibrary.h"
#include "vulkaninitialize/functionLibrary/VulkanFunctionLibrary.h"

namespace suz
{
	MainWindow* mainWindow = Engine::GetEngineInstance().GetMainWindow();

	namespace suz_swapchain
	{
		SwapChainSpec CreateDefaultSwapChain()
		{
			DeviceContext deviceContext = Engine::GetVulkanResourceManager().GetDeviceContext();
			const VkSurfaceKHR& surface = Engine::GetVulkanResourceManager().GetVulkanInstance()->GetSurface();

			SwapChainSpec spec(deviceContext, surface);
			spec.familyIndices = DeviceFunctionLibrary::FindQueueFamilies(deviceContext.physicalDevice, surface);
			spec.swapChainDetails = DeviceFunctionLibrary::QuerySwapChainSupport(deviceContext.physicalDevice, surface);
			spec.surfaceFormat = VulkanFunctionLibrary::ChooseSuitableSurfaceFormatKHR(spec.swapChainDetails.formats);
			spec.presentMode = VulkanFunctionLibrary::ChooseSuitablePresentModeKHR(spec.swapChainDetails.presentModes);
			spec.swapChainExtent = VulkanFunctionLibrary::GetSwapChainExtent(spec.swapChainDetails.capabilities, mainWindow);

			return spec;
		}
	}
}