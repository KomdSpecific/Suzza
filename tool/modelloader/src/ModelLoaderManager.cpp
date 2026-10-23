#include "ModelLoaderManager.h"

namespace suz
{
	void ModelLoaderManager::CreateModelLoaderDispatcher()
	{
		m_modelLoaderDispatcher = std::make_unique<ModelLoaderDispatcher>();
		m_modelLoaderDispatcher->RegisterCgltfModelLoader();
	}

	std::vector<ModelData> ModelLoaderManager::LoadModel(const std::string& FilePath)
	{
		return m_modelLoaderDispatcher->LoadTargetModel(FilePath);
	}

	void ModelLoaderManager::UnLoadModel(ModelData& TargetModelData)
	{
		m_modelLoaderDispatcher->UnLoadTargetModel(TargetModelData);
	}

	bool ModelLoaderManager::IsModelFormatSupport(const std::string& ExtensionName)
	{
		return m_modelLoaderDispatcher->TargetModelFormatSupport(ExtensionName);
	}
}