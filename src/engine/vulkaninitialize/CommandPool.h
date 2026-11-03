#pragma once

#include "DeviceContext.h"
#include "VulkanResourceBase.h"

#include <vulkan/vulkan.h>

namespace suz
{
	struct CommandPoolSpec
	{
		CommandPoolSpec(const DeviceContext& Context)
			: deviceContext(Context) {
		};

		DeviceContext deviceContext = {};

		static VkCommandPoolCreateInfo DefaultCommandPoolCreateInfo(const CommandPoolSpec& Spec);
	};

	class CommandPool : public VulkanResourceBase
	{
	public:
		CommandPool() = default;
		~CommandPool();

		bool Create(CommandPoolSpec& Spec);
		bool CreateCommandPool(CommandPoolSpec& Spec);

		void Destroy();

	private:
		DeviceContext m_deviceContext = {};

		VkCommandPool m_commandPool = {};

	public:
		inline const VkCommandPool& GetCommandPool() const { return m_commandPool; }
	};
}