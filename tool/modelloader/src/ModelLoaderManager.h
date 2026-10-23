#pragma once

#include "ModelLoaderDispatcher.h"

namespace suz
{
	class ModelLoaderManager
	{
	public:
		ModelLoaderManager() = default;

		void CreateModelLoaderDispatcher();

		std::vector<ModelData> LoadModel(const std::string& FilePath);
		void UnLoadModel(ModelData& TargetModelData);
		bool IsModelFormatSupport(const std::string& ExtensionName);

	private:
		ModelLoaderManager(const ModelLoaderManager&) = delete;
		ModelLoaderManager& operator=(const ModelLoaderManager&) = delete;

		std::unique_ptr<ModelLoaderDispatcher> m_modelLoaderDispatcher = nullptr;
	};
}