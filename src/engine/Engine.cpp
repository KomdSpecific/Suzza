#include "Engine.h"
#include "Filesystem.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "model/ModelResourceManager.h"
#include "ImageLoaderManager.h"
#include "commonmacro/VkMacro.h"
#include "GameApplication.h"

#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace suz
{
	/* Vulkan资源管理器分配内存 */
	VulkanResourceManager Engine::m_vkResourceManager;
	/* 模型资源管理器分配内存 */
	ModelResourceManager Engine::m_modelResourceManager;
	/* 图像加载管理器分配内存 */
	ImageLoaderManager Engine::m_imageLoaderManager;
	/* 图像Bindless插槽管理器分配内存 */
	BindlessSlotsAllocator Engine::m_bindlessSlotsAllocator;

	Engine& Engine::GetEngineInstance()
	{
		static Engine instance;
		return instance;
	}

	VulkanResourceManager& Engine::GetVulkanResourceManager()
	{
		return m_vkResourceManager;
	}

	ModelResourceManager& Engine::GetModelResourceManager()
	{
		return m_modelResourceManager;
	}

	ImageLoaderManager& Engine::GetImageLoaderManager()
	{
		return m_imageLoaderManager;
	}

	BindlessSlotsAllocator& Engine::GetBindlessSlotsAllocator()
	{
		return m_bindlessSlotsAllocator;
	}

	bool Engine::Init()
	{
		/* 图像加载管理器创建调度器 */
		m_imageLoaderManager.CreateImageDispatcher();

		/* 创建主窗口 */
		m_mainWindow = std::make_unique<MainWindow>(WIDTH, HEIGHT, "First Window");

		/* Vulkan初始化 */
		m_vkResourceManager.InitVulkan();

		/* 显式处理窗口大小调整, 某些情况导致窗口大小更改但交换链没有重建导致无法渲染整个窗口 */
		m_mainWindow->BindOnFrameBufferResizeCallback(m_vkResourceManager.GetBufferArea());

		/* 渲染每帧实例初始化 */
		m_render = std::make_unique<Render>();

		return m_gameApplication->Init();
	}

	void Engine::Run()
	{
		m_lastTime = std::chrono::high_resolution_clock::now();
		InputManager& inputManager = Engine::GetEngineInstance().GetMainWindow()->GetInputManager();

		m_gameApplication->BeginPlay();

		while (!m_mainWindow->WindowShouldClose())
		{
			/* 自动检查和处理窗口事件 */
			glfwPollEvents();

			auto now = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
			m_lastTime = now;

			m_gameApplication->Update(deltaTime);

			m_render->Draw(deltaTime);

			/* 保证鼠标旧坐标和当前坐标相等 */
			inputManager.SetMouseOldLocation(inputManager.GetMouseCurrentLocation());
		}

		/* 等待逻辑设备完成收尾操作 */
		vkDeviceWaitIdle(m_vkResourceManager.GetVulkanInitialize()->GetDevice());
	}

	void Engine::Destroy()
	{
		m_gameApplication->Destroy();

		/* 模型资源管理器统一销毁 */
		m_modelResourceManager.Destroy();
		/* 图像Bindless插槽统一释放 */
		m_bindlessSlotsAllocator.Destroy();
		/* 调用Vulkan资源管理器统一销毁 */
		m_vkResourceManager.Destroy();

		m_mainWindow->Destroy();
	}

	void Engine::SetApplication(GameApplication* gameApp)
	{
		m_gameApplication.reset(gameApp);
	}

	GameApplication* Engine::GetApplication()
	{
		return m_gameApplication.get();
	}
}