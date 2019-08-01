#pragma once

#include <iostream>
#include <tuple>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

typedef float Decimal;
typedef glm::tvec2<Decimal, glm::highp> EnginePoint2;
typedef glm::tvec3<Decimal, glm::highp> EnginePoint3;
typedef glm::tvec4<Decimal, glm::highp> EnginePoint4;
typedef glm::ivec2 EnginePoint2i;
typedef glm::ivec3 EnginePoint3i;
typedef glm::ivec4 EnginePoint4i;
typedef glm::tquat<Decimal, glm::highp> Quat;
typedef glm::tmat4x4<Decimal, glm::highp> Mat4;
#define Point2 EnginePoint2
#define Point3 EnginePoint3
#define Point4 EnginePoint4
#define Point2i EnginePoint2i
#define Point3i EnginePoint3i
#define Point4i EnginePoint4i
typedef std::tuple<Point3, Point3, Point3> EngineTriangle;
#define Triangle EngineTriangle

enum class GeometryType : uint8_t {
	None,
	Points,
	Lines,
	Triangles,
	TriangleStrip
};

typedef std::uint_fast64_t IDType;
#define INVALID_ID std::numeric_limits<IDType>::max
