#include "ImageLoaderDispatcher.h"
#include "stb/StbImageLoader.h"

#include <algorithm>

namespace suz
{
	std::string ImageLoaderDispatcher::GetFileExtension(const std::string& FilePath)
	{
		size_t extensionDot = FilePath.find_last_of(".");
		if (extensionDot == std::string::npos) return {};

		std::string extensionName = FilePath.substr(extensionDot);
		/* 将扩展名转换成小写统一判断 */
		std::transform(extensionName.begin(), extensionName.end(), extensionName.begin(),
			[](char c) {
				return static_cast<char>(tolower(c));
			});

		return extensionName;
	}

	void ImageLoaderDispatcher::RegisterImageLoader(std::unique_ptr<IImageLoader> Loader)
	{
		m_loaders.push_back(std::move(Loader));
	}

	void ImageLoaderDispatcher::RegisterStbImageLoader()
	{
		std::unique_ptr<IImageLoader> stbLoader = std::make_unique<StbImageLoader>();
		RegisterImageLoader(std::move(stbLoader));
	}

	ImageData ImageLoaderDispatcher::LoadTargetImageFile(const std::string& FilePath)
	{
		std::string extensionName = GetFileExtension(FilePath);

		for (const auto& loader : m_loaders)
		{
			/* 找到支持该格式的加载器, 直接调用 */
			if (loader->QuerySupportFormat(extensionName))
			{
				return loader->LoadTargetImageFile(FilePath);
			}
		}

		return {};
	}

	void ImageLoaderDispatcher::FreeImage(ImageData& Image)
	{
		/* 执行对应的释放函数, 不用记录加载器类型 */
		if (Image.releaseDataCallback)
		{
			Image.releaseDataCallback();
			Image.releaseDataCallback = nullptr;
			Image.pixels = nullptr;
		}
	}

	bool ImageLoaderDispatcher::QuerySupportFormat(const std::string& Extension) const
	{
		for (const auto& loader : m_loaders)
		{
			/* 只要有库支持查询即可返回True */
			if (loader->QuerySupportFormat(Extension))
			{
				return true;
			}
		}

		return false;
	}
}