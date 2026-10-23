#include "StbImageLoader.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace suz
{
	ImageData StbImageLoader::LoadTargetImageFile(const std::string& FilePath)
	{
		ImageData image{};
		int width, height, channels;
		uint8_t* ptr = stbi_load(FilePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!ptr)
		{
			throw std::runtime_error("加载纹理图像失败!");
		}

		image.texWidth = width;
		image.texHeight = height;
		image.texChannels = channels;
		image.pixels = ptr;
		/* 绑定对应的lambda函数 */
		image.releaseDataCallback = [ptr]()
			{
				if (ptr)
				{
					stbi_image_free(ptr);
				}
			};

		return image;
	}

	void StbImageLoader::FreeImage(ImageData& Image)
	{
		if (Image.releaseDataCallback)
		{
			Image.releaseDataCallback();
			Image.releaseDataCallback = nullptr;
			Image.pixels = nullptr;
		}
	}

	bool StbImageLoader::QuerySupportFormat(const std::string& Extension) const
	{
		return true;
	}
}