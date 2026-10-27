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

	bool ImGuiUIRender::TreeNodeEx(const char* Label, uint32 flags)
	{
		return ImGui::TreeNodeEx(Label, flags);
	}

	void ImGuiUIRender::TreePop()
	{
		ImGui::TreePop();
	}

	bool ImGuiUIRender::IsItemLeftClicked()
	{
		return ImGui::IsItemClicked(ImGuiMouseButton_Left);
	}

	bool ImGuiUIRender::IsItemRightClicked()
	{
		return ImGui::IsItemClicked(ImGuiMouseButton_Right);
	}

	bool ImGuiUIRender::BeginPopupContextItem()
	{
		return ImGui::BeginPopupContextItem();
	}

	void ImGuiUIRender::EndPopup()
	{
		ImGui::EndPopup();
	}

	bool ImGuiUIRender::MenuItem(const char* Label)
	{
		return ImGui::MenuItem(Label);
	}

	void ImGuiUIRender::Text(const char* Format, ...)
	{
		va_list valueList{};
		va_start(valueList, Format);
		ImGui::TextV(Format, valueList);
		va_end(valueList);
	}

	void ImGuiUIRender::SeparatorText(const char* Label)
	{
		ImGui::SeparatorText(Label);
	}

	bool ImGuiUIRender::Button(const char* Label)
	{
		return ImGui::Button(Label);
	}

	bool ImGuiUIRender::ShowFloat(const char* Label, float* Value, const float Speed)
	{
		return ImGui::DragFloat(Label, Value, Speed);
	}

	bool ImGuiUIRender::ShowVector2Float(const char* Label, const glm::vec2& Vector, const float Speed)
	{
		float v[2] = { Vector.x,Vector.y };
		return ImGui::DragFloat2(Label, v, Speed);
	}

	bool ImGuiUIRender::ShowVector3Float(const char* Label, const glm::vec3& Vector, const float Speed)
	{
		float v[3] = { Vector.x,Vector.y,Vector.z };
		return ImGui::DragFloat3(Label, v, Speed);
	}

	bool ImGuiUIRender::ShowVector2Float(const char* Label, float* Vector, const float Speed)
	{
		return ImGui::DragFloat2(Label, Vector, Speed);
	}

	bool ImGuiUIRender::ShowVector3Float(const char* Label, float* Vector, const float Speed)
	{
		return ImGui::DragFloat3(Label, Vector, Speed);
	}

	void ImGuiUIRender::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
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
}
#endif
