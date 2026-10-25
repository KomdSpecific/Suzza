#include "Texture.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/VulkanInitialize.h"
#include "vulkaninitialize/BufferArea.h"
#include "ImageLoaderManager.h"
#include "file/FileSystem.h"
#include "device/DeviceFunctionLibrary.h"

#include "debug/SuzzaLog.h"

#include <stdexcept>

namespace suz
{
	Texture::Texture(const VkDevice& Device)
		: m_device(Device)
	{
	}

	Texture::~Texture()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class Texture Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void Texture::InitTexture(const std::string FilePath)
	{
		CreateTextureImage(FilePath);
		CreateTextureImageView();
	}

	void Texture::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyImageView(m_device, m_textureImageView, nullptr);

		vkDestroyImage(m_device, m_textureImage, nullptr);
		vkFreeMemory(m_device, m_textureMemory, nullptr);

		*m_isAlive = false;
	}

	void Texture::CreateImage(uint32_t Width, uint32_t Height,
		const VkFormat& Format,
		const VkImageTiling& Tiling,
		const VkImageUsageFlags& Usage, const VkMemoryPropertyFlags& Properties,
		VkImage& Image, VkDeviceMemory& ImageMemory)
	{
		/* 定义图像创建信息 */
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;					// 图像设为2D视图
		imageInfo.extent.width = Width;
		imageInfo.extent.height = Height;
		imageInfo.extent.depth = 1;								// 图像深度值为1
		imageInfo.mipLevels = 1;								// 不生成mipmap
		imageInfo.arrayLayers = 1;								// 单层图像
		imageInfo.format = Format;								// 像素格式
		imageInfo.tiling = Tiling;								// 像素在显存中的排列方式
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// 初始布局: 无有效数据(后期进行布局转换)
		imageInfo.usage = Usage;								// 图像用途标记, 驱动对此优化
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;				// 无多重采样
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// 单队列独占: 性能最优

		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		VkDevice device = vkManager.GetVulkanInitialize()->GetDevice();
		if (vkCreateImage(device, &imageInfo, nullptr, &Image) != VK_SUCCESS)
		{
			throw std::runtime_error("创建图像失败!");
		}

		/* 查询图像显存需求 */
		VkMemoryRequirements memoryRequire{};
		vkGetImageMemoryRequirements(device, Image, &memoryRequire);

		/* 分配符合需求的显存 */
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequire.size;		// 设置显存大小
		/* 查找符合 "内存类型位掩码 + 属性要求" 的显存类型索引*/
		allocateInfo.memoryTypeIndex = BufferArea::FindMemoryType(memoryRequire.memoryTypeBits, Properties);

		if (vkAllocateMemory(device, &allocateInfo, nullptr, &ImageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("分配图像显存失败!");
		}

		/* 将显存绑定到图像 */
		/* 索引为0: 将整张图像绑定一整块显存*/
		vkBindImageMemory(device, Image, ImageMemory, 0);
	}

	VkImageView Texture::CreateImageView(const VkImage& Image, const VkFormat& Format, const VkImageAspectFlags& AspectFlags)
	{
		VkImageViewCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageCreateInfo.image = Image;									// 设置源图像
		imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;				// 设为2D视图
		imageCreateInfo.format = Format;

		/* 子资源范围: 指定图像中可见的部分 */
		imageCreateInfo.subresourceRange.aspectMask = AspectFlags;		// 可见通道面
		imageCreateInfo.subresourceRange.baseMipLevel = 0;				// 起始mip层级
		imageCreateInfo.subresourceRange.levelCount = 1;				// 层级数量
		imageCreateInfo.subresourceRange.baseArrayLayer = 0;			// 起始图层
		imageCreateInfo.subresourceRange.layerCount = 1;				// 图层数量

		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		VkDevice device = vkManager.GetVulkanInitialize()->GetDevice();

		VkImageView imageView{};
		if (vkCreateImageView(device, &imageCreateInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("创建图像视图失败!");
		}

		return imageView;
	}

	void Texture::UpdateTextureSlot(uint16 SlotIndex)
	{
		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		vkManager.GetDescriptor()->UpdateBindlessDescriptor(m_textureImageView, vkManager.GetTextureSampler(), SlotIndex);
	}

	void Texture::CreateTextureImage(const std::string FilePath)
	{
		BufferArea* bufferArea = Engine::GetVulkanResourceManager().GetBufferArea();

		ImageData image{};
		if (FilePath.empty())
		{
			std::filesystem::path filePath = FileSystem::GetAssetsFolder() / "textures" / "brick.jpg";
			image = Engine::GetImageLoaderManager().LoadImage(filePath.string());
		}
		else
		{
			image = Engine::GetImageLoaderManager().LoadImage(FilePath);
		}

		VkDeviceSize imageSize = image.texWidth * image.texHeight * 4;

		/* 创建暂存缓冲区, 用于临时存储 */
		VkBuffer stagingBuffer{};
		VkDeviceMemory stagingMemory{};

		BufferArea::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

		/* 将显存拷贝到CPU可读取的内存中 */
		void* imageData;
		vkMapMemory(m_device, stagingMemory, 0, imageSize, 0, &imageData);
		memcpy(imageData, image.pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device, stagingMemory);

		/* 清理原始像素数组 */
		Engine::GetImageLoaderManager().UnLoadImage(image);

		CreateImage(image.texWidth, image.texHeight, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_textureImage, m_textureMemory
		);

		bufferArea->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		/* 复制缓冲区到图像 */
		bufferArea->CopyBufferToImage(stagingBuffer, m_textureImage, image.texWidth, image.texHeight);
		/* 转换图像格式以便于着色器访问 */
		bufferArea->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		/* 销毁临时缓冲区 */
		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingMemory, nullptr);
	}

	void Texture::CreateTextureImageView()
	{
		m_textureImageView = CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}