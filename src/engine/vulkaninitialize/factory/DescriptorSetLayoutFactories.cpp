#include "DescriptorSetLayoutFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "macros/VkMacro.h"

namespace suz
{
	namespace suz_descLayout
	{
		DescriptorSetLayoutSpec CreateCameraDescriptorSetLayoutSpec()
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const VkDevice& device = manager.GetDevice()->GetDevice();

			DescriptorSetLayoutSpec cameraSpec(device);
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = 0;											// 要求和着色器的Layout序列号一样
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				// 在顶点着色器阶段被引用
			layoutBinding.pImmutableSamplers = nullptr;							// 与图像采样相关描述符有关

			cameraSpec.descriptorBinding.push_back(layoutBinding);

			return cameraSpec;
		}

		DescriptorSetLayoutSpec CreateBindlessDescriptorSetLayoutSpec()
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const VkDevice& device = manager.GetDevice()->GetDevice();

			DescriptorSetLayoutSpec bindlessSpec(device);
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = 1;											// 要求和着色器的Layout序列号一样
			layoutBinding.descriptorCount = MAX_SAMPLED_TEXTURE_IMAGES_SLOTS;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;			// 在片段着色器阶段被引用
			layoutBinding.pImmutableSamplers = nullptr;							// 与图像采样相关描述符有关

			bindlessSpec.descriptorBinding.push_back(layoutBinding);

			VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			bindlessSpec.flags.push_back(flags);
			/* 定义Bindless的Flag */
			bindlessSpec.bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;

			return bindlessSpec;
		}
	}
}