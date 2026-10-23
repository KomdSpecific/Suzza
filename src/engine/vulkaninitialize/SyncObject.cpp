#include "SyncObject.h"
#include "macros/VkMacro.h"

#include "debug/SuzzaLog.h"

#include <stdexcept>

namespace suz
{
	SyncObject::SyncObject(const VkDevice& Device)
		: m_device(Device)
	{
		CreateSyncObjects();
	}

	SyncObject::~SyncObject()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class SyncObject Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void SyncObject::Destroy()
	{
		if (!(*m_isAlive)) return;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device, m_imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_device, m_inFrameFence[i], nullptr);
		}

		*m_isAlive = false;
	}

	void SyncObject::CreateSyncObjects()
	{
		m_imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFrameFence.resize(MAX_FRAMES_IN_FLIGHT);

		/* 创建信号量信息 */
		VkSemaphoreCreateInfo semapCreateInfo{};
		semapCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		/* 创建围栏信息 */
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;						// 防止第一帧不存在围栏信号造成无限阻塞!

		for (size_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
		{
			if (vkCreateSemaphore(m_device, &semapCreateInfo, nullptr, &m_imageAvailableSemaphore[frameIndex]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device, &semapCreateInfo, nullptr, &m_renderFinishedSemaphore[frameIndex]) != VK_SUCCESS ||
				vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFrameFence[frameIndex]) != VK_SUCCESS)
			{
				throw std::runtime_error("创建信号量和围栏失败!");
			}
		}
	}
}