#pragma once

#include "SuzzaTypes.h"
#include "structtype/ModelStructor.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>

namespace suz
{
	struct MaterialInfo
	{
		glm::vec4 baseColorFactor{ 1,1,1,1 };
		
		std::string baseColorTexturePath;
	};

	struct ModelData
	{
		std::string modelFolderPath;				// 模型所在文件夹路径

		std::vector<ModelVertexData> vertices;
		std::vector<uint32> indices;
		MaterialInfo material;
	};

	class IModelLoader
	{
	public:
		IModelLoader() = default;
		virtual std::vector<ModelData> LoadTargetModel(const std::string& FilePath) = 0;
		virtual void UnLoadTargetModel(ModelData& ModelData) = 0;
		virtual bool TargetModelFormatSupport(const std::string& ExtensionName) = 0;
	};
}