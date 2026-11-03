#include "ModelResourceManager.h"
#include "Engine.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	void MeshResourceManager::Destroy()
	{
		for (uint32 i = 0; i < m_ptrSlots.size(); i++)
		{
			ObjectPtrInfo& ptrInfo = m_ptrSlots[i];
			if (!ptrInfo.isAlive)
				continue;

			ptrInfo.isAlive = false;
			ptrInfo.generation += 1;

			ptrInfo.ptr->Destroy();
			delete ptrInfo.ptr;
			ptrInfo.ptr = nullptr;

			m_freeIndices.push_back(i);
		}
	}

	MeshHandle MeshResourceManager::LoadMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices)
	{
		Mesh* mesh = new Mesh(Vertices, Indices);
		HandleBase base = Allocate(mesh);

		return MeshHandle(base.index, base.generation);
	}

	MeshHandle MeshResourceManager::LoadMesh(Mesh* MeshToLoad)
	{
		HandleBase base = Allocate(MeshToLoad);

		return MeshHandle(base.index, base.generation);
	}

	void MeshResourceManager::UnLoadMesh(MeshHandle Handle)
	{
		Free({ Handle.index,Handle.generation });
	}

	Mesh* MeshResourceManager::GetLoadedMesh(MeshHandle Handle)
	{
		return Find({ Handle.index,Handle.generation });
	}



	void MaterialResourceManager::Destroy()
	{
		for (uint32 i = 0; i < m_ptrSlots.size(); i++)
		{
			ObjectPtrInfo& ptrInfo = m_ptrSlots[i];
			if (!ptrInfo.isAlive)
				continue;

			ptrInfo.isAlive = false;
			ptrInfo.generation += 1;

			ptrInfo.ptr->Destroy();
			delete ptrInfo.ptr;
			ptrInfo.ptr = nullptr;

			m_freeIndices.push_back(i);
		}
	}

	MaterialHandle MaterialResourceManager::LoadMaterial(Material* MaterialToLoad)
	{
		if (!MaterialToLoad) return MaterialHandle();

		HandleBase base = Allocate(MaterialToLoad);

		MaterialHandle handle(base.index, base.generation);
		/* 给材质分配纹理插槽 */
		uint16 slotIndex = Engine::GetBindlessSlotsAllocator().AllocateBindlessSlots(handle);
		if (slotIndex == MAX_SAMPLED_TEXTURE_IMAGES_SLOTS)
		{
			SUZZA_LOG_ERROR("分配纹理插槽失败, 插槽正在被旧资源占用!");
		}

		MaterialToLoad->SetMaterialSlot(slotIndex);

		return handle;
	}

	void MaterialResourceManager::UnLoadMaterial(const MaterialHandle& Handle)
	{
		Material* material = Find({ Handle.index,Handle.generation });
		if (!material) return;

		/* 标记纹理插槽待释放 */
		Engine::GetBindlessSlotsAllocator().MarkedTargetSlotsRelease(Handle);
		/* 重置材质纹理插槽为默认值 */
		material->SetMaterialSlot(MAX_SAMPLED_TEXTURE_IMAGES_SLOTS);

		Free({ Handle.index,Handle.generation });
	}

	Material* MaterialResourceManager::GetLoadedMaterial(const MaterialHandle& Handle)
	{
		return Find({ Handle.index,Handle.generation });
	}



	void BindlessSlotsAllocator::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_textureSlots.clear();
		m_slotsStatus = { false };

		m_waitReleaseSlots.clear();

		*m_isAlive = false;
	}

	uint16 BindlessSlotsAllocator::AllocateBindlessSlots(const MaterialHandle& Handle)
	{
		auto it = m_textureSlots.find(Handle.index);
		if (it != m_textureSlots.end())
		{
			/* 假设插槽被旧资源占用返回插槽默认值 */
			if (Handle.generation == it->second.generation)
			{
				return it->second.slotIndex;
			}
			else
			{
				return MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;
			}
		}

		/* 循环遍历插槽索引 */
		while (m_slotsIndex < MAX_SAMPLED_TEXTURE_IMAGES_SLOTS)
		{
			/* 找到所需的插槽索引 */
			if (m_slotsStatus[m_slotsIndex] == false)
			{
				m_textureSlots[Handle.index] = { m_slotsIndex,Handle.generation };
				m_slotsStatus[m_slotsIndex] = true;

				m_slotsIndex++;
				break;
			}

			m_slotsIndex++;
		}

		if (m_slotsIndex == MAX_SAMPLED_TEXTURE_IMAGES_SLOTS)
		{
			/* 代表无可用的索引 */
			return MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;
		}
		else
		{
			return m_slotsIndex;
		}
	}

	uint16 BindlessSlotsAllocator::GetMaterialBindlessSlots(const MaterialHandle& Handle)
	{
		auto it = m_textureSlots.find(Handle.index);
		if (it != m_textureSlots.end() && Handle.generation == it->second.generation)
		{
			return it->second.slotIndex;
		}
		else
		{
			/* 处理纹理Id不存在的情况 */
			return MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;
		}
	}

	void BindlessSlotsAllocator::MarkedTargetSlotsRelease(const MaterialHandle& Handle)
	{
		auto it = m_textureSlots.find(Handle.index);
		/* 不能拿老或新句柄释放插槽 */
		if (it != m_textureSlots.end() && Handle.generation == it->second.generation)
		{
			/* 纹理Handle存在 */
			m_waitReleaseSlots[it->first] = MAX_FRAMES_IN_FLIGHT;
		}

		SUZZA_LOG_INFO("纹理不存在! Index: ", Handle.index, ", Generation: ", Handle.generation);
	}

	void BindlessSlotsAllocator::MarkedTargetSlotsRelease(const uint16& SlotIndex)
	{
		if (SlotIndex >= MAX_SAMPLED_TEXTURE_IMAGES_SLOTS)
		{
			SUZZA_LOG_WARN("纹理槽索引限制在0 ~ %d之间 索引已超出范围! SlotIndex: %d", MAX_SAMPLED_TEXTURE_IMAGES_SLOTS, SlotIndex);
		}

		for (auto& pair : m_textureSlots)
		{
			if (pair.second.slotIndex == SlotIndex)
			{
				m_waitReleaseSlots[pair.first] = MAX_FRAMES_IN_FLIGHT;
			}
		}

		SUZZA_LOG_INFO("纹理插槽 %d 未使用!", SlotIndex);
	}

	void BindlessSlotsAllocator::ReleaseSlots()
	{
		for (auto& pair : m_waitReleaseSlots)
		{
			if (pair.second == 0)
			{
				uint16 slotIndex = m_textureSlots[pair.first].slotIndex;

				/* 正式释放纹理插槽 */
				m_slotsStatus[slotIndex] = false;
				m_textureSlots.erase(pair.first);
				m_waitReleaseSlots.erase(pair.first);
			}
			else
			{
				pair.second--;
			}
		}
	}
}