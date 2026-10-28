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
				DrawObjField(m_selectedActor, field);
			}

			/* 遍历所有的Actor组件信息 */
			for (Component* comp : m_selectedActor->GetAllActorComponents())
			{
				ClassInfo classInfo = comp->GetClassInfo();
				m_uiRender->SeparatorText(classInfo.className.c_str());

				for (const FieldInfo& field : classInfo.fields)
				{
					DrawObjField(comp, field);
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

	void EditorUI::DrawObjField(void* TargetObj, const FieldInfo& TargetField)
	{
		switch (TargetField.category)
		{
		case suz::ETypeCategory::Primitive:
		{
			DrawPrimitiveField(TargetObj, TargetField);
			break;
		}

		case suz::ETypeCategory::Enum:
			break;
		case suz::ETypeCategory::Struct:
			break;
		case suz::ETypeCategory::Array:
			break;
		case suz::ETypeCategory::Pointer:
			break;
		case suz::ETypeCategory::Reference:
			break;
		case suz::ETypeCategory::Container:
		{
			DrawContainerField(TargetObj, TargetField);
			break;
		}

		case suz::ETypeCategory::Unknown:
			break;
		default:
			break;
		}
	}

	void EditorUI::DrawPrimitiveField(void* TargetObj, const FieldInfo& TargetField)
	{
		switch (TargetField.primitiveType)
		{
		case suz::EPrimitiveType::Bool:
			break;
		case suz::EPrimitiveType::Int8:
			break;
		case suz::EPrimitiveType::UInt8:
			break;
		case suz::EPrimitiveType::Int16:
			break;
		case suz::EPrimitiveType::UInt16:
			break;
		case suz::EPrimitiveType::Int32:
			break;
		case suz::EPrimitiveType::UInt32:
			break;
		case suz::EPrimitiveType::Int64:
			break;
		case suz::EPrimitiveType::UInt64:
			break;
		case suz::EPrimitiveType::Float:
		{
			float value = 0;
			TargetField.getterFunc(TargetObj, &value);
			if (m_uiRender->ShowFloat(TargetField.name.c_str(), &value, 0.1f))
			{
				TargetField.setterFunc(TargetObj, &value);
			}
			break;
		}

		case suz::EPrimitiveType::Unknown:
			break;
		default:
			break;
		}
	}

	void EditorUI::DrawContainerField(void* TargetObj, const FieldInfo& TargetField)
	{
		switch (TargetField.containerType)
		{
		case suz::EContainerType::Vector3:
		{
			glm::vec3 vector3{};
			TargetField.getterFunc(TargetObj, &vector3);
			float v[3] = { vector3.x,vector3.y,vector3.z };

			if (m_uiRender->ShowVector3Float(TargetField.name.c_str(), v))
			{
				TargetField.setterFunc(TargetObj, v);
			}
			break;
		}

		case suz::EContainerType::Vector4:
			break;

		case suz::EContainerType::Quat:
		{
			glm::vec3 vector3{};
			TargetField.getterFunc(TargetObj, &vector3);
			float v[3] = { vector3.x,vector3.y,vector3.z };

			if (m_uiRender->ShowVector3Float(TargetField.name.c_str(), v))
			{
				TargetField.setterFunc(TargetObj, v);
			}
			break;
		}

		case suz::EContainerType::Matrix3:
			break;
		case suz::EContainerType::Matrix4:
			break;
		case suz::EContainerType::String:
			break;
		case suz::EContainerType::Vector:
			break;
		case suz::EContainerType::Map:
			break;
		case suz::EContainerType::Set:
			break;
		case suz::EContainerType::Unknown:
			break;
		default:
			break;
		}
	}
}
#endif