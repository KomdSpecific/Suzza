#include "ImGuiUIRender.h"
#include "macros/VkMacro.h"

#include <cstdarg>

#ifdef EDITOR

namespace suz
{
	bool ImGuiUIRender::BeginWindow(const char* Title)
	{
		return ImGui::Begin(Title);
	}

	void ImGuiUIRender::EndWindow()
	{
		ImGui::End();
	}

	void ImGuiUIRender::Text(const char* Format, ...)
	{
		va_list valueList{};
		va_start(valueList, Format);
		ImGui::TextV(Format, valueList);
		va_end(valueList);
	}

	bool ImGuiUIRender::Button(const char* Label)
	{
		return ImGui::Button(Label);
	}

	void ImGuiUIRender::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiUIRender::DrawSurfacePanels(SuzzaApplication* Application)
	{
		BeginWindow("Suzza Engine Editor");
		Text("Actor Count: %d", 2);
		if (Button("Spawn Actor"))
		{
			printf("Spawn Actor Success!");
		}

		EndWindow();

		DrawHierarchyPanel(Application);
	}

	void ImGuiUIRender::RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer)
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);
	}

#pragma region LifeCycle
	void ImGuiUIRender::Init(GLFWwindow* window,
		const VkInstance& Instance,
		const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,
		uint32 GraphicsFamily, const VkQueue& GraphicsQueue,
		const VkRenderPass& RenderPass,
		const VkCommandPool& CommandPool)
	{
		IMGUI_CHECKVERSION();								// 检查Imgui版本
		ImGui::CreateContext();								// 创建Imgui上下文

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		/* 绑定GLFW窗口 */
		ImGui_ImplGlfw_InitForVulkan(window, true);

		/* Vulkan初始化结构体 */
		ImGui_ImplVulkan_InitInfo vk_init_info{};
		vk_init_info.Instance = Instance;
		vk_init_info.PhysicalDevice = PhysicalDevice;
		vk_init_info.Device = Device;
		vk_init_info.QueueFamily = GraphicsFamily;
		vk_init_info.Queue = GraphicsQueue;
		vk_init_info.PipelineCache = VK_NULL_HANDLE;
		vk_init_info.DescriptorPoolSize = 1000;
		vk_init_info.PipelineInfoMain.RenderPass = RenderPass;
		vk_init_info.MinImageCount = 2;
		vk_init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;

		ImGui_ImplVulkan_Init(&vk_init_info);
	}

	void ImGuiUIRender::Destroy()
	{
		if (!(*m_isAlive)) return;

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		*m_isAlive = false;
	}

#pragma endregion

	void ImGuiUIRender::DrawHierarchyPanel(SuzzaApplication* Application)
	{
		BeginWindow("Hierarchy");

		EndWindow();
	}
}
#endif
