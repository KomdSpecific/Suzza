#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"
#include "structtype/ModelStructor.h"

#include <vulkan/vulkan.h>

namespace suz
{
	struct Vertex : public ModelVertexData
	{
		static VkVertexInputBindingDescription GetBindingDescription();

		static Vector<VkVertexInputAttributeDescription> GetAttributeDescription();
	};

	class Mesh : public BaseObject
	{
	public:
		Mesh(const Vector<Vertex>& Vertices, const Vector<uint32>& Indices);
		Mesh(const Vector<ModelVertexData>& Vertices, const Vector<uint32>& Indices);
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