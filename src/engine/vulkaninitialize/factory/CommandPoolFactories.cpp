#include "CommandPoolFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"

namespace suz
{
	namespace suz_commandPool
	{
		CommandPoolSpec CreateDefaultCommandPoolSpec()
		{
			const DeviceContext& deviceContext = Engine::GetVulkanResourceManager().GetDeviceContext();
			CommandPoolSpec spec(deviceContext);
			
			return spec;
		}
	}
}