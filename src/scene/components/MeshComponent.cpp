#include "MeshComponent.h"
#include "Engine.h"
#include "Actor.h"
#include "model/ModelResourceManager.h"

namespace suz
{
	MeshComponent::MeshComponent()
	{
	}

	MeshComponent::MeshComponent(Mesh* mesh)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& meshId = modelManager.LoadMesh(mesh);

		m_meshPair.first = meshId;
		m_meshPair.second = modelManager.GetLoadedMeshPtr(meshId);
	}

	MeshComponent::~MeshComponent()
	{
	}

	void MeshComponent::CreateMesh(const std::string& MeshId)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		std::shared_ptr<Mesh> meshPtr = modelManager.GetLoadedMeshPtr(MeshId);

		if(meshPtr)
		{
			m_meshPair.first = MeshId;
			m_meshPair.second = meshPtr;
			m_meshPair.second->RegisterOwnerComponent<ThisClass>(this);
		}
	}

	void MeshComponent::CreateMesh(const std::vector<Vertex>& Vertices, const std::vector<uint16_t>& Indices)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& meshId = modelManager.LoadMesh(Vertices, Indices);

		m_meshPair.first = meshId;
		m_meshPair.second = modelManager.GetLoadedMeshPtr(meshId);
		m_meshPair.second->RegisterOwnerComponent<ThisClass>(this);
	}

	void MeshComponent::CreateMesh(Mesh* mesh)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& meshId = modelManager.LoadMesh(mesh);

		m_meshPair.first = meshId;
		m_meshPair.second = modelManager.GetLoadedMeshPtr(meshId);
		m_meshPair.second->RegisterOwnerComponent<ThisClass>(this);
	}

	void MeshComponent::BeginPlay()
	{
		if (!m_meshPair.second)
		{
			m_meshPair.second = Engine::GetModelResourceManager().GetLoadedMeshPtr(m_meshPair.first);
			m_meshPair.second->RegisterOwnerComponent<ThisClass>(this);
		}

		/* 提交Mesh到渲染队列, 能够获取顶点、索引缓冲区 */
		Engine::GetEngineInstance().GetRender()->SubmitMesh(m_meshPair.second.get(), m_actorOwner->GetActorModelMatrix());
	}

	void MeshComponent::Update(float DeltaTime)
	{
	}

	void MeshComponent::Destroy()
	{
	}
}