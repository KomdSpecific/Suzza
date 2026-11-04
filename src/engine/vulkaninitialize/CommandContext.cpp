#include "CommandContext.h"
#include "factory/CommandPoolFactories.h"
#include "functionLibrary/VulkanFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	CommandContext::CommandContext(const DeviceContext& Context)
		: m_deviceContext(Context)
	{
	}

	CommandContext::~CommandContext()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class CommandContext Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool CommandContext::CreateCommandContext(usize MaxFlightFrames)
	{
		CommandPoolSpec commandPoolSpec = suz_commandPool::CreateDefaultCommandPoolSpec();
		if (!m_commandPool.Create(commandPoolSpec))
			return false;

		/* 每帧一个主命令缓冲(非帧缓冲) */
		m_commandBuffers.resize(MaxFlightFrames);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = m_commandPool.GetCommandPool();
		allocateInfo.commandBufferCount = static_cast<uint32>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_deviceContext.device, &allocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("分配命令缓冲区失败!");
			return false;
		}

		return true;
	}

	void CommandContext::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_commandPool.Destroy();

		*m_isAlive = false;
	}

	void CommandContext::TransitionImageLayout(const VkImage& Image, const VkFormat& Format, const VkImageLayout& OldLayout, const VkImageLayout& NewLayout)
	{
		/* 开启一次性命令缓冲 */
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		/* 图像内存屏障 */
		/* 1. 执行布局转换 2. 保证前后管线阶段的内存可见性 3. 同步资源访问 */
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = OldLayout;
		barrier.newLayout = NewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// 忽视队列族转移
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = Image;

		/* 子资源范围: 指定要转换的图像区域 */
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		/* 源/目标阶段: 访问掩码 */
		VkPipelineStageFlags sourceFlags{};								// 屏障前的管线阶段
		VkPipelineStageFlags destinationFlags{};						// 屏障后的管线阶段

		/* 场景: 从未定义布局 -> 上传纹理数据最优布局 */
		if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;									// UNDEFINED对应源端无访问权限, 旧数据直接丢弃
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// 屏障提交的命令在管线最开始完成
			destinationFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		/* 场景: 从未定义布局 -> 深度/模板附件最优布局 */
		else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;									// UNDEFINED对应源端无访问权限, 旧数据直接丢弃
			barrier.dstAccessMask =										// 目标端 深度附件设置读和写权限
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// 屏障提交的命令在管线最开始完成
			destinationFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;// 管线到达早期片段着色器测试阶段时调用屏障
		}
		/* 场景: 从上传纹理数据最优布局 -> shader采样读布局 */
		else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			SUZZA_LOG_ERROR("不受支持的图像布局转换!");
			return;
		}

		if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			/* 设置aspect掩码 */
			/* 深度布局: 默认可见面为深度面, 如果格式含模板, 额外加上模板面 */
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (VulkanFunctionLibrary::HasStencilComponent(Format))
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		/* 提交管线屏障 */
		vkCmdPipelineBarrier(commandBuffer,
			sourceFlags, destinationFlags,
			0,
			0, nullptr,						// 无内存屏障
			0, nullptr,						// 无缓冲屏障
			1, &barrier);					// 1个图像屏障

		EndSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer CommandContext::BeginSingleTimeCommands()
	{
		/* 命令缓冲区分配 */
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = m_commandPool.GetCommandPool();
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer{};
		vkAllocateCommandBuffers(m_deviceContext.device, &allocateInfo, &commandBuffer);

		/* 定义命令缓冲信息 */
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		/* 开始记录命令缓冲 */
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void CommandContext::EndSingleTimeCommands(const VkCommandBuffer& CommandBuffer)
	{
		vkEndCommandBuffer(CommandBuffer);

		/* 执行命令缓冲提交 */
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(m_deviceContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_deviceContext.graphicsQueue);

		/* 清理临时命令缓冲 */
		vkFreeCommandBuffers(m_deviceContext.device, m_commandPool.GetCommandPool(), 1, &CommandBuffer);
	}

	void CommandContext::CopyBuffer(const VkBuffer& SrcBuffer, const VkBuffer& DstBuffer, const VkDeviceSize& BufferSize)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy bufferCopy{};
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = BufferSize;
		vkCmdCopyBuffer(commandBuffer, SrcBuffer, DstBuffer, 1, &bufferCopy);

		EndSingleTimeCommands(commandBuffer);
	}

	void CommandContext::CopyBufferToImage(const VkBuffer& SrcBuffer, const VkImage& DstImage, uint32 Width, uint32 Height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy imageCopy{};
		imageCopy.bufferOffset = 0;
		imageCopy.bufferRowLength = 0;
		imageCopy.bufferImageHeight = 0;

		imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.imageSubresource.baseArrayLayer = 0;
		imageCopy.imageSubresource.mipLevel = 0;
		imageCopy.imageSubresource.layerCount = 1;

		/* 指定目标图像需要复制的部分 */
		imageCopy.imageOffset = { 0,0,0 };
		imageCopy.imageExtent = { Width,Height,1 };

		vkCmdCopyBufferToImage(commandBuffer, SrcBuffer, DstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

		EndSingleTimeCommands(commandBuffer);
	}
}