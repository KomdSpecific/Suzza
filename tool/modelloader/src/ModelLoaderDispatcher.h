#pragma once

#include "ModelLoader.h"

#include <memory>

namespace suz
{
	class ModelLoaderDispatcher : public IModelLoader
	{
	public:
		std::string GetFileExtensionName(const std::string& FilePath);

		void RegisterModelLoader(std::unique_ptr<IModelLoader> ModelLoader);
		void RegisterCgltfModelLoader();

		virtual std::vector<ModelData> LoadTargetModel(const std::string& FilePath) override;
		virtual void UnLoadTargetModel(ModelData& ModelData) override;
		virtual bool TargetModelFormatSupport(const std::string& ExtensionName) override;

	private:
		std::vector<std::unique_ptr<IModelLoader>> m_modelLoaders = {};
	};
}