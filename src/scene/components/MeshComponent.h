#pragma once

#include "Component.h"
#include "model/Mesh.h"
#include "commonmacro/GameSceneMacro.h"

#include <memory>
#include <vector>
#include <string>
#include <utility>

namespace suz
{
	class MeshComponent : public Component
	{
		DECLARE_CLASS(MeshComponent, Component)
		COMPONENT(MeshComponent)
	public:
		MeshComponent();
		MeshComponent(Mesh* mesh);
		~MeshComponent();

		void CreateMesh(const std::string& MeshId);
		void CreateMesh(const std::vector<Vertex>& Vertices, const std::vector<uint16_t>& Indices);
		void CreateMesh(Mesh* mesh);

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:
		std::pair<std::string, std::shared_ptr<Mesh>> m_meshPair = {};
	};
}