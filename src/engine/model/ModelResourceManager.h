#pragma once

#include "BaseObject.h"
#include "Mesh.h"
#include "Material.h"
#include "macros/VkMacro.h"
#include "SuzzaTypes.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace suz
{
	/* 负责全局管理Mesh */
	class ModelResourceManager : public BaseObject
	{
	public:
		ModelResourceManager() = default;

		virtual void Destroy() override;

	private:
		ModelResourceManager(const ModelResourceManager&) = delete;
		ModelResourceManager& operator=(const ModelResourceManager&) = delete;

	public:
#pragma region Mesh
		const std::string LoadMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		const std::string LoadMesh(Mesh* MeshToLoad);

		void UnLoadMesh(const std::string& MeshId);

		std::shared_ptr<Mesh> GetLoadedMesh(const std::string& MeshId);

#pragma endregion

#pragma region Material
		const std::string LoadMaterial(Material* MaterialToLoad);

		void UnLoadMaterial(const std::string& MaterialId);

		std::shared_ptr<Material> GetLoadedMaterial(const std::string& MaterialId);

#pragma endregion

	private:
		const std::string GenerateRandomMeshId() const;

		const std::string GemerateRandomMaterialId() const;

		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_cacheMesh = {};
		std::unordered_map<Mesh*, std::string> m_meshPtrToId = {};

		std::unordered_map<std::string, std::shared_ptr<Material>> m_cacheMaterial = {};
		std::unordered_map<Material*, std::string> m_materialPtrToId = {};
	};



	class BindlessSlotsAllocator : public BaseObject
	{
	public:
		BindlessSlotsAllocator() = default;

		virtual void Destroy() override;

	private:
		BindlessSlotsAllocator(const BindlessSlotsAllocator&) = delete;
		BindlessSlotsAllocator& operator=(const BindlessSlotsAllocator&) = delete;

	public:
		uint16 AllocateBindlessSlots(const std::string& MaterialId);
		uint16 GetMaterialBindlessSlots(const std::string& MaterialId);

		void MarkedTargetSlotsRelease(const std::string& MaterialId);
		void MarkedTargetSlotsRelease(const uint16& SlotIndex);

		void ReleaseSlots();

	private:
		std::unordered_map<std::string, uint16> m_textureSlots = {};
		std::array<bool, MAX_SAMPLED_TEXTURE_IMAGES_SLOTS> m_slotsStatus = { false };

		/* 等待释放纹理资源的插槽 */
		std::unordered_map<std::string, byte> m_waitReleaseSlots = {};

		uint16 m_slotsIndex = 0;
	};
}