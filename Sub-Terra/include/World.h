#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <glm/gtc/noise.hpp>
#include "System.h"

class World : public System {
private:
protected:
	void Update(DeltaTicks &) override final {}
public:
	static bool IsSupported() { return true; }

	World(Polar *engine) : System(engine) {}

	inline bool Eval(const Point3 &p) const {
		float eval = glm::simplex(p / 20.0f) * 0.5f + 0.5f;
		return eval >= 0.7;
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
