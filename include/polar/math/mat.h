#pragma once

#include <polar/math/number.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

namespace polar::math {
	using mat4x4 = glm::tmat4x4<decimal, glm::highp>;

	inline mat4x4 perspective(point2 size, decimal near, decimal far) {
		const decimal pixel_distance_from_screen = 1000;
		const decimal fov_plus                   = 10;

		auto fovy = 2.0f * glm::atan(size.y, 2.0f * pixel_distance_from_screen) + fov_plus;
		return glm::perspective(fovy, size.x / size.y, near, far);
	}
} // namespace polar::math
