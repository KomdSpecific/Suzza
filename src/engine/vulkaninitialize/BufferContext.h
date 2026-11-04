#pragma once

#include "VulkanResourceBase.h"
#include "DeviceContext.h"

namespace suz
{
	class CommandContext;

	/* 所有缓冲区唯一出入口 如 uniform缓冲 顶点缓冲 索引缓冲 */
	class BufferContext : public VulkanResourceBase
	{
	public:
		BufferContext(const DeviceContext& Context, CommandContext* Command);
		~BufferContext();

		void CreateBufferContext(usize MaxFlightFrame);

		void Destroy();

#pragma region UNIFORM
		void CreateUniformBuffers(usize MaxFlightFrame);
		void DestroyUniformBuffers();

#pragma endregion

		/* 创建特定的缓冲区
		*  1. 缓冲区大小
		*  2. 将要定义缓冲的数据(顶点缓冲或片段缓冲等)
		*  3. 缓冲区内存类型的变量
		*/
		bool CreateBuffer(const VkDeviceSize& DeviceSize, const VkBufferUsageFlags& Usage, const VkMemoryPropertyFlags& Property, VkBuffer& OutBuffer, VkDeviceMemory& OutBufferMemory);
		void DestroyBuffer(VkBuffer& Buffer, VkDeviceMemory& DeviceMemory);

#pragma region GEOMETRY_UPLOAD
		void CreateVertexBuffer(const void* Vertices, VkDeviceSize DataSize, VkBuffer& VertexBuffer, VkDeviceMemory& VertexMemory);
		void CreateIndexBuffer(const uint32* Indices, usize IndexNum, VkBuffer& IndexBuffer, VkDeviceMemory& IndexMemory);

#pragma endregion

	private:
		DeviceContext m_deviceContext = {};

		CommandContext* m_commandContext = nullptr;

		/* uniform缓冲资源 */
		Vector<VkBuffer> m_uniforms = {};
		Vector<VkDeviceMemory> m_uniformsMemory = {};
		Vector<void*> m_uniformsMapped = {};

	public:
		inline const std::vector<VkBuffer>& GetUniformBuffers() const { return m_uniforms; }
		inline std::vector<VkDeviceMemory>& GetUniformBuffersMemory() { return m_uniformsMemory; }
		inline std::vector<void*>& GetUniformBuffersMapped() { return m_uniformsMapped; }
	};
}