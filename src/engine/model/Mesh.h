#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"
#include "Component.h"
#include "structtype/ModelStructor.h"

#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include <unordered_set>

namespace suz
{
	struct Vertex : public ModelVertexData
	{
		static VkVertexInputBindingDescription GetBindingDescription();

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription();
	};

	class Mesh : public BaseObject
	{
	public:
		Mesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		Mesh(const std::vector<ModelVertexData>& Vertices, const std::vector<uint32>& Indices);
		~Mesh();

		virtual void Destroy() override;

	private:
		VkBuffer m_vertexBuffer = {};
		VkDeviceMemory m_vertexBufferMemory = {};

		VkBuffer m_indexBuffer = {};
		VkDeviceMemory m_indexBufferMemory = {};
		uint32 m_indexBufferSize = 0;

	public:
		inline VkBuffer& GetMeshVertexBuffer() { return m_vertexBuffer; }
		inline VkBuffer& GetMeshIndexBuffer() { return m_indexBuffer; }
		inline const uint32& GetIndexBufferSize() const { return m_indexBufferSize; }
	};
}