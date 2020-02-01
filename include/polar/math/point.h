#pragma once

#include <polar/math/number.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

namespace polar::math {
	using point2 = glm::tvec2<decimal, glm::highp>;
	using point3 = glm::tvec3<decimal, glm::highp>;
	using point4 = glm::tvec4<decimal, glm::highp>;

	using point2i = glm::ivec2;
	using point3i = glm::ivec3;
	using point4i = glm::ivec4;
} // namespace polar::math
