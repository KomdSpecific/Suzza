#pragma once

#include <memory>

#include <vector>

namespace suz
{
	class BaseObject
	{
	public:
		BaseObject() : m_isAlive(std::make_shared<bool>(true)) {}
		virtual ~BaseObject() = default;

		/* ½ûÖ¹¿½±´ */
		BaseObject(const BaseObject&) = delete;
		BaseObject& operator=(const BaseObject&) = delete;

		virtual void Destroy() = 0;

		inline bool IsDestroyed() const { return !(*m_isAlive); }
		inline bool IsAlive() const { return *m_isAlive; }
		inline std::shared_ptr<bool> GetValidPtr() { return m_isAlive; }

	protected:
		std::shared_ptr<bool> m_isAlive;
	};
}