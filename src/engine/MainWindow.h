#pragma once

#include "BaseObject.h"
#include "input/InputManager.h"
#include "MultipleDelegateDeclareMacro.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

DECLARE_MULTIPLE_DELEGATE_TWO_PARAMS(WindowSizeChangedDelegate, int, int)

namespace suz
{
	class BufferArea;

	class MainWindow : public BaseObject
	{
	public:
		MainWindow(int Width, int Height, const std::string& WindowName);
		~MainWindow();

		virtual void Destroy() override;

		bool WindowShouldClose() { return glfwWindowShouldClose(m_window); }

		void CreateWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface);

		/* 获取当前窗口的分辨率 */
		inline VkExtent2D GetWindowExtent() { return { static_cast<uint32_t>(m_width),static_cast<uint32_t>(m_height) }; }
		void ResetWindowExtent();

		void BindOnFrameBufferResizeCallback(BufferArea* Bufferarea);

		static void OnFrameBufferResized(GLFWwindow* Window, int Width, int Height);

	private:
		void InitWindow();

		int m_width = 0;
		int m_height = 0;
		std::string m_windowName = {};
		GLFWwindow* m_window = nullptr;

		WindowSizeChangedDelegate m_windowChangedDelegate;

		InputManager m_inputManager;

	public:
		inline GLFWwindow* GetMainWindow() { return m_window; }
		inline InputManager& GetInputManager() { return m_inputManager; }
		inline WindowSizeChangedDelegate& GetWindowSizeChangedDelegate() { return m_windowChangedDelegate; }
	};
}