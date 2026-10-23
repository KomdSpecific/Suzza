#pragma once

#include "BaseObject.h"
#include "model/Render.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace suz
{
	class Pipeline;

	struct ViewportConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
	};

	class BufferArea : public BaseObject
	{
	public:
		BufferArea(
			Pipeline& Pipeline,
			const VkPhysicalDevice& PhysicalDevice,
			const VkDevice& Device,
			const VkSurfaceKHR& Surface,
			const VkExtent2D& SwapChainExtent,
			const VkRenderPass& RenderPass
		);
		~BufferArea();

		void InitBufferArea();

		virtual void Destroy() override;

		void RegisterBindlessDescriptorSet(const VkDescriptorSet& TargetDescriptorSet);

#pragma region BUFFER_HELPER_FUNCTION
		static void CreateBuffer(const VkDeviceSize& Buffersize, const VkBufferUsageFlags& Usage, const VkMemoryPropertyFlags& Properties, VkBuffer& BufferToCreate, VkDeviceMemory& AllocatedMemory);
		void CreateVertexBuffer(const std::vector<Vertex>& Vertices, VkBuffer& VertexBuffer, VkDeviceMemory& VertexBufferMemory);
		uint32_t CreateIndexBuffer(const std::vector<uint16_t>& Indices, VkBuffer& IndexBuffer, VkDeviceMemory& IndexBufferMemory);
		/* 复制缓冲区到图像 */
		void CopyBufferToImage(const VkBuffer& SrcBuffer, const VkImage& DstImage, uint32_t Width, uint32_t Height);
		/* 只创建缓冲区并关联内存, 不正式更改顶点属性 */
		void CreateUniformBuffers();

#pragma endregion

		void RecordCommandBuffer(const VkCommandBuffer& CommandBuffer, const VkDescriptorSet& CameraDescriptor, const std::unordered_map<Mesh*, std::vector<ModelConstant>>& MeshToDraw, uint32_t ImageIndex);

		void ReCreateBufferArea(const VkExtent2D& NewSwapChainExtent);
		void CleanupBufferArea();

#pragma region STATIC_HELPER_FUNCTION
		static VkFormat FindDepthFormat();
		/* 找到正确的内存类型 */
		static uint32_t FindMemoryType(uint32_t TypeFilter, const VkMemoryPropertyFlags& Properties);
		/* 找到支持的格式 */
		static VkFormat FindSupportFormat(const std::vector<VkFormat>& Candidates, const VkImageTiling& Tiling, const VkFormatFeatureFlags& Features);

#pragma endregion

		/* 图像布局变换 */
		void TransitionImageLayout(const VkImage& Image, const VkFormat& Format, const VkImageLayout& OldLayout, const VkImageLayout& NewLayout);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(const VkCommandBuffer& CommandBuffer);

	private:
		void CreateFrameBuffers();

		void CreateCommandPool();
		void CreateDepthResources();
		void CreateCommandBuffer();

		void InitViewportConfig();
		/* 复制缓冲区 */
		void CopyBuffer(const VkBuffer& SrcBuffer, const VkBuffer& DstBuffer, const VkDeviceSize& BufferSize);
		/* 判断选择的深度格式是否包含模板分量 */
		bool HasStencilComponent(const VkFormat& DepthFormat);

		Pipeline& m_pipeline;
		const VkPhysicalDevice& m_physicalDevice;
		const VkDevice& m_device;
		const VkSurfaceKHR& m_surface;
		const VkRenderPass& m_renderPass;
		/* 缓存描述符引用集合 */
		VkDescriptorSet m_bindlessDescriptorSet = {};

		VkExtent2D m_swapChainExtent = {};

		ViewportConfigInfo m_viewConfigInfo = {};

		/* uniform缓冲变量 */
		std::vector<VkBuffer> m_uniformBuffers = {};
		std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
		std::vector<void*> m_uniformBuffersMapped = {};

		/* 帧缓冲变量 */
		std::vector<VkFramebuffer> m_frameBuffers = {};
		std::vector<VkCommandBuffer> m_commandBuffer = {};
		VkCommandPool m_commandPool = {};

		/* 深度图像变量 */
		VkImage m_depthImage = {};
		VkDeviceMemory m_depthImageMemory = {};
		VkImageView m_depthImageView = {};
		
		VkDescriptorPool m_descriptorPool = {};
		std::vector<VkDescriptorSet> m_descriptorSet = {};

		bool m_frameBufferResized = false;

	public:
		inline const std::vector<VkFramebuffer>& GetFrameBuffers() const { return m_frameBuffers; }
		inline const std::vector<VkCommandBuffer>& GetCommandBuffer() const { return m_commandBuffer; }
		inline const VkCommandPool& GetCommandPool() const { return m_commandPool; }

		inline const std::vector<VkBuffer>& GetUniformBuffers() const { return m_uniformBuffers; }
		inline std::vector<VkDeviceMemory>& GetUniformBuffersMemory() { return m_uniformBuffersMemory; }
		inline std::vector<void*>& GetUniformBuffersMapped() { return m_uniformBuffersMapped; }

		inline const bool& IsFrameBufferResized() const { return m_frameBufferResized; }
		inline void EnableBufferResized() { m_frameBufferResized = true; }
		inline void DisableBufferResized() { m_frameBufferResized = false; }
	};
}