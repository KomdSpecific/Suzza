#include "ModelResourceManager.h"

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

		m_meshPtrToId.clear();
		m_cacheMesh.clear();

		*m_isAlive = false;
	}

	const std::string ModelResourceManager::LoadMesh(const std::vector<Vertex>& Vertices, const std::vector<uint16_t>& Indices)
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

		return {};
	}

	const std::string ModelResourceManager::LoadMesh(Mesh* MeshToLoad)
	{
		if (!MeshToLoad) return {};

		/* 如果映射表中查找到相同Mesh即返回对应的Id */
		auto it = m_meshPtrToId.find(MeshToLoad);
		if (it != m_meshPtrToId.end())
		{
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
	}

	std::shared_ptr<Mesh> ModelResourceManager::GetLoadedMeshPtr(const std::string& MeshId)
	{
		auto it = m_cacheMesh.find(MeshId);
		if (it != m_cacheMesh.end())
		{
			return m_cacheMesh[MeshId];
		}
		else
		{
			return nullptr;
		}
	}

	const std::string ModelResourceManager::GenerateRandomMeshId() const
	{
		static std::random_device randomDevice;
		static std::mt19937 gen(randomDevice());
		static std::uniform_int_distribution<uint32_t> dist;

		/* 生成不同片段的32位字符串 */
		uint32_t data[4];
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
}