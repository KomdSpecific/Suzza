#pragma once

#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class Device;
	class SwapChain;
	class CommandContext;
	class SyncObject;

	class FrameContext
	{
	public:
		FrameContext(
			Device* InDevice,
			SwapChain* InSwapChain,
			CommandContext* InCommandContext,
			SyncObject* InSyncObject
		);
		~FrameContext() = default;

		bool BeginFrame();
		bool SubmitFrame(VkSemaphore& OutSemaphore);
		bool PresentFrame(const VkSemaphore& WaitSemaphore);

	private:
		Device* m_device;
		SwapChain* m_swapChain;
		CommandContext* m_commandContext;
		SyncObject* m_syncObject;

		/* 当前飞行帧索引 */
		uint32 m_currentFrame = 0;
		/* 当前交换链图像索引 */
		uint32 m_imageIndex = 0;

	public:
		inline uint32 GetFrameIndex() const { return m_currentFrame; }
		inline uint32 GetImageIndex() const { return m_imageIndex; }
	};
}