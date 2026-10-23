#include "MeshComponent.h"
#include "Engine.h"
#include "Actor.h"
#include "model/ModelResourceManager.h"

namespace suz
{
	MeshComponent::MeshComponent()
	{
	}

	MeshComponent::MeshComponent(Mesh* mesh, Material* material)
	{
		if (!mesh || !material) return;

		/* 注册网格 */
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& meshId = modelManager.LoadMesh(mesh);

		m_meshPair.first = meshId;
		m_meshPair.second = modelManager.GetLoadedMesh(meshId);

		/* 注册材质 */
		const std::string& materialId = modelManager.LoadMaterial(material);

		m_materialPair.first = materialId;
		m_materialPair.second = modelManager.GetLoadedMaterial(materialId);
	}

	MeshComponent::~MeshComponent()
	{
	}

	void MeshComponent::CreateMesh(const std::string& MeshId)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		std::shared_ptr<Mesh> meshPtr = modelManager.GetLoadedMesh(MeshId);

		if(meshPtr)
		{
			m_meshPair.first = MeshId;
			m_meshPair.second = meshPtr;
		}
	}

	void MeshComponent::CreateMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& meshId = modelManager.LoadMesh(Vertices, Indices);

		m_meshPair.first = meshId;
		m_meshPair.second = modelManager.GetLoadedMesh(meshId);
	}

	void MeshComponent::CreateMesh(Mesh* mesh)
	{
		if (!mesh) return;

		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& meshId = modelManager.LoadMesh(mesh);

		m_meshPair.first = meshId;
		m_meshPair.second = modelManager.GetLoadedMesh(meshId);
	}

	void MeshComponent::CreateMaterial(const std::string& MaterialId)
	{
		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		std::shared_ptr<Material> materialPtr = modelManager.GetLoadedMaterial(MaterialId);

		if (materialPtr)
		{
			m_materialPair.first = MaterialId;
			m_materialPair.second = materialPtr;
		}
	}

	void MeshComponent::CreateMaterial(Material* material)
	{
		if (!material) return;

		ModelResourceManager& modelManager = Engine::GetModelResourceManager();
		const std::string& materialId = modelManager.LoadMaterial(material);

		m_materialPair.first = materialId;
		m_materialPair.second = modelManager.GetLoadedMaterial(materialId);
	}

	void MeshComponent::BeginPlay()
	{
		if (!m_meshPair.second)
		{
			m_meshPair.second = Engine::GetModelResourceManager().GetLoadedMesh(m_meshPair.first);
		}

		if (!m_materialPair.second)
		{
			m_materialPair.second = Engine::GetModelResourceManager().GetLoadedMaterial(m_materialPair.first);
		}

		ModelConstant modelConstant{};
		modelConstant.modelMatrix = m_actorOwner->GetActorModelMatrix();
		modelConstant.slotIndex = m_materialPair.second->GetMaterialSlot();

		/* 提交Mesh到渲染队列, 能够获取顶点、索引缓冲区 */
		Engine::GetEngineInstance().GetRender()->SubmitMesh(m_meshPair.second.get(), modelConstant);
	}

	void MeshComponent::Update(float DeltaTime)
	{
	}

	void MeshComponent::Destroy()
	{
	}
}