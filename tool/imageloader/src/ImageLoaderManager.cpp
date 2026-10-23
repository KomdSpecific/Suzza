#include "ImageLoaderManager.h"

namespace suz
{
    void ImageLoaderManager::CreateImageDispatcher()
    {
        m_loaderDispatcher = std::make_unique<ImageLoaderDispatcher>();
        m_loaderDispatcher->RegisterStbImageLoader();
    }

    ImageData ImageLoaderManager::LoadImage(const std::string& FilePath)
    {
        return m_loaderDispatcher->LoadTargetImageFile(FilePath);
    }

    void ImageLoaderManager::UnLoadImage(ImageData& Image)
    {
        m_loaderDispatcher->FreeImage(Image);
    }

    bool ImageLoaderManager::IsSupportFileFormat(const std::string& Extension)
    {
        return m_loaderDispatcher->QuerySupportFormat(Extension);
    }
}