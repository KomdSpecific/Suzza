#include "ModelResourceManager.h"
#include "Engine.h"

#include "debug/SuzzaLog.h"

#include <random>
#include <sstream>
#include <iomanip>

namespace suz
{
	void ModelResourceManager::Destroy()
	{
		if (!(*m_isAlive)) return;

		for (auto& pair : m_cacheMesh)
		{
			pair.second->Destroy();
		}

		for (auto& pair : m_cacheMaterial)
		{
			pair.second->Destroy();
		}

		m_meshPtrToId.clear();
		m_cacheMesh.clear();

		m_materialPtrToId.clear();
		m_cacheMaterial.clear();

		*m_isAlive = false;
	}

	const std::string ModelResourceManager::LoadMesh(const std::vector<Vertex>& Vertices, const std::vector<uint16>& Indices)
	{
		std::shared_ptr<Mesh> loadedMesh = std::make_shared<Mesh>(Vertices, Indices);

		const std::string& meshId = GenerateRandomMeshId();

		if (loadedMesh && !meshId.empty())
		{
			/* 加入id映射表和裸指针映射表 */
			m_cacheMesh[meshId] = loadedMesh;
			m_meshPtrToId[loadedMesh.get()] = meshId;

			return meshId;
		}

		SUZZA_LOG_ERROR("分配网格失败!");

		return {};
	}

	const std::string ModelResourceManager::LoadMesh(Mesh* MeshToLoad)
	{
		if (!MeshToLoad) return {};

		/* 如果映射表中查找到相同Mesh即返回对应的Id */
		auto it = m_meshPtrToId.find(MeshToLoad);
		if (it != m_meshPtrToId.end())
		{
			SUZZA_LOG_WARN("该网格已存在!");
			return it->second;
		}

		std::shared_ptr<Mesh> loadedMesh = std::shared_ptr<Mesh>(MeshToLoad);

		const std::string& meshId = GenerateRandomMeshId();

		if (!meshId.empty())
		{
			m_cacheMesh[meshId] = loadedMesh;
			m_meshPtrToId[loadedMesh.get()] = meshId;				// 记录对象指针id映射

			return meshId;
		}

		SUZZA_LOG_ERROR("分配网格失败!");

		return {};
	}

	void ModelResourceManager::UnLoadMesh(const std::string& MeshId)
	{
		auto it = m_cacheMesh.find(MeshId);
		if (it != m_cacheMesh.end())
		{
			m_cacheMesh[MeshId]->Destroy();

			/* 销毁映射表和管理的Mesh实例 */
			m_meshPtrToId.erase(it->second.get());
			m_cacheMesh.erase(it);
		}

		SUZZA_LOG_WARN("不存在需要销毁的网格!");
	}

	std::shared_ptr<Mesh> ModelResourceManager::GetLoadedMesh(const std::string& MeshId)
	{
		auto it = m_cacheMesh.find(MeshId);
		if (it != m_cacheMesh.end())
		{
			return m_cacheMesh[MeshId];
		}
		else
		{
			SUZZA_LOG_WARN("不存在目标网格!");
			return nullptr;
		}
	}

	const std::string ModelResourceManager::LoadMaterial(Material* MaterialToLoad)
	{
		if (!MaterialToLoad) return {};

		auto it = m_materialPtrToId.find(MaterialToLoad);
		if (it != m_materialPtrToId.end())
		{
			SUZZA_LOG_WARN("该材质已存在!");
			return it->second;
		}

		std::shared_ptr<Material> material = std::shared_ptr<Material>(MaterialToLoad);

		std::string materialId = GemerateRandomMaterialId();

		if (!materialId.empty())
		{
			m_cacheMaterial[materialId] = material;
			m_materialPtrToId[material.get()] = materialId;

			/* 给材质分配纹理插槽 */
			uint16 slotIndex = Engine::GetBindlessSlotsAllocator().AllocateBindlessSlots(materialId);
			material->SetMaterialSlot(slotIndex);

			return materialId;
		}

		SUZZA_LOG_ERROR("分配材质失败!");

		return {};
	}

	void ModelResourceManager::UnLoadMaterial(const std::string& MaterialId)
	{
		auto it = m_cacheMaterial.find(MaterialId);
		if (it != m_cacheMaterial.end())
		{
			it->second->Destroy();

			/* 标记纹理插槽待释放 */
			Engine::GetBindlessSlotsAllocator().MarkedTargetSlotsRelease(MaterialId);
			/* 重置材质纹理插槽为默认值 */
			it->second->SetMaterialSlot(MAX_SAMPLED_TEXTURE_IMAGES_SLOTS);

			m_materialPtrToId.erase(it->second.get());
			m_cacheMaterial.erase(it);
		}

		SUZZA_LOG_WARN("不存在需要销毁的材质!");
	}

	std::shared_ptr<Material> ModelResourceManager::GetLoadedMaterial(const std::string& MaterialId)
	{
		auto it = m_cacheMaterial.find(MaterialId);
		if (it != m_cacheMaterial.end())
		{
			return it->second;
		}
		else
		{
			SUZZA_LOG_WARN("不存在目标材质!");
			return nullptr;
		}
	}

	const std::string ModelResourceManager::GenerateRandomMeshId() const
	{
		static std::random_device randomDevice;
		static std::mt19937 gen(randomDevice());
		static std::uniform_int_distribution<uint32> dist;

		/* 生成不同片段的32位字符串 */
		uint32 data[4];
		data[0] = dist(gen);
		data[1] = dist(gen);
		data[2] = dist(gen);
		data[3] = dist(gen);

		/* 设置版本4: 第三组的第一个4bit = 0x4 */
		data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
		/* 设置variant: 第四组第一个2bit = 0b10 */
		data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;

		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		oss << std::setw(8) << data[0] << "-";

		oss << std::setw(4) << ((data[1] >> 16) & 0xFFFF) << "-";
		oss << std::setw(4) << (data[1] & 0xFFFF) << "-";

		oss << std::setw(4) << ((data[2] >> 16) & 0xFFFF) << "-";
		oss << std::setw(4) << (data[2] & 0xFFFF);

		oss << std::setw(8) << data[3];

		return oss.str();
	}

	const std::string ModelResourceManager::GemerateRandomMaterialId() const
	{
		static std::random_device randomDevice;
		static std::mt19937 gen(randomDevice());
		static std::uniform_int_distribution<uint32> dist;

		/* 生成不同片段的32位字符串 */
		uint32 data[4];
		data[0] = dist(gen);
		data[1] = dist(gen);
		data[2] = dist(gen);
		data[3] = dist(gen);

		/* 设置版本4: 第三组的第一个4bit = 0x4 */
		data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
		/* 设置variant: 第四组第一个2bit = 0b10 */
		data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;

		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		oss << std::setw(8) << data[0] << "-";

		oss << std::setw(4) << ((data[1] >> 16) & 0xFFFF) << "-";
		oss << std::setw(4) << (data[1] & 0xFFFF) << "-";

		oss << std::setw(4) << ((data[2] >> 16) & 0xFFFF) << "-";
		oss << std::setw(4) << (data[2] & 0xFFFF);

		oss << std::setw(8) << data[3];

		return oss.str();
	}



	void BindlessSlotsAllocator::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_textureSlots.clear();
		m_slotsStatus = { false };

		m_waitReleaseSlots.clear();

		*m_isAlive = false;
	}

	uint16 BindlessSlotsAllocator::AllocateBindlessSlots(const std::string& MaterialId)
	{
		auto it = m_textureSlots.find(MaterialId);
		if (it != m_textureSlots.end())
		{
			return it->second;
		}

		/* 循环遍历插槽索引 */
		while (m_slotsIndex < MAX_SAMPLED_TEXTURE_IMAGES_SLOTS)
		{
			/* 找到所需的插槽索引 */
			if (m_slotsStatus[m_slotsIndex] == false)
			{
				m_textureSlots[MaterialId] = m_slotsIndex;
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

	uint16 BindlessSlotsAllocator::GetMaterialBindlessSlots(const std::string& MaterialId)
	{
		auto it = m_textureSlots.find(MaterialId);
		if (it != m_textureSlots.end())
		{
			return it->second;
		}
		else
		{
			/* 处理纹理Id不存在的情况 */
			return MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;
		}
	}

	void BindlessSlotsAllocator::MarkedTargetSlotsRelease(const std::string& MaterialId)
	{
		auto it = m_textureSlots.find(MaterialId);
		if (it != m_textureSlots.end())
		{
			/* 纹理Id存在 */
			m_waitReleaseSlots[it->first] = MAX_FRAMES_IN_FLIGHT;
		}

		SUZZA_LOG_INFO("纹理Id不存在! MaterialId: ", MaterialId);
	}

	void BindlessSlotsAllocator::MarkedTargetSlotsRelease(const uint16& SlotIndex)
	{
		if (SlotIndex >= MAX_SAMPLED_TEXTURE_IMAGES_SLOTS)
		{
			SUZZA_LOG_WARN("纹理槽索引限制在0 ~ %d之间 索引已超出范围! SlotIndex: %d", MAX_SAMPLED_TEXTURE_IMAGES_SLOTS, SlotIndex);
		}

		for (auto& pair : m_textureSlots)
		{
			if (pair.second == SlotIndex)
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
				uint16 slotIndex = m_textureSlots[pair.first];

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