#pragma once

#include "Utils/Maths.h"

namespace RS
{
	struct AABB
	{
		glm::vec3 min = glm::vec3(0.f);
		glm::vec3 max = glm::vec3(0.f);
	};
}