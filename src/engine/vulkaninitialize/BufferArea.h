#pragma once

#include "BaseObject.h"
#include "model/Mesh.h"
#include "model/DrawList.h"
#include "CommandContext.h"
#include "DeviceContext.h"
#include "BufferContext.h"
#include "SwapChain.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace suz
{
	class EngineUIRenderEventDelegate;
	class Pipeline;

	struct ViewportConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
	};


	struct DrawStage
	{
		Pipeline* pipeline = nullptr;
		DrawList* drawList = nullptr;

		/* 网格这种"每条 pipeline 只有一个常量"的场合用它 */
		const void* pipelinePushConstant = nullptr;
		uint32 constantSize = 0;
	};

	class BufferArea : public BaseObject
	{
	public:
		BufferArea(
			Pipeline& Pipeline,
			const VkDevice& Device,
			const VkExtent2D& SwapChainExtent,
			const VkRenderPass& RenderPass
		);
		~BufferArea();

		void InitBufferArea();

		virtual void Destroy() override;

		void RegisterBindlessDescriptorSet(const VkDescriptorSet& TargetDescriptorSet);

		/* 绘制画面的核心! 录制命令缓冲区靠它不断执行 */
		void RecordCommandBuffer(const VkCommandBuffer& CommandBuffer, const VkDescriptorSet& CameraDescriptor, const Vector<DrawStage>& Stages, uint32_t ImageIndex);

		/*void ReCreateBufferArea(const VkExtent2D& NewSwapChainExtent);
		void CleanupBufferArea();*/

	private:
		void CreateFrameBuffers();

		void CreateDepthResources();

		void InitViewportConfig();

		EngineUIRenderEventDelegate& m_engineUIDelegate;

		Pipeline& m_pipeline;
		const VkDevice& m_device;
		const VkRenderPass& m_renderPass;
		CommandContext* m_commandContext;
		BufferContext* m_bufferContext;
		SwapChain* m_swapChain;
		/* 缓存描述符引用集合 */
		VkDescriptorSet m_bindlessDescriptorSet = {};

		ViewportConfigInfo m_viewConfigInfo = {};

		/* uniform缓冲变量 */
		//std::vector<VkBuffer> m_uniformBuffers = {};
		//std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
		//std::vector<void*> m_uniformBuffersMapped = {};

		/* 帧缓冲变量 */
		//std::vector<VkFramebuffer> m_frameBuffers = {};
		//std::vector<VkCommandBuffer> m_commandBuffer = {};
		//VkCommandPool m_commandPool = {};

		/* 深度图像变量 */
		/*VkImage m_depthImage = {};
		VkDeviceMemory m_depthImageMemory = {};
		VkImageView m_depthImageView = {};*/
		
		//VkDescriptorPool m_descriptorPool = {};
		//std::vector<VkDescriptorSet> m_descriptorSet = {};

		bool m_frameBufferResized = false;

	public:
		//inline const std::vector<VkFramebuffer>& GetFrameBuffers() const { return m_frameBuffers; }
		//inline const std::vector<VkCommandBuffer>& GetCommandBuffer() const { return m_commandBuffer; }
		//inline const VkCommandPool& GetCommandPool() const { return m_commandPool; }

		//inline const std::vector<VkBuffer>& GetUniformBuffers() const { return m_uniformBuffers; }
		//inline std::vector<VkDeviceMemory>& GetUniformBuffersMemory() { return m_uniformBuffersMemory; }
		//inline std::vector<void*>& GetUniformBuffersMapped() { return m_uniformBuffersMapped; }

		inline const bool& IsFrameBufferResized() const { return m_frameBufferResized; }
		inline void EnableBufferResized() { m_frameBufferResized = true; }
		inline void DisableBufferResized() { m_frameBufferResized = false; }
	};
}