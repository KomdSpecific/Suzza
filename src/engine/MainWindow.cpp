#include "MainWindow.h"
#include "Engine.h"

#include <iostream>

namespace suz
{
	void KeywordCallback(GLFWwindow* Window, int Key, int, int Action, int)
	{
		InputManager& inputManager = Engine::GetEngineInstance().GetMainWindow()->GetInputManager();
		if (Action == GLFW_PRESS)
		{
			inputManager.SetKeywordKeyPressed(Key, true);
		}
		else if (Action == GLFW_RELEASE)
		{
			inputManager.SetKeywordKeyPressed(Key, false);
		}
	}

	void MouseKeyCallback(GLFWwindow* Window, int Key, int Action, int)
	{
		InputManager& inputManager = Engine::GetEngineInstance().GetMainWindow()->GetInputManager();
		if (Action == GLFW_PRESS)
		{
			inputManager.SetMouseKeyPressed(Key, true);
		}
		else if (Action == GLFW_RELEASE)
		{
			inputManager.SetMouseKeyPressed(Key, false);
		}
	}

	void MouseLocationChangedCallback(GLFWwindow* Window, double XPosition, double YPosition)
	{
		InputManager& inputManager = Engine::GetEngineInstance().GetMainWindow()->GetInputManager();

		inputManager.SetMouseOldLocation(inputManager.GetMouseCurrentLocation());
		inputManager.SetMouseCurrentLocation(glm::vec2(XPosition, YPosition));
	}

	MainWindow::MainWindow(int Width, int Height, const std::string& WindowName)
		: m_width(Width), m_height(Height), m_windowName(WindowName)
	{
		InitWindow();

		/* 设置glfw输入回调函数 */
		glfwSetKeyCallback(m_window, KeywordCallback);
		glfwSetMouseButtonCallback(m_window, MouseKeyCallback);
		glfwSetCursorPosCallback(m_window, MouseLocationChangedCallback);
	}

	MainWindow::~MainWindow()
	{
		if (*m_isAlive)
		{
			std::cerr << "Class MainWindow Warning: 未调用Destroy手动销毁!" << std::endl;
			Destroy();
		}
	}

	void MainWindow::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_windowChangedDelegate.RemoveAll();

		glfwDestroyWindow(m_window);
		glfwTerminate();

		*m_isAlive = false;
	}

	bool MainWindow::CreateWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
		{
			std::cerr << "创建窗口表面失败!" << std::endl;
			return false;
		}

		return true;
	}

	void MainWindow::ResetWindowExtent()
	{
		glfwGetFramebufferSize(m_window, &m_width, &m_height);
	}

	void MainWindow::BindOnFrameBufferResizeCallback()
	{
		/* 绑定窗口大小变化回调函数 */
		glfwSetFramebufferSizeCallback(m_window, OnFrameBufferResized);
	}

	void MainWindow::OnFrameBufferResized(GLFWwindow* Window, int Width, int Height)
	{
		Engine::GetEngineInstance().GetMainWindow()->m_windowChangedDelegate.Broadcast(Width, Height);
	}

	void MainWindow::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);				// 禁止创建OpenGL上下文

		m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
	}
}