#pragma once

#include "SuzzaTypes.h"
#include "DeviceContext.h"
#include "CommandPool.h"

#include <vulkan/vulkan.h>

namespace suz
{
	/* 命令缓冲的唯一出口: 命令池 帧命令缓冲 一次性命令 布局转换 拷贝 */
	class CommandContext
	{
	public:
		CommandContext(const DeviceContext& Context);
		~CommandContext();

		/* 转换图像布局到期望值 */
		void TransitionImageLayout(const VkImage& Image, const VkFormat& Format, const VkImageLayout& OldLayout, const VkImageLayout& NewLayout);

#pragma region SINGLE_TIME_COMMAND
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(const VkCommandBuffer& CommandBuffer);

#pragma endregion

#pragma region Copy
		/* 复制缓冲区 */
		void CopyBuffer(const VkBuffer& SrcBuffer, const VkBuffer& DstBuffer, const VkDeviceSize& BufferSize);

#pragma endregion

	private:
		DeviceContext m_deviceContext = {};
		CommandPool m_commandPool = {};

	public:
		inline const VkCommandPool& GetCommandPool() const { return m_commandPool.GetCommandPool(); }
	};
}