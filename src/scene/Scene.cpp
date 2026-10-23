#include "Scene.h"
#include "Engine.h"
#include "model/ModelResourceManager.h"
#include "ModelLoaderManager.h"
#include "components/MeshComponent.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	void CopyVertexDataStruct(std::vector<ModelVertexData> SrcData, std::vector<Vertex>& DstData);

	bool Scene::RegisterActor(Actor* TargetActor)
	{
		if (!TargetActor) return false;

		/* 防止重复注册 */
		for (std::unique_ptr<Actor>& actor : m_actorList)
		{
			if (actor.get() == TargetActor) return false;
		}

		/* 默认注册根Actor */
		m_actorList.push_back(std::unique_ptr<Actor>(TargetActor));
		m_rootActorList.push_back(TargetActor);

		TargetActor->SetOwnerScene(this);

		return true;
	}

	bool Scene::RegisterActor(Actor* TargetActor, Actor* ParentActor, bool bRegisterComponents)
	{
		if (!TargetActor) return false;

		/* 第一种情况, 新的父Actor为空 */
		if (!ParentActor)
		{
			/* 1. 当前Actor在场景中已存在且不是场景根节点 */
			if (Actor* oldParentActor = TargetActor->GetParentActor())
			{
				/* 注册Actor为根节点 */
				TargetActor->SetParentActor(nullptr);
				oldParentActor->RemoveChildActor(TargetActor);

				m_rootActorList.push_back(TargetActor);
				return true;
			}
			/* 2. 当前Actor刚刚创建 */
			else
			{
				/* 防止重复注册 */
				for (std::unique_ptr<Actor>& actor : m_actorList)
				{
					if (actor.get() == TargetActor)
					{
						return false;
					}
				}

				m_actorList.push_back(std::unique_ptr<Actor>(TargetActor));
				m_rootActorList.push_back(TargetActor);
				
				TargetActor->SetOwnerScene(this);

				return true;
			}
		}
		/* 第二种情况, 新的父Actor不为空 */
		else
		{
			/* 1. 当前Actor在场景中已存在且不是场景根节点 */
			if (Actor* oldParent = TargetActor->GetParentActor())
			{
				if (CheckWouldCreateCycle(TargetActor, ParentActor))
				{
					SUZZA_LOG_ERROR("在新父祖先中发现了目标Actor, 更换失败!");
					return false;
				}

				oldParent->RemoveChildActor(TargetActor);
				TargetActor->SetParentActor(ParentActor);
				ParentActor->RegisterChildActor<Actor>(TargetActor);
				return true;
			}
			else
			{
				/* 查找根节点中是否存在目标Actor */
				auto findActor = std::find_if(m_rootActorList.begin(), m_rootActorList.end(),
					[TargetActor](Actor* actor)
					{
						return TargetActor == actor;
					});

				if (findActor != m_rootActorList.end())
				/* 2. Actor为场景根节点 */
				{
					m_rootActorList.erase(findActor);
					TargetActor->SetParentActor(ParentActor);
					ParentActor->RegisterChildActor<Actor>(TargetActor);
					return true;
				}
				/* 3. Actor刚刚创建, 场景不存在目标Actor */
				else
				{
					TargetActor->SetParentActor(ParentActor);
					ParentActor->RegisterChildActor<Actor>(TargetActor);

					TargetActor->SetOwnerScene(this);

					m_actorList.push_back(std::unique_ptr<Actor>(TargetActor));

					return true;
				}
			}
		}
	}

	void Scene::MarkedActorDestroy(Actor* TargetActor)
	{
		/* 非立即删除, 而是标记为待删除列表 */
		if (!TargetActor) return;
		TargetActor->SetActorStatus(ActorStatus::PendingDestroy);
	}

	void Scene::LoadModel(const std::string& FilePath)
	{
		std::vector<ModelData> modelData = Engine::GetModelLoaderManager().LoadModel(FilePath);
		ModelResourceManager& resourceManager = Engine::GetModelResourceManager();

		for (const ModelData& model : modelData)
		{
			std::vector<Vertex> vertex{};
			CopyVertexDataStruct(model.vertices, vertex);

			suz::MeshComponent* meshComponent = new suz::MeshComponent();
			/* 加载Mesh */
			meshComponent->CreateMesh(vertex, model.indices);

			std::string texturePath = model.modelFolderPath + "/" + model.material.baseColorTexturePath;
			/* 加载Material */
			meshComponent->CreateMaterial(new Material(texturePath));

			Actor* actor = new Actor();
			RegisterActor(actor);
			actor->RegisterComponent<MeshComponent>(meshComponent);
		}
	}

	void Scene::BeginPlay()
	{
		for (std::unique_ptr<Actor>& actor : m_actorList)
		{
			/* 给Actor注册组件 */
			actor->BatchRegisterAllSubComponents();
		}

		/* 仅对根节点进行Update逻辑 */
		for (Actor* actor : m_rootActorList)
		{
			actor->BeginPlay();
		}
	}

	void Scene::Update(float DeltaTime)
	{
		for (auto it = m_actorList.begin(); it != m_actorList.end(); it++)
		{
			/* 仅对根节点进行Update逻辑 */
			if ((*it)->GetParentActor() == nullptr)
			{
				(*it)->Update(DeltaTime);
			}

			/* 帧结束再执行销毁逻辑 */
			ActorStatus actorStatus = (*it)->GetActorStatus();
			if (actorStatus == ActorStatus::PendingDestroy)
			{
				(*it)->Destroy();
			}

			/* 对已经执行销毁逻辑的Actor进行内存回收 */
			if (actorStatus == ActorStatus::Destroyed)
			{
				m_actorList.erase(it);
			}
		}
	}

	void Scene::Destroy()
	{
		for (Actor* actor : m_rootActorList)
		{
			actor->Destroy();
		}

		m_rootActorList.clear();
		m_actorList.clear();
	}

	bool Scene::CheckWouldCreateCycle(Actor* TargetActor, Actor* ParentActor)
	{
		Actor* parent = ParentActor;
		while (parent)
		{
			if (parent == TargetActor)
			{
				return true;
			}

			parent = parent->GetParentActor();
		}

		return false;
	}

	void CopyVertexDataStruct(std::vector<ModelVertexData> SrcData, std::vector<Vertex>& DstData)
	{
		DstData.resize(SrcData.size());
		memcpy(DstData.data(), SrcData.data(), sizeof(ModelVertexData) * SrcData.size());
	}
}