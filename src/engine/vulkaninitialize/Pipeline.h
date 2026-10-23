#pragma once

#include "BaseObject.h"
#include "VulkanInitialize.h"

#include <vector>
#include <string>

namespace suz
{
	/* 存储所有的管线配置信息 */
	struct PipelineConfigInfo
	{
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		uint32 subpass = 0;
	};

	class Pipeline : public BaseObject
	{
	public:
		Pipeline(
			VulkanInitialize& VulkanInitialize,
			const VkRenderPass& renderPass,
			const std::string& VertexFilePath,
			const std::string& FragmentFilePath,
			const PipelineConfigInfo& ConfigInfo
		);
		~Pipeline();

		virtual void Destroy() override;

		static PipelineConfigInfo DefaultPipelineConfigInfo();

	private:
		static std::vector<char> ReadFile(const std::string& FilePath);

		void CreateDescriptorSetLayout();
		void CreatePushConstantRange();
		void CreatePipelineLayout();

		void CreateGraphicsPipeline(const std::string& VertexFilePath, const std::string& FragmentFilePath, const PipelineConfigInfo& ConfigInfo);

		void CreateShaderMoudle(const std::vector<char>& ShaderCode, VkShaderModule* ShaderMoudle);

		VulkanInitialize& m_vulkanInitialize;
		const VkRenderPass& m_renderPass;

		VkViewport m_viewport = {};
		VkRect2D m_scissor = {};
		VkPipeline m_graphicsPipeline = {};
		VkDescriptorSetLayout m_descriptorSetLayout = {};
		VkPushConstantRange m_pushConstantRange = {};
		VkPipelineLayout m_pipelineLayout = {};
		VkShaderModule m_vertShaderMoudle = {};
		VkShaderModule m_fragShaderMoudle = {};

	public:
		inline const VkPipeline& GetGraphicsPipeline() const { return m_graphicsPipeline; }
		inline const VkViewport& GetViewport() const { return m_viewport; }
		inline const VkRect2D& GetScissor() const { return m_scissor; }
		inline const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
		inline const VkPipelineLayout& GetPipelineLayout() const { return m_pipelineLayout; }
	};
}
