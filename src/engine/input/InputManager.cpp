#include "InputManager.h"

namespace suz
{
	void InputManager::SetKeywordKeyPressed(int KeyIndex, bool Pressed)
	{
		if (KeyIndex < 0 || KeyIndex >= static_cast<int>(m_keywordKeys.size())) return;
		m_keywordKeys[KeyIndex] = Pressed;
	}

	bool InputManager::IsKeywordKeyPressed(int KeyIndex)
	{
		if (KeyIndex < 0 || KeyIndex >= static_cast<int>(m_keywordKeys.size())) return false;
		return m_keywordKeys[KeyIndex];
	}

	void InputManager::SetMouseKeyPressed(int KeyIndex, bool Pressed)
	{
		if (KeyIndex < 0 || KeyIndex >= static_cast<int>(m_mouseKeys.size())) return;
		m_mouseKeys[KeyIndex] = Pressed;
	}

	bool InputManager::IsMouseKeyPressed(int KeyIndex)
	{
		if (KeyIndex < 0 || KeyIndex >= static_cast<int>(m_mouseKeys.size())) return false;
		return m_mouseKeys[KeyIndex];
	}

	void InputManager::SetMouseOldLocation(const glm::vec2& Location)
	{
		m_oldMouseLocation = Location;
	}

	const glm::vec2& InputManager::GetMouseOldLocation() const
	{
		return m_oldMouseLocation;
	}

	void InputManager::SetMouseCurrentLocation(const glm::vec2& Location)
	{
		m_currentMouseLocation = Location;
	}

	const glm::vec2& InputManager::GetMouseCurrentLocation() const
	{
		return m_currentMouseLocation;
	}
}
