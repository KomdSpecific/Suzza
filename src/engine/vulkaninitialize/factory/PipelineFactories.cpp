#include "PipelineFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "file/FileSystem.h"
#include "model/Mesh.h"
#include "model/RenderTypes.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	namespace suz_pipeline
	{
		PipelineLayoutSpec CreateDefaultPipelineLayoutSpec()
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const DeviceContext& deviceContext = manager.GetDeviceContext();
			const VkDescriptorSetLayout& cameraDescriptor = manager.GetDescriptorLayouts().GetResource("CameraDesc")->GetDescriptorLayout();
			const VkDescriptorSetLayout& bindlessDescriptor = manager.GetDescriptorLayouts().GetResource("BindlessDesc")->GetDescriptorLayout();

			PipelineLayoutSpec spec(deviceContext);
			spec.descriptorSetLayout.resize(2);
			spec.descriptorSetLayout[0] = cameraDescriptor;
			spec.descriptorSetLayout[1] = bindlessDescriptor;

			spec.constantRage.resize(1);
			spec.constantRage[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				// 推送常量给顶点着色器使用
			spec.constantRage[0].offset = 0;
			spec.constantRage[0].size = sizeof(ModelConstant);

			return spec;
		}

		PipelineSpec CreateDefaultGraphicsPipelineSpec(const String& VertexFilePath, const String& FragmentFilePath)
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const DeviceContext& deviceContext = manager.GetDeviceContext();
			const VkRenderPass& renderPass = manager.GetRenderPasses().GetResource("DefaultRenderPass")->GetRenderPass();
			const VkPipelineLayout& layout = manager.GetPipelines().GetResource("DefaultPipeline")->GetPipelineLayout();

			PipelineSpec spec(deviceContext, renderPass, layout);

			Vector<char> vertexShader = FileSystem::ReadFile(VertexFilePath);
			Vector<char> fragmentShader = FileSystem::ReadFile(FragmentFilePath);

			/* 创建着色器模块 */
			Pipeline::CreateShaderMoudle(vertexShader, &spec.vertShaderMoudle);
			Pipeline::CreateShaderMoudle(fragmentShader, &spec.fragShaderMoudle);

			spec.shaderStages.resize(2);
			/* 创建管线顶点着色器阶段信息 */
			spec.shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			spec.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			spec.shaderStages[0].module = spec.vertShaderMoudle;
			spec.shaderStages[0].pName = "main";
			spec.shaderStages[0].flags = 0;
			spec.shaderStages[0].pNext = nullptr;
			spec.shaderStages[0].pSpecializationInfo = nullptr;						// 定制着色器功能

			spec.shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			spec.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			spec.shaderStages[1].module = spec.fragShaderMoudle;
			spec.shaderStages[1].pName = "main";
			spec.shaderStages[1].flags = 0;
			spec.shaderStages[1].pNext = nullptr;
			spec.shaderStages[1].pSpecializationInfo = nullptr;						// 定制着色器功能

			/* 设置顶点和属性读取规则 */
			spec.bindingDescriptions = { Vertex::GetBindingDescription() };
			spec.attributeDescriptions = Vertex::GetAttributeDescription();

			spec.pipelineConfigInfo = PipelineSpec::DefaultPipelineConfigInfo();	// 固定配置

			spec.pipelineConfigInfo.vertexInputState = PipelineSpec::DefaultVertexInputStateCreateInfo(spec);// 设置解析顶点方法

			return spec;
		}

		PipelineSpec CreateDefaultGridPipelineSpec(const String& VertexFilePath, const String& FragmentFilePath)
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const DeviceContext& deviceContext = manager.GetDeviceContext();
			const VkRenderPass& renderPass = manager.GetRenderPasses().GetResource("DefaultRenderPass")->GetRenderPass();
			const VkPipelineLayout& layout = manager.GetPipelines().GetResource("GridPipeline")->GetPipelineLayout();

			PipelineSpec spec(deviceContext, renderPass, layout);

			Vector<char> vertexShader = FileSystem::ReadFile(VertexFilePath);
			Vector<char> fragmentShader = FileSystem::ReadFile(FragmentFilePath);

			/* 创建着色器模块 */
			Pipeline::CreateShaderMoudle(vertexShader, &spec.vertShaderMoudle);
			Pipeline::CreateShaderMoudle(fragmentShader, &spec.fragShaderMoudle);

			spec.shaderStages.resize(2);
			/* 创建管线顶点着色器阶段信息 */
			spec.shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			spec.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			spec.shaderStages[0].module = spec.vertShaderMoudle;
			spec.shaderStages[0].pName = "main";
			spec.shaderStages[0].flags = 0;
			spec.shaderStages[0].pNext = nullptr;
			spec.shaderStages[0].pSpecializationInfo = nullptr;						// 定制着色器功能

			spec.shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			spec.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			spec.shaderStages[1].module = spec.fragShaderMoudle;
			spec.shaderStages[1].pName = "main";
			spec.shaderStages[1].flags = 0;
			spec.shaderStages[1].pNext = nullptr;
			spec.shaderStages[1].pSpecializationInfo = nullptr;						// 定制着色器功能

			/* 设置顶点和属性读取规则 */
			spec.bindingDescriptions.clear();
			spec.attributeDescriptions.clear();

			spec.pipelineConfigInfo = PipelineSpec::DefaultPipelineConfigInfo();	// 固定配置

			spec.pipelineConfigInfo.vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			spec.pipelineConfigInfo.vertexInputState.vertexAttributeDescriptionCount = 0;
			spec.pipelineConfigInfo.vertexInputState.pVertexAttributeDescriptions = nullptr;
			spec.pipelineConfigInfo.vertexInputState.vertexBindingDescriptionCount = 0;
			spec.pipelineConfigInfo.vertexInputState.pVertexBindingDescriptions = nullptr;

			/* 全屏三角形, 不做剔除 (绕序由自己决定) */
			spec.pipelineConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
			
			/* 深度: 测试开, 写入关. 网格是"无限远"的东西,
			不写深度可以避免后续任何东西被网格挡住 */
			spec.pipelineConfigInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
			spec.pipelineConfigInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;
			spec.pipelineConfigInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

			/* 透明度混合 (只影响这一条管线) */
			spec.pipelineConfigInfo.colorBlendAttachment.blendEnable = VK_TRUE;
			spec.pipelineConfigInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			spec.pipelineConfigInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			spec.pipelineConfigInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			spec.pipelineConfigInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			spec.pipelineConfigInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

			return spec;
		}
	}
}