#pragma once

#include <glm/glm.hpp>

namespace suz
{
	class Math
	{
	public:
		static const glm::mat4 GameSceneLookAt(const glm::vec3& Eye, const glm::vec3& Target, const glm::vec3 Up);
	};
}