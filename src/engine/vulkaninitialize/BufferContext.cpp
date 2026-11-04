#include "BufferContext.h"
#include "CommandContext.h"
#include "model/RenderTypes.h"
#include "functionLibrary/VulkanFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	BufferContext::BufferContext(const DeviceContext& Context, CommandContext* Command)
		: m_deviceContext(Context), m_commandContext(Command)
	{
	}

	BufferContext::~BufferContext()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class BufferContext Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void BufferContext::CreateBufferContext(usize MaxFlightFrame)
	{
		if (!(*m_isAlive)) return;

		CreateUniformBuffers(MaxFlightFrame);

		*m_isAlive = false;
	}

	void BufferContext::Destroy()
	{
		DestroyUniformBuffers();
	}

	void BufferContext::CreateUniformBuffers(usize MaxFlightFrame)
	{
		VkDeviceSize bufferSize = sizeof(CameraViewportInfo);

		/* 只创建缓冲区并关联内存, 不正式更改顶点属性 */
		m_uniforms.resize(MaxFlightFrame);
		m_uniformsMemory.resize(MaxFlightFrame);
		m_uniformsMapped.resize(MaxFlightFrame);

		for (usize flightIndex = 0; flightIndex < MaxFlightFrame; flightIndex++)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniforms[flightIndex], m_uniformsMemory[flightIndex]);
			/* 只进行映射而不释放, 称为持久映射 */
			/* 缓冲内存作为桥梁链接显存和CPU内存 */
			vkMapMemory(m_deviceContext.device, m_uniformsMemory[flightIndex], 0, bufferSize, 0, &m_uniformsMapped[flightIndex]);
		}
	}

	void BufferContext::DestroyUniformBuffers()
	{
		for (usize uniformIndex = 0; uniformIndex < m_uniforms.size(); uniformIndex++)
		{
			vkDestroyBuffer(m_deviceContext.device, m_uniforms[uniformIndex], nullptr);
			vkFreeMemory(m_deviceContext.device, m_uniformsMemory[uniformIndex], nullptr);
		}
	}

	bool BufferContext::CreateBuffer(const VkDeviceSize& DeviceSize, const VkBufferUsageFlags& Usage, const VkMemoryPropertyFlags& Property, VkBuffer& OutBuffer, VkDeviceMemory& OutBufferMemory)
	{
		/* 1. 定义顶点缓冲区信息 */
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = DeviceSize;
		createInfo.usage = Usage;										// 将要定义缓冲的数据(顶点缓冲或片段缓冲等)
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// 只有特定队列族独占

		if (vkCreateBuffer(m_deviceContext.device, &createInfo, nullptr, &OutBuffer) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建顶点缓冲区失败!");
			return false;
		}

		/* 查询缓冲区内存的要求 */
		VkMemoryRequirements memoryRequired{};
		vkGetBufferMemoryRequirements(m_deviceContext.device, OutBuffer, &memoryRequired);

		/* 2. 定义内存分配信息 */
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequired.size;
		allocateInfo.memoryTypeIndex = VulkanFunctionLibrary::FindMemoryType(m_deviceContext.physicalDevice, memoryRequired.memoryTypeBits, Property);

		if (vkAllocateMemory(m_deviceContext.device, &allocateInfo, nullptr, &OutBufferMemory) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("分配顶点缓冲区内存失败!");
			return false;
		}

		/* 3. 将分配的内存与顶点缓冲关联起来 */
		vkBindBufferMemory(m_deviceContext.device, OutBuffer, OutBufferMemory, 0);
	}

	void BufferContext::DestroyBuffer(VkBuffer& Buffer, VkDeviceMemory& DeviceMemory)
	{
		if (Buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_deviceContext.device, Buffer, nullptr);
			Buffer = VK_NULL_HANDLE;
		}

		if (DeviceMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_deviceContext.device, DeviceMemory, nullptr);
			DeviceMemory = VK_NULL_HANDLE;
		}
	}

	void BufferContext::CreateVertexBuffer(const void* Vertices, VkDeviceSize DataSize, VkBuffer& VertexBuffer, VkDeviceMemory& VertexMemory)
	{
		if (!m_commandContext)
		{
			SUZZA_LOG_ERROR("未创建命令上下文, 创建顶点缓冲区失败!");
			return;
		}

		/* 1. 创建主机可见的缓冲作为临时缓冲 */
		VkBuffer stageBuffer = {};
		VkDeviceMemory stageMemory = {};
		CreateBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stageBuffer, stageMemory);

		/* 将缓冲内存映射到CPU可访问的内存 */
		void* data;
		vkMapMemory(m_deviceContext.device, stageMemory, 0, DataSize, 0, &data);
		/* 拷贝数据到映射的内存 */
		memcpy(data, Vertices, static_cast<usize>(DataSize));
		vkUnmapMemory(m_deviceContext.device, stageMemory);

		/* 2. 创建设备本地内存的缓冲区(内存类型最佳CPU无法直接访问) */
		CreateBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexMemory);

		/* 3. 复制临时缓冲区的数据到顶点缓冲区 */
		m_commandContext->CopyBuffer(stageBuffer, VertexBuffer, DataSize);

		/* 4. 销毁旧的临时缓冲区 */
		vkDestroyBuffer(m_deviceContext.device, stageBuffer, nullptr);
		vkFreeMemory(m_deviceContext.device, stageMemory, nullptr);
	}

	void BufferContext::CreateIndexBuffer(const uint32* Indices, usize IndexNum, VkBuffer& IndexBuffer, VkDeviceMemory& IndexMemory)
	{
		if (!m_commandContext)
		{
			SUZZA_LOG_ERROR("未创建命令上下文, 创建顶点缓冲区失败!");
			return;
		}

		VkDeviceSize dataSize = sizeof(uint32) * IndexNum;

		/* 1. 创建临时缓冲区 */
		VkBuffer stageBuffer = {};
		VkDeviceMemory stageMemory = {};

		CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stageBuffer, stageMemory);

		/* 将缓冲内存映射到CPU可访问的内存 */
		void* data;
		vkMapMemory(m_deviceContext.device, stageMemory, 0, dataSize, 0, &data);
		memcpy(data, Indices, static_cast<usize>(dataSize));
		vkUnmapMemory(m_deviceContext.device, stageMemory);

		/* 2. 创建索引缓冲区 */
		CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexMemory);

		/* 3. 复制临时缓冲区到索引缓冲区 */
		m_commandContext->CopyBuffer(stageBuffer, IndexBuffer, dataSize);

		/* 4. 销毁旧的临时缓冲区 */
		vkDestroyBuffer(m_deviceContext.device, stageBuffer, nullptr);
		vkFreeMemory(m_deviceContext.device, stageMemory, nullptr);
	}
}