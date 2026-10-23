#pragma once

#include "BaseObject.h"
#include "Component.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <unordered_set>

namespace suz
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription GetBindingDescription();

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription();
	};

	class Mesh : public BaseObject
	{
	public:
		Mesh(const std::vector<Vertex>& Vertices, const std::vector<uint16_t>& Indices);
		~Mesh();

		virtual void Destroy() override;

	private:
		VkBuffer m_vertexBuffer = {};
		VkDeviceMemory m_vertexBufferMemory = {};

		VkBuffer m_indexBuffer = {};
		VkDeviceMemory m_indexBufferMemory = {};
		uint32_t m_indexBufferSize = 0;

	public:
		inline VkBuffer& GetMeshVertexBuffer() { return m_vertexBuffer; }
		inline VkBuffer& GetMeshIndexBuffer() { return m_indexBuffer; }
		inline const uint32_t& GetIndexBufferSize() const { return m_indexBufferSize; }
	};
}