#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>

namespace suz
{
	class Texture : public BaseObject
	{
	public:
		Texture(const VkDevice& Device);
		~Texture();

		void InitTexture(const std::string FilePath = "");

		virtual void Destroy() override;

		/* 创建图像 */
		static void CreateImage(uint32_t Width, uint32_t Height,
			const VkFormat& Format,
			const VkImageTiling& Tiling,
			const VkImageUsageFlags& Usage, const VkMemoryPropertyFlags& Properties,
			VkImage& Image, VkDeviceMemory& ImageMemory);

		/* 创建图像视图 */
		static VkImageView CreateImageView(const VkImage& Image, const VkFormat& Format, const VkImageAspectFlags& AspectFlags);

		void UpdateTextureSlot(uint16 SlotIndex);

	private:
		void CreateTextureImage(const std::string FilePath);		// 创建纹理图像
		void CreateTextureImageView();								// 创建纹理图像视图

		const VkDevice& m_device;

		VkImage m_textureImage = {};
		VkDeviceMemory m_textureMemory = {};
		VkImageView m_textureImageView = {};

	public:
		inline const VkImage& GetTextureImage() const { return m_textureImage; }
		inline const VkDeviceMemory& GetTextureMemory() const { return m_textureMemory; }
		inline const VkImageView& GetTextureImageView() const { return m_textureImageView; }
	};
}