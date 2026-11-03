#pragma once

#include <memory>

namespace suz
{
	class VulkanResourceBase
	{
	public:
		VulkanResourceBase() : m_isAlive(std::make_shared<bool>(true)) {}
		virtual ~VulkanResourceBase() = default;

		/* 禁止拷贝 */
		VulkanResourceBase(const VulkanResourceBase&) = delete;
		VulkanResourceBase& operator=(const VulkanResourceBase&) = delete;

		inline bool IsDestroyed() const { return !(*m_isAlive); }
		inline bool IsAlive() const { return *m_isAlive; }
		inline std::shared_ptr<bool> GetValidPtr() { return m_isAlive; }

	protected:
		std::shared_ptr<bool> m_isAlive;
	};
}