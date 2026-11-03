#include "CommandPool.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkCommandPoolCreateInfo CommandPoolSpec::DefaultCommandPoolCreateInfo(const CommandPoolSpec& Spec)
	{
		/* 创建命令池 */
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;		// 允许单独重新记录命令缓冲区
		commandPoolInfo.queueFamilyIndex = Spec.deviceContext.graphicsFamilyIndex;

		return commandPoolInfo;
	}

	CommandPool::~CommandPool()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class CommandPool Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool CommandPool::Create(CommandPoolSpec& Spec)
	{
		return CreateCommandPool(Spec);
	}

	bool CommandPool::CreateCommandPool(CommandPoolSpec& Spec)
	{
		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建命令池: 逻辑设备device不合法!");
			return false;
		}

		m_deviceContext = Spec.deviceContext;

		VkCommandPoolCreateInfo commandPoolInfo = CommandPoolSpec::DefaultCommandPoolCreateInfo(Spec);
		if (vkCreateCommandPool(m_deviceContext.device, &commandPoolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建命令池失败!");
			return false;
		}

		return true;
	}

	void CommandPool::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyCommandPool(m_deviceContext.device, m_commandPool, nullptr);

		*m_isAlive = false;
	}
}