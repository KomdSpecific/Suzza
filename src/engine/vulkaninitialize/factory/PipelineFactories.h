#pragma once

#include "vulkaninitialize/Pipeline.h"

namespace suz
{
	/* 上层手写硬编码的工厂函数 */
	namespace suz_pipeline
	{
		PipelineLayoutSpec CreateDefaultPipelineLayoutSpec();
		PipelineSpec CreateDefaultGraphicsPipelineSpec(const String& VertexFilePath, const String& FragmentFilePath);

		PipelineSpec CreateDefaultGridPipelineSpec(const String& VertexFilePath, const String& FragmentFilePath);
	}
}