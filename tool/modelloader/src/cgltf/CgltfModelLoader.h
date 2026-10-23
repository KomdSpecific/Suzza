#pragma once

#include "ModelLoader.h"

namespace suz
{
	class CgltfModelLoader : public IModelLoader
	{
	public:
		virtual std::vector<ModelData> LoadTargetModel(const std::string& FilePath) override;
		virtual void UnLoadTargetModel(ModelData& ModelData) override;
		virtual bool TargetModelFormatSupport(const std::string& ExtensionName) override;

	private:
		std::string GetFileFolderPath(const std::string& FilePath);
	};
}