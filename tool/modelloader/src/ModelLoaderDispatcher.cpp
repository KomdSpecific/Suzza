#include "ModelLoaderDispatcher.h"
#include "cgltf/CgltfModelLoader.h"

#include <algorithm>

namespace suz
{
	std::string ModelLoaderDispatcher::GetFileExtensionName(const std::string& FilePath)
	{
		usize extensionIndex = FilePath.find_last_of(".");
		if (extensionIndex == std::string::npos) return {};

		std::string extensionName = FilePath.substr(extensionIndex);
		/* 将扩展名转换成小写统一判断 */
		std::transform(extensionName.begin(), extensionName.begin(), extensionName.end(),
			[](char c)
			{
				return static_cast<char>(tolower(c));
			});

		return extensionName;
	}

	void ModelLoaderDispatcher::RegisterModelLoader(std::unique_ptr<IModelLoader> ModelLoader)
	{
		m_modelLoaders.push_back(std::move(ModelLoader));
	}

	void ModelLoaderDispatcher::RegisterCgltfModelLoader()
	{
		std::unique_ptr<IModelLoader> modelLoader = std::make_unique<CgltfModelLoader>();
		RegisterModelLoader(std::move(modelLoader));
	}

	std::vector<ModelData> ModelLoaderDispatcher::LoadTargetModel(const std::string& FilePath)
	{
		std::string extensionName = GetFileExtensionName(FilePath);
		for (const auto& loader : m_modelLoaders)
		{
			/* 找到支持该格式的加载器, 直接调用 */
			if (loader->TargetModelFormatSupport(extensionName))
			{
				return loader->LoadTargetModel(FilePath);
			}
		}

		return {};
	}

	void ModelLoaderDispatcher::UnLoadTargetModel(ModelData& ModelData)
	{
	}

	bool ModelLoaderDispatcher::TargetModelFormatSupport(const std::string& ExtensionName)
	{
		for (const auto& loader : m_modelLoaders)
		{
			/* 只要有库支持查询即可返回True */
			if (loader->TargetModelFormatSupport(ExtensionName))
			{
				return true;
			}
		}

		return false;
	}
}