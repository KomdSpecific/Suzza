#pragma once

#include "ImageLoader.h"

#include <vector>
#include <memory>

namespace suz
{
	class ImageLoaderDispatcher : public IImageLoader
	{
	public:
		static std::string GetFileExtension(const std::string& FilePath);

		void RegisterImageLoader(std::unique_ptr<IImageLoader> Loader);
		void RegisterStbImageLoader();

		virtual ImageData LoadTargetImageFile(const std::string& FilePath) override;
		virtual void FreeImage(ImageData& Image) override;
		virtual bool QuerySupportFormat(const std::string& Extension) const override;

	private:
		std::vector<std::unique_ptr<IImageLoader>> m_loaders = {};
	};
}