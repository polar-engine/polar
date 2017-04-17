#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <glm/gtc/noise.hpp>
#include "System.h"
#include "Renderer.h"

class World : public System {
private:
protected:
	void Update(DeltaTicks &) override final {}
public:
	static bool IsSupported() { return true; }

	World(Polar *engine) : System(engine) {}

	float Threshold() {
		auto renderer = engine->GetSystem<Renderer>().lock();

		float s = glm::sin(Decimal(renderer->time) / renderer->GetUniformDecimal("u_beatTicks"));
		float f = 1.0 - glm::pow(glm::abs(s), Decimal(1.0) / renderer->GetUniformDecimal("u_beatPower"));
		return renderer->GetUniformDecimal("u_baseThreshold") + f * renderer->GetUniformDecimal("u_beatStrength");
	}

	bool Eval(Point3 coord) {
		auto renderer = engine->GetSystem<Renderer>().lock();

		Point3 finalCoord = coord / renderer->GetUniformPoint3("u_worldScale");

		if(renderer->GetUniformDecimal("u_voxelFactor") > 1) {
			float factor2 = glm::max(renderer->GetWidth(), renderer->GetHeight()) / glm::pow(renderer->GetUniformDecimal("u_voxelFactor") + 1, Decimal(1.5));
			finalCoord = glm::floor(finalCoord * factor2) / factor2;
		}

		auto result = glm::simplex(finalCoord) * 0.5 + 0.5;
		return result > Threshold();
	}

	inline bool Eval(const Point3 &p) const {
		Decimal eval = glm::simplex(p / Decimal(20)) * Decimal(0.5) + Decimal(0.5);
		return eval >= Decimal(0.7);
		return false;
	}

	/* old logo generation matching */
	inline bool Match(const Point3 &p) const {
		if(p.z == -25) {
			return true;
		} else if(p.z == -6) {
			if((p.y == -4 && p.x >=  1 && p.x <= 6) ||
			   (p.y == -5 && p.x ==  2) ||
			   (p.y == -3 && p.x ==  5)) { return true; } else { return false; }
		} else { return false; }
	}
};
