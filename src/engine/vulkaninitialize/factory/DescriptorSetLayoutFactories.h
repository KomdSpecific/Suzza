#pragma once

#include "vulkaninitialize/DescriptorSetLayout.h"

namespace suz
{
	/* 上层手写硬编码的工厂函数 */
	namespace suz_descLayout
	{
		DescriptorSetLayoutSpec CreateCameraDescriptorSetLayoutSpec();
		DescriptorSetLayoutSpec CreateBindlessDescriptorSetLayoutSpec();
	}
}