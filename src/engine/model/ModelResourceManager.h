#pragma once

#include "BaseObject.h"
#include "Mesh.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace suz
{
	class ModelResourceManager : public BaseObject
	{
	public:
		ModelResourceManager() = default;

		virtual void Destroy() override;

	private:
		ModelResourceManager(const ModelResourceManager&) = delete;
		ModelResourceManager& operator=(const ModelResourceManager&) = delete;

	public:
		const std::string LoadMesh(const std::vector<Vertex>& Vertices, const std::vector<uint16_t>& Indices);
		const std::string LoadMesh(Mesh* MeshToLoad);

		void UnLoadMesh(const std::string& MeshId);

		std::shared_ptr<Mesh> GetLoadedMeshPtr(const std::string& MeshId);

	private:
		const std::string GenerateRandomMeshId() const;

		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_cacheMesh = {};
		std::unordered_map<Mesh*, std::string> m_meshPtrToId = {};
	};
}