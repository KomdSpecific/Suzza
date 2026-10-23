#include "Scene.h"
#include "Engine.h"
#include "model/ModelResourceManager.h"
#include "ModelLoaderManager.h"
#include "components/MeshComponent.h"

namespace suz
{
	void CopyVertexDataStruct(std::vector<ModelVertexData> SrcData, std::vector<Vertex>& DstData);

	void Scene::RegisterActor(Actor* TargetActor)
	{
		if (!TargetActor) return;

		/* 防止重复注册 */
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			if (actor.get() == TargetActor) return;
		}

		m_rootActorList.push_back(std::unique_ptr<Actor>(TargetActor));
	}

	void Scene::MarkedActorDestroy(Actor* TargetActor)
	{
		/* 非立即删除, 而是标记为待删除列表 */
		if (!TargetActor) return;
		m_actorForDestroy.push_back(TargetActor);
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
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			actor->BatchRegisterAllSubComponents();
			actor->BeginPlay();
		}
	}

	void Scene::Update(float DeltaTime)
	{
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			actor->Update(DeltaTime);
		}

		for (Actor* actorToDestroy : m_actorForDestroy)
		{
			actorToDestroy->Destroy();

			for (auto it = m_rootActorList.begin(); it != m_rootActorList.end();)
			{
				if (it->get() == actorToDestroy)
				{
					/* unique_ptr自动析构对象 */
					it = m_rootActorList.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
		}

		m_actorForDestroy.clear();
	}

	void Scene::Destroy()
	{
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			actor->Destroy();
		}

		m_actorForDestroy.clear();
		m_rootActorList.clear();
	}

	void CopyVertexDataStruct(std::vector<ModelVertexData> SrcData, std::vector<Vertex>& DstData)
	{
		DstData.resize(SrcData.size());
		memcpy(DstData.data(), SrcData.data(), sizeof(ModelVertexData) * SrcData.size());
	}
}