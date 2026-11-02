#pragma once

#include "BaseObject.h"
#include "Mesh.h"
#include "Material.h"
#include "macros/VkMacro.h"
#include "SuzzaTypes.h"
#include "ObjectManagerBase.h"

namespace suz
{
	struct MeshHandle : public HandleBase
	{
		using HandleBase::HandleBase;
	};

	/* 负责全局管理Mesh */
	class MeshResourceManager : public PointerPoolBase<Mesh>
	{
	public:
		MeshResourceManager() = default;

		void Destroy();

	private:
		MeshResourceManager(const MeshResourceManager&) = delete;
		MeshResourceManager& operator=(const MeshResourceManager&) = delete;

	public:
		MeshHandle LoadMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		MeshHandle LoadMesh(Mesh* MeshToLoad);

		void UnLoadMesh(MeshHandle Handle);

		Mesh* GetLoadedMesh(MeshHandle Handle);
	};


	/* 负责全局管理Material */
	struct MaterialHandle : public HandleBase
	{
		using HandleBase::HandleBase;
	};

	class MaterialResourceManager : public PointerPoolBase<Material>
	{
	public:
		MaterialResourceManager() = default;

		void Destroy();

	private:
		MaterialResourceManager(const MaterialResourceManager&) = delete;
		MaterialResourceManager& operator=(const MaterialResourceManager&) = delete;

	public:
		MaterialHandle LoadMaterial(Material* MaterialToLoad);

		void UnLoadMaterial(const MaterialHandle& MaterialId);

		Material* GetLoadedMaterial(const MaterialHandle& MaterialId);
	};



	class BindlessSlotsAllocator : public BaseObject
	{
		struct SlotInfo
		{
			uint16 slotIndex = 0;
			uint32 generation = 0;
		};

	public:
		BindlessSlotsAllocator() = default;

		virtual void Destroy() override;

	private:
		BindlessSlotsAllocator(const BindlessSlotsAllocator&) = delete;
		BindlessSlotsAllocator& operator=(const BindlessSlotsAllocator&) = delete;

	public:
		uint16 AllocateBindlessSlots(const MaterialHandle& Handle);
		uint16 GetMaterialBindlessSlots(const MaterialHandle& Handle);

		void MarkedTargetSlotsRelease(const MaterialHandle& Handle);
		void MarkedTargetSlotsRelease(const uint16& SlotIndex);

		void ReleaseSlots();

	private:
		std::unordered_map<uint32, SlotInfo> m_textureSlots = {};
		std::array<bool, MAX_SAMPLED_TEXTURE_IMAGES_SLOTS> m_slotsStatus = { false };

		/* 等待释放纹理资源的插槽 */
		std::unordered_map<uint32, byte> m_waitReleaseSlots = {};

		uint16 m_slotsIndex = 0;
	};
}