#include "Pipeline.h"
#include "model/Mesh.h"

#include <glm/mat4x4.hpp>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace suz
{
	/* 定义动态状态列表 */
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};


	Pipeline::Pipeline(
		VulkanInitialize& VulkanInitialize,
		const VkRenderPass& RenderPass,
		const std::string& VertexFilePath,
		const std::string& FragmentFilePath,
		const PipelineConfigInfo& ConfigInfo
	) : m_vulkanInitialize(VulkanInitialize),
		m_renderPass(RenderPass)
	{
		CreateDescriptorSetLayout();
		CreatePushConstantRange();
		CreatePipelineLayout();
		CreateGraphicsPipeline(VertexFilePath, FragmentFilePath, ConfigInfo);
	}

	Pipeline::~Pipeline()
	{
		if (*m_isAlive)
		{
			std::cerr << "Class Pipeline Warning: 未调用Destroy手动销毁!" << std::endl;
			Destroy();
		}
	}

	void Pipeline::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyPipeline(m_vulkanInitialize.GetDevice(), m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_vulkanInitialize.GetDevice(), m_pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_vulkanInitialize.GetDevice(), m_descriptorSetLayout, nullptr);

		*m_isAlive = false;
	}

	PipelineConfigInfo Pipeline::DefaultPipelineConfigInfo()
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
		pipelineConfigInfo.colorBlendInfo.attachmentCount = 1;
		pipelineConfigInfo.colorBlendInfo.pAttachments = &pipelineConfigInfo.colorBlendAttachment;
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

	std::vector<char> Pipeline::ReadFile(const std::string& FilePath)
	{
		/* 打开文件时跳转到文件末尾, 并以二进制读取 */
		std::ifstream file(FilePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("打开文件失败!" + FilePath);
		}

		/* 初始化文件缓冲区大小 */
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		/* 跳转到文件开头并读取文件 */
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void Pipeline::CreateDescriptorSetLayout()
	{
		/* 定义描述符集绑定信息 */
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;											// 要求和着色器的Layout序列号一样
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				// 在顶点着色器阶段被引用
		uboLayoutBinding.pImmutableSamplers = nullptr;							// 与图像采样相关描述符有关

		/* 定义图像描述符集绑定信息 */
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;			// 在片段着色器阶段被引用
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		/* 定义Bindless全局集合 */
		VkDescriptorSetLayoutBinding bindlessTexBinding{};

		std::array<VkDescriptorSetLayoutBinding, 2> descriptorBinding{ uboLayoutBinding,samplerLayoutBinding };

		/* 定义描述符集配置信息 */
		VkDescriptorSetLayoutCreateInfo uboLayoutInfo{};
		uboLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		uboLayoutInfo.bindingCount = static_cast<uint32>(descriptorBinding.size());
		uboLayoutInfo.pBindings = descriptorBinding.data();
		if (vkCreateDescriptorSetLayout(m_vulkanInitialize.GetDevice(), &uboLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("创建描述字符集失败!");
		}
	}

	void Pipeline::CreatePushConstantRange()
	{
		m_pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;						// 推送常量给顶点着色器使用
		m_pushConstantRange.offset = 0;
		m_pushConstantRange.size = sizeof(glm::mat4);
	}

	void Pipeline::CreatePipelineLayout()
	{
		/* 设置管线布局创建信息 */
		VkPipelineLayoutCreateInfo pipeLayoutInfo{};
		pipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts = &m_descriptorSetLayout;					// 配置字符集布局信息
		pipeLayoutInfo.pushConstantRangeCount = 1;
		pipeLayoutInfo.pPushConstantRanges = &m_pushConstantRange;

		if (vkCreatePipelineLayout(m_vulkanInitialize.GetDevice(), &pipeLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("创建管线布局失败!");
		}
	}

	void Pipeline::CreateGraphicsPipeline(const std::string& VertexFilePath, const std::string& FragmentFilePath, const PipelineConfigInfo& ConfigInfo)
	{
		assert(m_renderPass != VK_NULL_HANDLE && "无法创建图形管线: 没有合适的队列renderPass!");

		std::vector<char> vertexShader = ReadFile(VertexFilePath);
		std::vector<char> fragmentShader = ReadFile(FragmentFilePath);

		CreateShaderMoudle(vertexShader, &m_vertShaderMoudle);
		CreateShaderMoudle(fragmentShader, &m_fragShaderMoudle);

		VkPipelineShaderStageCreateInfo shaderStages[2]{};
		/* 创建管线顶点着色器阶段信息 */
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = m_vertShaderMoudle;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;						// 定制着色器功能

		/* 创建管线片段着色器阶段信息 */
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = m_fragShaderMoudle;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;						// 定制着色器功能

		/* 获取顶点绑定和顶点属性描述信息, 告诉如何获取顶点属性 */
		VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::GetAttributeDescription();
		/* 定义如何解析顶点缓冲数据 */
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());// 顶点属性描述数量
		vertexInputInfo.vertexBindingDescriptionCount = 1;									// 顶点绑定描述数量
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

		/* 创建图形管线对象 */
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		/* 将管线所有的配置信息整合一起 */
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;								// 设置着色器阶段信息
		pipelineInfo.pVertexInputState = &vertexInputInfo;					// 设置解析顶点方法
		pipelineInfo.pInputAssemblyState = &ConfigInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &ConfigInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &ConfigInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &ConfigInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &ConfigInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &ConfigInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &ConfigInfo.dynamicStateInfo;

		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = ConfigInfo.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;			// TODO: 性能优化点

		/* 真正创建图形管线过程 */
		if (vkCreateGraphicsPipelines(m_vulkanInitialize.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("创建图形管线对象失败!");
		}

		/* 销毁顶点和片段着色器模块 */
		vkDestroyShaderModule(m_vulkanInitialize.GetDevice(), m_vertShaderMoudle, nullptr);
		vkDestroyShaderModule(m_vulkanInitialize.GetDevice(), m_fragShaderMoudle, nullptr);
	}

	void Pipeline::CreateShaderMoudle(const std::vector<char>& ShaderCode, VkShaderModule* ShaderMoudle)
	{
		VkShaderModuleCreateInfo shaderCreateInfo{};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.codeSize = ShaderCode.size();
		shaderCreateInfo.pCode = reinterpret_cast<const uint32*>(ShaderCode.data());

		if (vkCreateShaderModule(m_vulkanInitialize.GetDevice(), &shaderCreateInfo, nullptr, ShaderMoudle) != VK_SUCCESS)
		{
			throw std::runtime_error("创建着色器模块失败!");
		}
	}
}