#pragma once

#include "BaseObject.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace suz
{
	class SyncObject : public BaseObject
	{
	public:
		SyncObject(const VkDevice& Device);
		~SyncObject();

		virtual void Destroy() override;

	private:
		void CreateSyncObjects();

		const VkDevice& m_device;
		
		std::vector<VkSemaphore> m_imageAvailableSemaphore = {};
		std::vector<VkSemaphore> m_renderFinishedSemaphore = {};
		std::vector<VkFence> m_inFrameFence = {};

	public:
		inline const std::vector<VkSemaphore>& GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
		inline const std::vector<VkSemaphore>& GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }
		inline const std::vector<VkFence>& GetInFrameFence() const { return m_inFrameFence; }
	};
}