#include "EditorUI.h"
#include "Engine.h"
#include "imgui/ImGuiUIRender.h"
#include "macros/VkMacro.h"
#include "ReflectionInfo.h"

#ifdef EDITOR

namespace suz
{
	void EditorUI::Init(
		GLFWwindow* window,
		const VkInstance& Instance,
		const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,
		uint32 GraphicsFamily, const VkQueue& GraphicsQueue,
		const VkRenderPass& RenderPass,
		const VkCommandPool& CommandPool)
	{
		std::unique_ptr<ImGuiUIRender> uiRender = std::make_unique<ImGuiUIRender>();
		uiRender->Init(window, Instance, PhysicalDevice, Device, GraphicsFamily, GraphicsQueue, RenderPass, CommandPool);
		m_uiRender = std::unique_ptr<ImGuiUIRender>(std::move(uiRender));

		/* 执行引擎全局广播委托 */
		EngineUIRenderEventDelegate& engineDelegate = Engine::GetEngineInstance().GetEngineUIRenderEventDelegate();
		engineDelegate.Bind([this](const VkCommandBuffer& CommandBuffer)
			{
				SuzzaApplication* application = Engine::GetEngineInstance().GetApplication();

				this->DrawSurface(application);
				this->RecordDrawCommandBuffer(CommandBuffer);
			});
	}

	void EditorUI::BeginFrame()
	{
		m_uiRender->BeginFrame();
	}

	void EditorUI::DrawSurface(SuzzaApplication* Application)
	{
		DrawHierarchyPanel(Application);
		DrawInspectorPanel();
	}

	void EditorUI::RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer)
	{
		m_uiRender->RecordDrawCommandBuffer(CommandBuffer);
	}

	void EditorUI::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_uiRender->Destroy();

		*m_isAlive = false;
	}

	void EditorUI::DrawHierarchyPanel(SuzzaApplication* Application)
	{
		m_uiRender->BeginWindow("Hierarchy");

		Scene* scene = Application->GetScene();
		for (Actor* rootActor : scene->GetSceneRootActorList())
		{
			DrawActorTreeNode(rootActor);
		}

		m_uiRender->EndWindow();
	}

	void EditorUI::DrawInspectorPanel()
	{
		if (!m_selectedActor) return;

		if (m_uiRender->BeginWindow("Inspector"))
		{
			for (const FieldInfo& field : m_selectedActor->GetClassInfo().fields)
			{
				if (field.typeName == "glm::vec3")
				{
					glm::vec3 location{};
					(field.getterFunc(m_selectedActor, &location));
					float loc[3] = { location.x,location.y,location.z };
					if (m_uiRender->ShowVector3Float(field.name.c_str(), loc))
					{
						field.setterFunc(m_selectedActor, loc);
					}
				}
			}

			/* 遍历所有的Actor组件信息 */
			for (Component* comp : m_selectedActor->GetAllActorComponents())
			{
				ClassInfo classInfo = comp->GetClassInfo();
				m_uiRender->SeparatorText(classInfo.className.c_str());

				for (const FieldInfo& field : classInfo.fields)
				{
					DrawComponentField(comp, field);
				}
			}
		}

		m_uiRender->EndWindow();
	}

	void EditorUI::DrawActorTreeNode(Actor* TargetActor)
	{
		bool isSelected = (TargetActor == m_selectedActor);

		uint32 flags = UITreeNodeFlags_OpenOnArrow | UITreeNodeFlags_SpanAvailWidth | UITreeNodeFlags_Framed;
		if (isSelected)
		{
			/* 标记为已选 */
			flags |= UITreeNodeFlags_Selected;
		}

		if (TargetActor->GetChildActorList().empty())
		{
			/* 标记为叶子节点, 叶子节点会自动展开节点信息 */
			//flags |= UITreeNodeFlags_Leaf;
		}

		/* 绘制TreeNode */
		bool nodeOpen = m_uiRender->TreeNodeEx(TargetActor->GetActorName().c_str(), flags);

		/* 业务: 点击事件处理, 选中菜单中的Actor */
		if (m_uiRender->IsItemLeftClicked()) m_selectedActor = TargetActor;

		/* 右键菜单 */
		if (m_uiRender->BeginPopupContextItem())
		{
			if (m_uiRender->MenuItem("Delete Actor"))
			{
				if (m_selectedActor == TargetActor)
				{
					m_selectedActor = nullptr;
				}
				TargetActor->Destroy();
			}

			m_uiRender->EndPopup();
		}

		if (nodeOpen)
		{
			for (Actor* childActor : TargetActor->GetChildActorList())
			{
				DrawActorTreeNode(childActor);
			}

			m_uiRender->TreePop();
		}
	}

	void EditorUI::DrawComponentField(Component* CurrentComponent, const FieldInfo& TargetField)
	{
		if (TargetField.typeName == "float")
		{
			float value = 0;
			TargetField.getterFunc(CurrentComponent, &value);
			if (m_uiRender->ShowFloat(TargetField.name.c_str(), &value))
			{
				TargetField.setterFunc(CurrentComponent, &value);
			}
		}
	}
}
#endif