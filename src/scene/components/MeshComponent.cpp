#include "MeshComponent.h"
#include "Engine.h"
#include "Actor.h"
#include "model/Render.h"

namespace suz
{
	MeshComponent::MeshComponent()
		: Super()
	{
	}

	MeshComponent::MeshComponent(const std::string& ComponentName)
		: Super(ComponentName)
	{
	}

	MeshComponent::MeshComponent(Mesh* mesh, Material* material)
	{
		if (!mesh || !material) return;

		/* 注册网格 */
		MeshResourceManager& meshManager = Engine::GetMeshResourceManager();
		MeshHandle handle = meshManager.LoadMesh(mesh);

		m_meshPair.first = handle;
		m_meshPair.second = meshManager.GetLoadedMesh(handle);

		/* 注册材质 */
		MaterialResourceManager& materialManager = Engine::GetMaterialResourceManager();
		MaterialHandle materialHandle = materialManager.LoadMaterial(material);

		m_materialPair.first = materialHandle;
		m_materialPair.second = materialManager.GetLoadedMaterial(materialHandle);
	}

	MeshComponent::~MeshComponent()
	{
	}

	void MeshComponent::CreateMesh(const MeshHandle& MeshId)
	{
		MeshResourceManager& meshManager = Engine::GetMeshResourceManager();
		Mesh* meshPtr = meshManager.GetLoadedMesh(MeshId);

		if(meshPtr)
		{
			m_meshPair.first = MeshId;
			m_meshPair.second = meshPtr;
		}
	}

	void MeshComponent::CreateMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices)
	{
		MeshResourceManager& meshManager = Engine::GetMeshResourceManager();
		MeshHandle handle = meshManager.LoadMesh(Vertices, Indices);

		m_meshPair.first = handle;
		m_meshPair.second = meshManager.GetLoadedMesh(handle);
	}

	void MeshComponent::CreateMesh(Mesh* mesh)
	{
		if (!mesh) return;

		MeshResourceManager& meshManager = Engine::GetMeshResourceManager();
		MeshHandle handle = meshManager.LoadMesh(mesh);

		m_meshPair.first = handle;
		m_meshPair.second = meshManager.GetLoadedMesh(handle);
	}

	void MeshComponent::CreateMaterial(const MaterialHandle& MaterialHandle)
	{
		MaterialResourceManager& materialManager = Engine::GetMaterialResourceManager();
		Material* materialPtr = materialManager.GetLoadedMaterial(MaterialHandle);

		if (materialPtr)
		{
			m_materialPair.first = MaterialHandle;
			m_materialPair.second = materialPtr;
		}
	}

	void MeshComponent::CreateMaterial(Material* material)
	{
		if (!material) return;

		MaterialResourceManager& materialManager = Engine::GetMaterialResourceManager();
		MaterialHandle materialHandle = materialManager.LoadMaterial(material);

		m_materialPair.first = materialHandle;
		m_materialPair.second = materialManager.GetLoadedMaterial(materialHandle);
	}

	void MeshComponent::BeginPlay()
	{
		if (!m_meshPair.second)
		{
			m_meshPair.second = Engine::GetMeshResourceManager().GetLoadedMesh(m_meshPair.first);
		}

		if (!m_materialPair.second)
		{
			m_materialPair.second = Engine::GetMaterialResourceManager().GetLoadedMaterial(m_materialPair.first);
		}
	}

	void MeshComponent::Update(float DeltaTime)
	{
		ModelConstant modelConstant{};
		modelConstant.modelMatrix = m_actorOwner->GetActorModelMatrix();
		modelConstant.slotIndex = m_materialPair.second->GetMaterialSlot();

		/* 提交Mesh到渲染队列, 能够获取顶点、索引缓冲区 */
		Engine::GetEngineInstance().GetRender()->SubmitMesh(m_meshPair.second, modelConstant);
	}

	void MeshComponent::Destroy()
	{
	}
}