#pragma once

#include <array>
#include <glm/vec2.hpp>

namespace suz
{
	class InputManager
	{
	private:
		InputManager() = default;
		InputManager(InputManager&) = delete;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

	public:
		void SetKeywordKeyPressed(int KeyIndex, bool Pressed);
		bool IsKeywordKeyPressed(int KeyIndex);

		void SetMouseKeyPressed(int KeyIndex, bool Pressed);
		bool IsMouseKeyPressed(int KeyIndex);

		void SetMouseOldLocation(const glm::vec2& Location);
		const glm::vec2& GetMouseOldLocation() const;

		void SetMouseCurrentLocation(const glm::vec2& Location);
		const glm::vec2& GetMouseCurrentLocation() const;

	private:
		std::array<bool, 256> m_keywordKeys = { false };
		std::array<bool, 16> m_mouseKeys = { false };

		glm::vec2 m_oldMouseLocation = glm::vec2(0.0f);
		glm::vec2 m_currentMouseLocation = glm::vec2(0.0f);

		friend class MainWindow;
	};
}