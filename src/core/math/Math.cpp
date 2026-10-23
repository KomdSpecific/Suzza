#include "Math.h"

namespace suz
{
	const glm::mat4 Math::GameSceneLookAt(const glm::vec3& Eye, const glm::vec3& Target, const glm::vec3 Up)
	{
		glm::vec3 forward = glm::normalize(Target - Eye);
		glm::vec3 right = glm::normalize(glm::cross(forward, Up));
		glm::vec3 newUp = glm::cross(right, forward);

		/* 设置三维空间各个坐标轴方向 */
		glm::mat4 view(1.0f);
		view[0][0] = -right.x;	view[0][1] = newUp.x;	view[0][2] = -forward.x;
		view[1][0] = -right.y;	view[1][1] = newUp.y;	view[1][2] = -forward.y;
		view[2][0] = -right.z;	view[2][1] = newUp.z;	view[2][2] = -forward.z;

		view[3][0] = glm::dot(right, Eye);
		view[3][1] = -glm::dot(newUp, Eye);
		view[3][2] = glm::dot(forward, Eye);
		return view;
	}
}