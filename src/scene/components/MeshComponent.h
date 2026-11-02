#pragma once

#include "Component.h"
#include "model/Mesh.h"
#include "model/Material.h"
#include "macros/ReflectMacros.h"
#include "model/ModelResourceManager.h"
#include "MeshComponent.generated.h"

namespace suz
{
	class MeshComponent : public Component
	{
		SUZZA_REFLECT_CLASS()
		COMPONENT(MeshComponent)
	public:
		MeshComponent();
		MeshComponent(const std::string& ComponentName);
		MeshComponent(Mesh* mesh, Material* material);
		~MeshComponent();

		void CreateMesh(const MeshHandle& MeshId);
		void CreateMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		void CreateMesh(Mesh* mesh);

		void CreateMaterial(const MaterialHandle& MaterialHandle);
		void CreateMaterial(Material* material);

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:
		std::pair<MeshHandle, Mesh*> m_meshPair = {};
		std::pair<MaterialHandle, Material*> m_materialPair = {};
	};
}