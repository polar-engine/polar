#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <glm/gtc/noise.hpp>
#include "System.h"

class World : public System {
private:
	float seed;
protected:
	void Update(DeltaTicks &) override final {}
public:
	static bool IsSupported() { return true; }

	World(Polar *engine, float seed = 0.0f) : System(engine), seed(seed) {}

	inline bool Eval(const Point3 &p) const {
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
