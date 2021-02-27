#pragma once

#include "Utils/Maths.h"

namespace RS
{
	struct AABB
	{
		glm::vec3 min = glm::vec3(0.f);
		glm::vec3 max = glm::vec3(0.f);

		static AABB Transform(const AABB& aabb, const glm::mat4& transform)
		{
			glm::vec4 minT = transform * glm::vec4(aabb.min, 1.f);
			glm::vec4 maxT = transform * glm::vec4(aabb.max, 1.f);
			AABB result;
			result.min = (glm::vec3)minT;
			result.max = (glm::vec3)maxT;
			return result;
		}
	};
}