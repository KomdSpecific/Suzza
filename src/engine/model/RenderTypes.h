#pragma once

#include "SuzzaTypes.h"

#include <glm/mat4x4.hpp>

namespace suz
{
	struct ModelConstant
	{
		glm::mat4 modelMatrix;
		uint32 slotIndex;
	};

	static_assert(sizeof(ModelConstant) == 68, "ModelConstant布局与vertexShader.vert布局不一致!");
	static_assert(offsetof(ModelConstant, ModelConstant::slotIndex) == 64, "slotIndex必须位于ModelConstant的偏移量64处!");


	struct CameraViewportInfo
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	static_assert(sizeof(CameraViewportInfo) == 128, "CameraViewportInfo的布局与vertexShader.vert的 uniform 不一致!");
}