#pragma once

#include "ImageLoaderDispatcher.h"

namespace suz
{
	class ImageLoaderManager
	{
	public:
		ImageLoaderManager() = default;

		void CreateImageDispatcher();

		ImageData LoadImage(const std::string& FilePath);
		void UnLoadImage(ImageData& Image);
		bool IsSupportFileFormat(const std::string& Extension);
		
	private:
		ImageLoaderManager(const ImageLoaderManager&) = delete;
		ImageLoaderManager& operator=(const ImageLoaderManager&) = delete;

		std::unique_ptr<ImageLoaderDispatcher> m_loaderDispatcher = {};
	};
}