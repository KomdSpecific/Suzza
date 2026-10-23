#include "Mesh.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/BufferArea.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkVertexInputBindingDescription Vertex::GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;									// 顶点数组中开始绑定的位置(索引)
		bindingDescription.stride = sizeof(Vertex);						// 每个顶点所有属性的总字节
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;		// 每个顶点之后移动到下一个实例条目

		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;							// 对应着色器的location指令
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// 描述属性的所有分量, 此处3个32位浮点分量
		attributeDescriptions[0].offset = offsetof(Vertex, location);	// 从顶点内开始读取的偏移量

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	Mesh::Mesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices)
	{
		Engine& engInstance = Engine::GetEngineInstance();
		BufferArea* bufferarea = engInstance.GetVulkanResourceManager().GetBufferArea();
		bufferarea->CreateVertexBuffer(Vertices, m_vertexBuffer, m_vertexBufferMemory);
		m_indexBufferSize = bufferarea->CreateIndexBuffer(Indices, m_indexBuffer, m_indexBufferMemory);
	}

	Mesh::~Mesh()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class Mesh Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void Mesh::Destroy()
	{
		if (!(*m_isAlive)) return;

		VkDevice device = Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetDevice();
		vkDestroyBuffer(device, m_vertexBuffer, nullptr);
		vkFreeMemory(device, m_vertexBufferMemory, nullptr);

		vkDestroyBuffer(device, m_indexBuffer, nullptr);
		vkFreeMemory(device, m_indexBufferMemory, nullptr);

		*m_isAlive = false;
	}
}