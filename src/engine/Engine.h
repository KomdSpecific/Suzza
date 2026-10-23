#pragma once

#include "MainWindow.h"
#include "model/Render.h"

#include <memory>
#include <chrono>

class GameApplication;

namespace suz
{
	class VulkanResourceManager;
	class ModelResourceManager;
	class ImageLoaderManager;
	class ModelLoaderManager;
	class BindlessSlotsAllocator;

	class Engine
	{
	public:
		static Engine& GetEngineInstance();
		static VulkanResourceManager& GetVulkanResourceManager();
		static ModelResourceManager& GetModelResourceManager();
		static ImageLoaderManager& GetImageLoaderManager();
		static ModelLoaderManager& GetModelLoaderManager();
		static BindlessSlotsAllocator& GetBindlessSlotsAllocator();

	private:
		Engine() = default;
		Engine(Engine&&) = delete;
		Engine(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;

	public:
		 bool Init();

		 void Run();

		 void Destroy();

		 int WIDTH = 800;
		 int HEIGHT = 600;

	private:
		/* Vulkan资源管理器, 负责管理Vulkan的资源(交换链、管线、缓冲区等)生命周期 */
		static VulkanResourceManager m_vkResourceManager;
		/* 模型资源管理器 */
		static ModelResourceManager m_modelResourceManager;
		/* 图像加载管理器 */
		static ImageLoaderManager m_imageLoaderManager;
		/* 模型加载管理器 */
		static ModelLoaderManager m_modelLoaderManager;
		/* 图像Bindless插槽管理器 */
		static BindlessSlotsAllocator m_bindlessSlotsAllocator;
		/* 引擎窗口 */
		std::unique_ptr<MainWindow> m_mainWindow = nullptr;
		/* 游戏应用 */
		std::unique_ptr<GameApplication> m_gameApplication = nullptr;
		/* 存储上一次记录的时间 */
		std::chrono::steady_clock::time_point m_lastTime;
		/* 渲染每帧 */
		std::unique_ptr<Render> m_render;

	public:
		void SetApplication(GameApplication* gameApp);
		GameApplication* GetApplication();

		inline MainWindow* GetMainWindow() { return m_mainWindow.get(); }
		inline Render* GetRender() { return m_render.get(); }
	};
}