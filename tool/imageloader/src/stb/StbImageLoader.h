#pragma once

#include "ImageLoader.h"

namespace suz
{
	class StbImageLoader: public IImageLoader
	{
	public:
		virtual ImageData LoadTargetImageFile(const std::string& FilePath) override;
		virtual void FreeImage(ImageData& Image) override;
		virtual bool QuerySupportFormat(const std::string& Extension) const override;
	};
}