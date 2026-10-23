#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace suz
{
	class Descriptor : public BaseObject
	{
	public:
		Descriptor(const VkDevice& Device);
		~Descriptor();

		void InitDescriptor();

		virtual void Destroy() override;

		void UpdateBindlessDescriptor(const VkImageView& ImageView, const VkSampler& Sampler, uint32 SlotIndex);

	private:
		void CreateCameraDescriptorPool();
		void CreateCameraDescriptorSets();

		void CreateBindlessDescriptorPool();
		void CreateBindlessDescriptorSets();

		const VkDevice& m_device;

		VkDescriptorPool m_cameraDescriptorPool = {};
		std::vector<VkDescriptorSet> m_cameraDescriptorSet = {};

		VkDescriptorPool m_bindlessDescriptorPool = {};
		VkDescriptorSet m_bindlessDescriptorSet = {};

	public:
		inline const VkDescriptorPool& GetCameraDescriptorPool() const { return m_cameraDescriptorPool; }
		inline const std::vector<VkDescriptorSet>& GetCameraDescriptorSets() const { return m_cameraDescriptorSet; }
		inline const VkDescriptorSet& GetCameraDescriptorSetElement(uint32_t Index) const { return m_cameraDescriptorSet[Index]; }

		inline const VkDescriptorPool& GetBindlessDescriptorPool() const { return m_bindlessDescriptorPool; }
		inline const VkDescriptorSet& GetBindlessDescriptorSets() const { return m_bindlessDescriptorSet; }
	};
}