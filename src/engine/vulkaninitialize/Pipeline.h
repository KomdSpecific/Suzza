#pragma once

#include "VulkanResourceBase.h"
#include "VulkanInitialize.h"

namespace suz
{
	/* 存储所有的管线配置信息 */
	struct PipelineConfigInfo
	{
		VkPipelineViewportStateCreateInfo viewportInfo = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
		VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		uint32 subpass = 0;
	};

	struct PipelineLayoutSpec
	{
		PipelineLayoutSpec(const VkDevice& Device)
			: device(Device) {
		}

		const VkDevice& device;

		Vector<VkDescriptorSetLayout> descriptorSetLayout{};
		Vector<VkPushConstantRange> constantRage{};

		static VkPipelineLayoutCreateInfo DefaultPipelineLayoutCreateInfo(const PipelineLayoutSpec& Spec);
	};

	struct PipelineSpec
	{
		PipelineSpec(
			const VkDevice& Device,
			const VkRenderPass& RenderPass,
			const VkPipelineLayout& Layout)
			: device(Device), renderPass(RenderPass), layout(Layout) {
		}

		const VkDevice& device;
		const VkRenderPass& renderPass;
		const VkPipelineLayout& layout;

		/* 管线着色器信息 */
		Vector<VkPipelineShaderStageCreateInfo> shaderStages{};

		VkShaderModule vertShaderMoudle{};
		VkShaderModule fragShaderMoudle{};

		/* 顶点绑定和顶点属性描述信息 */
		Vector<VkVertexInputBindingDescription> bindingDescriptions{};
		Vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		PipelineConfigInfo pipelineConfigInfo{};

		static PipelineConfigInfo DefaultPipelineConfigInfo();
		static VkPipelineVertexInputStateCreateInfo DefaultVertexInputStateCreateInfo(const PipelineSpec& Spec);
		static VkGraphicsPipelineCreateInfo DefaultGraphicsPipelineCreateInfo(const PipelineSpec& Spec);
	};

	class Pipeline : public VulkanResourceBase
	{
	public:
		Pipeline() = default;
		~Pipeline();

		bool Create(PipelineLayoutSpec& Spec);
		bool Create(PipelineSpec& Spec);
		bool CreatePipelineLayout(PipelineLayoutSpec& Spec);
		bool CreateGraphicsPipeline(PipelineSpec& Spec);

		void Destroy();

		static void CreateShaderMoudle(const std::vector<char>& ShaderCode, VkShaderModule* ShaderMoudle);

	private:
		VkDevice m_device = {};

		VkPipeline m_graphicsPipeline = {};
		VkPipelineLayout m_pipelineLayout = {};

	public:
		inline const VkPipeline& GetGraphicsPipeline() const { return m_graphicsPipeline; }
		inline const VkPipelineLayout& GetPipelineLayout() const { return m_pipelineLayout; }
	};
}
