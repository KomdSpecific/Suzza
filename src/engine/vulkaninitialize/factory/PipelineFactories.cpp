#include "PipelineFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "file/FileSystem.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	namespace suz_pipeline
	{
		PipelineLayoutSpec CreateDefaultPipelineLayoutSpec()
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const VkDevice& device = manager.GetDevice()->GetDevice();
			const VkDescriptorSetLayout& cameraDescriptor = manager.GetDescriptorLayouts().GetResource("CameraDesc")->GetDescriptorLayout();
			const VkDescriptorSetLayout& bindlessDescriptor = manager.GetDescriptorLayouts().GetResource("BindlessDesc")->GetDescriptorLayout();

			PipelineLayoutSpec spec(device);
			spec.descriptorSetLayout.resize(2);
			spec.descriptorSetLayout[0] = cameraDescriptor;
			spec.descriptorSetLayout[1] = bindlessDescriptor;

			spec.constantRage.resize(1);
			spec.constantRage[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				// 推送常量给顶点着色器使用
			spec.constantRage[0].offset = 0;
			spec.constantRage[0].size = sizeof(ModelConstant);

			return spec;
		}

		PipelineSpec CreateDefaultGraphicsPipelineSpec(const std::string& VertexFilePath, const std::string& FragmentFilePath)
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const VkDevice& device = manager.GetDevice()->GetDevice();
			const VkRenderPass& renderPass = manager.GetRenderPasses().GetResource("DefaultRenderPass")->GetRenderPass();
			const VkPipelineLayout& layout = manager.GetPipelines().GetResource("DefaultPipeline")->GetPipelineLayout();

			PipelineSpec spec(device, renderPass, layout);

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
	}
}