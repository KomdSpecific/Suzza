#include "Pipeline.h"
#include "Engine.h"
#include "VulkanResourceManager.h"
#include "macros/VkMacro.h"
#include "file/FileSystem.h"

#include "debug/SuzzaLog.h"

#include <fstream>
#include <cassert>

namespace suz
{
	/* 定义动态状态列表 */
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};


#pragma region PipelineLayoutSpec
	VkPipelineLayoutCreateInfo PipelineLayoutSpec::DefaultPipelineLayoutCreateInfo(const PipelineLayoutSpec& Spec)
	{
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		if (Spec.descriptorSetLayout.size() > 0)
		{
			layoutInfo.setLayoutCount = static_cast<uint32>(Spec.descriptorSetLayout.size());
			layoutInfo.pSetLayouts = Spec.descriptorSetLayout.data();
		}

		if (Spec.constantRage.size() > 0)
		{
			layoutInfo.pushConstantRangeCount = static_cast<uint32>(Spec.constantRage.size());
			layoutInfo.pPushConstantRanges = Spec.constantRage.data();
		}

		return layoutInfo;
	}

#pragma endregion


#pragma region PipelineSpec
	PipelineConfigInfo PipelineSpec::DefaultPipelineConfigInfo()
	{
		PipelineConfigInfo pipelineConfigInfo{};

		pipelineConfigInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		/* 告诉输入组装器要求构建三角形 */
		pipelineConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineConfigInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;					// 是否开启图元重启

		/* 将视口(负责绘制并显示在窗口给用户)和裁剪矩形合并成一个信息变量 */
		/* 选择动态视口和裁剪矩形时, 只需指定计数即可 */
		pipelineConfigInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineConfigInfo.viewportInfo.viewportCount = 1;
		pipelineConfigInfo.viewportInfo.scissorCount = 1;

		/* 光栅化阶段, 将几何体分解成片段, 每个绘制的像素对应三角形覆盖的部分 */
		pipelineConfigInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineConfigInfo.rasterizationInfo.depthClampEnable = VK_FALSE;				// 不开启限制gl_Position的z分量功能
		pipelineConfigInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;		// 光栅化前不丢弃所有图元
		pipelineConfigInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;		// 填充整个三角形
		pipelineConfigInfo.rasterizationInfo.lineWidth = 1.0f;							// 设置划线宽度为1.0f
		pipelineConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;				// 设置剔除正反面模式
		pipelineConfigInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;		// (CLOCKWISE):设置顺时针为正面
		pipelineConfigInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		pipelineConfigInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
		pipelineConfigInfo.rasterizationInfo.depthBiasClamp = 0.0f;
		pipelineConfigInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		/* 多重采样, 与物体锯齿问题有关 */
		pipelineConfigInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineConfigInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		pipelineConfigInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineConfigInfo.multisampleInfo.minSampleShading = 1.0f;
		pipelineConfigInfo.multisampleInfo.pSampleMask = nullptr;
		pipelineConfigInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineConfigInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

		/* 颜色混合设置, 物体重叠时重叠像素产生多个颜色 */
		pipelineConfigInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		pipelineConfigInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		pipelineConfigInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		pipelineConfigInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipelineConfigInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		pipelineConfigInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		pipelineConfigInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipelineConfigInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		pipelineConfigInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineConfigInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		pipelineConfigInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		pipelineConfigInfo.colorBlendInfo.blendConstants[0] = 0.0f;
		pipelineConfigInfo.colorBlendInfo.blendConstants[1] = 0.0f;
		pipelineConfigInfo.colorBlendInfo.blendConstants[2] = 0.0f;
		pipelineConfigInfo.colorBlendInfo.blendConstants[3] = 0.0f;

		/* 设置深度缓冲区, 为每个像素存储一个值, 通过设置深度值大小实现前后图层的效果 */
		pipelineConfigInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineConfigInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		pipelineConfigInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		pipelineConfigInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		pipelineConfigInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineConfigInfo.depthStencilInfo.minDepthBounds = 0.0f;
		pipelineConfigInfo.depthStencilInfo.maxDepthBounds = 1.0f;
		pipelineConfigInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		pipelineConfigInfo.depthStencilInfo.front = {};
		pipelineConfigInfo.depthStencilInfo.back = {};

		/* 创建动态状态管线信息结构体 */
		pipelineConfigInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipelineConfigInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
		pipelineConfigInfo.dynamicStateInfo.pDynamicStates = dynamicStates.data();

		return pipelineConfigInfo;
	}

	VkPipelineVertexInputStateCreateInfo PipelineSpec::DefaultVertexInputStateCreateInfo(const PipelineSpec& Spec)
	{
		if (Spec.bindingDescriptions.size() == 0 || Spec.attributeDescriptions.size() == 0)
		{
			SUZZA_LOG_ERROR("未配置顶点描述或属性描述, 生成顶点读取规则失败!");
			return {};
		}

		VkPipelineVertexInputStateCreateInfo inputStateCreateInfo{};
		inputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(Spec.attributeDescriptions.size());
		inputStateCreateInfo.pVertexAttributeDescriptions = Spec.attributeDescriptions.data();
		inputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32>(Spec.bindingDescriptions.size());
		inputStateCreateInfo.pVertexBindingDescriptions = Spec.bindingDescriptions.data();

		return inputStateCreateInfo;
	}

	VkGraphicsPipelineCreateInfo PipelineSpec::DefaultGraphicsPipelineCreateInfo(const PipelineSpec& Spec)
	{
		/* 创建图形管线对象 */
		VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
		graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		/* 将管线所有的配置信息整合 */
		graphicsPipelineInfo.stageCount = static_cast<uint32>(Spec.shaderStages.size());
		graphicsPipelineInfo.pStages = Spec.shaderStages.data();					// 设置着色器阶段信息

		graphicsPipelineInfo.pVertexInputState = &Spec.pipelineConfigInfo.vertexInputState;		// 设置解析顶点方法

		graphicsPipelineInfo.pViewportState = &Spec.pipelineConfigInfo.viewportInfo;
		graphicsPipelineInfo.pInputAssemblyState = &Spec.pipelineConfigInfo.inputAssemblyInfo;
		graphicsPipelineInfo.pRasterizationState = &Spec.pipelineConfigInfo.rasterizationInfo;
		graphicsPipelineInfo.pMultisampleState = &Spec.pipelineConfigInfo.multisampleInfo;
		graphicsPipelineInfo.pColorBlendState = &Spec.pipelineConfigInfo.colorBlendInfo;
		graphicsPipelineInfo.pDepthStencilState = &Spec.pipelineConfigInfo.depthStencilInfo;
		graphicsPipelineInfo.pDynamicState = &Spec.pipelineConfigInfo.dynamicStateInfo;

		graphicsPipelineInfo.layout = Spec.layout;
		graphicsPipelineInfo.renderPass = Spec.renderPass;
		graphicsPipelineInfo.subpass = Spec.pipelineConfigInfo.subpass;

		graphicsPipelineInfo.basePipelineIndex = -1;
		graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		return graphicsPipelineInfo;
	}

#pragma endregion

	Pipeline::~Pipeline()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class Pipeline Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool Pipeline::Create(PipelineLayoutSpec& Spec)
	{
		return CreatePipelineLayout(Spec);
	}

	bool Pipeline::Create(PipelineSpec& Spec)
	{
		return CreateGraphicsPipeline(Spec);
	}

	bool Pipeline::CreatePipelineLayout(PipelineLayoutSpec& Spec)
	{
		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建图形管线: 逻辑设备device不合法!");
			return false;
		}

		m_deviceContext = Spec.deviceContext;

		VkPipelineLayoutCreateInfo layoutCreateInfo = PipelineLayoutSpec::DefaultPipelineLayoutCreateInfo(Spec);
		if (vkCreatePipelineLayout(Spec.deviceContext.device, &layoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建管线布局失败!");
			return false;
		}

		return true;
	}

	bool Pipeline::CreateGraphicsPipeline(PipelineSpec& Spec)
	{
		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建图形管线: 逻辑设备device不合法!");
			return false;
		}

		m_deviceContext = Spec.deviceContext;

		if (Spec.layout == VK_NULL_HANDLE)
		{
			SUZZA_LOG_ERROR("无法创建图形管线: 没有合适的管线布局layout!");
			return false;
		}

		if (Spec.renderPass == VK_NULL_HANDLE)
		{
			SUZZA_LOG_ERROR("无法创建图形管线: 没有合适的队列renderPass!");
			return false;
		}

		/* 配置颜色混合设置 */
		Spec.pipelineConfigInfo.colorBlendInfo.attachmentCount = 1;
		Spec.pipelineConfigInfo.colorBlendInfo.pAttachments = &Spec.pipelineConfigInfo.colorBlendAttachment;

		VkGraphicsPipelineCreateInfo graphicsPipelineInfo = PipelineSpec::DefaultGraphicsPipelineCreateInfo(Spec);
		if (vkCreateGraphicsPipelines(Spec.deviceContext.device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建图形管线对象失败!");
			return false;
		}

		vkDestroyShaderModule(Spec.deviceContext.device, Spec.vertShaderMoudle, nullptr);
		vkDestroyShaderModule(Spec.deviceContext.device, Spec.fragShaderMoudle, nullptr);

		return true;
	}

	void Pipeline::Destroy()
	{
		if (!(*m_isAlive)) return;

		if (!m_deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("资源销毁失败: 逻辑设备device不合法!");
			return;
		}

		vkDestroyPipeline(m_deviceContext.device, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_deviceContext.device, m_pipelineLayout, nullptr);

		*m_isAlive = false;
	}

	void Pipeline::CreateShaderMoudle(const std::vector<char>& ShaderCode, VkShaderModule* ShaderMoudle)
	{
		const VkDevice& device = Engine::GetVulkanResourceManager().GetDevice()->GetDevice();

		VkShaderModuleCreateInfo shaderInfo{};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = ShaderCode.size();
		shaderInfo.pCode = reinterpret_cast<const uint32*>(ShaderCode.data());

		if (vkCreateShaderModule(device, &shaderInfo, nullptr, ShaderMoudle) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建着色器模块失败");
		}
	}
}