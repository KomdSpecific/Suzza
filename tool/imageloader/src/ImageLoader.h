#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace suz
{
	struct ImageData
	{
		uint8_t* pixels = nullptr;
		uint32_t texWidth = 0;
		uint32_t texHeight = 0;
		uint32_t texChannels = 0;
		std::function<void()> releaseDataCallback;
	};

	class IImageLoader
	{
	public:
		virtual ~IImageLoader() = default;
		virtual ImageData LoadTargetImageFile(const std::string& FilePath) = 0;
		virtual void FreeImage(ImageData& Image) = 0;
		virtual bool QuerySupportFormat(const std::string& Extension) const = 0;
	};
}