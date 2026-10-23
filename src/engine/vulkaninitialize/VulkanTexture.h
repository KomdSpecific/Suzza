#pragma once

#include "BaseObject.h"

#include <vulkan/vulkan.h>
#include <cstdint>

namespace suz
{
	class VulkanTexture : public BaseObject
	{
	public:
		VulkanTexture(const VkDevice& Device);
		~VulkanTexture();

		void InitVulkanTexture();

		virtual void Destroy() override;

		/* 创建图像 */
		static void CreateImage(uint32_t Width, uint32_t Height,
			const VkFormat& Format,
			const VkImageTiling& Tiling,
			const VkImageUsageFlags& Usage, const VkMemoryPropertyFlags& Properties,
			VkImage& Image, VkDeviceMemory& ImageMemory);

		/* 创建图像视图 */
		static VkImageView CreateImageView(const VkImage& Image, const VkFormat& Format, const VkImageAspectFlags& AspectFlags);

	private:
		void CreateTextureImage();				// 创建纹理图像
		void CreateTextureImageView();			// 创建纹理图像视图
		void CreateTextureSampler();			// 创建采样器

		const VkDevice& m_device;

		VkImage m_textureImage = {};
		VkDeviceMemory m_textureMemory = {};
		VkImageView m_textureImageView = {};
		VkSampler m_textureSampler = {};

	public:
		inline const VkImage& GetTextureImage() const { return m_textureImage; }
		inline const VkDeviceMemory& GetTextureMemory() const { return m_textureMemory; }
		inline const VkImageView& GetTextureImageView() const { return m_textureImageView; }
		inline const VkSampler& GetTextureSampler() const { return m_textureSampler; }
	};
}