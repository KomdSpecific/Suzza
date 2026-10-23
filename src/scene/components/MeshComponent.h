#pragma once

#include "Component.h"
#include "model/Mesh.h"
#include "model/Material.h"
#include "macros/ReflectMacros.h"
#include "MeshComponent.generated.h"

#include <memory>
#include <vector>
#include <string>
#include <utility>

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

		void CreateMesh(const std::string& MeshId);
		void CreateMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		void CreateMesh(Mesh* mesh);

		void CreateMaterial(const std::string& MaterialId);
		void CreateMaterial(Material* material);

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:
		std::pair<std::string, std::shared_ptr<Mesh>> m_meshPair = {};
		std::pair<std::string, std::shared_ptr<Material>> m_materialPair = {};
	};
}