#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <glm/gtc/noise.hpp>
#include "System.h"
#include "Renderer.h"
#include "Level.h"

#define WORLD_SCALE 100000.0
#define WORLD_DECIMAL(X) (Decimal(X / WORLD_SCALE))

class World : public System {
private:
	uint64_t globalTicks;
	Level oldLevel;
	Level level;
	float alpha = 1;

	void SetUniforms() {
		auto kf = level.GetNow();
		if(alpha < 1) {
			auto oldKf = oldLevel.GetNow();
			kf = kf * alpha + oldKf * (1 - alpha);
		}

		auto renderer = engine->GetSystem<Renderer>().lock();

		size_t iColor = kf.ticks / kf.colorTicks % kf.colors.size();
		auto color = kf.colors[iColor];
		if(glm::length(color) < Decimal(1)) { color = Point3(0.8); }
#
		renderer->SetUniform("u_period",        float(double(globalTicks % (ENGINE_TICKS_PER_SECOND * 100)) / double(ENGINE_TICKS_PER_SECOND * 100)));
		renderer->SetUniform("u_time",          uint32_t(level.ticks));
		renderer->SetUniform("u_threshold",     Threshold());
		renderer->SetUniform("u_waveParams",    Point3(Decimal(kf.ticks) / kf.waveTicks, Decimal(1) / kf.wavePower, kf.waveStrength));
		renderer->SetUniform("u_worldScale",    kf.worldScale / Decimal(WORLD_SCALE));
		renderer->SetUniform("u_color",         color);
	}
protected:
	void Init() override final {
		SetUniforms();
	}

	void Update(DeltaTicks &dt) override final {
		auto deltaTicks = dt.Ticks();
		if(turbo) { deltaTicks *= turboFactor; }

		globalTicks += deltaTicks;
		if(active) {
			level.ticks += deltaTicks;
		}

		if(alpha < 1) {
			alpha = glm::min(Decimal(1.0), alpha + dt.Seconds() * Decimal(8.0));
		}

		SetUniforms();
	}
public:
	bool active;
	bool turbo = false;
	const Decimal turboFactor = Decimal(1.5);

	static bool IsSupported() { return true; }

	World(Polar *engine, std::shared_ptr<Level> level, bool active = true) : System(engine), level(*level), active(active) {}

	uint64_t GetTicks() const { return level.ticks; }

	void SetLevel(std::shared_ptr<Level> lvl) {
		oldLevel = level;
		level = *lvl;
		alpha = 0;
		SetUniforms();
	}

	float Threshold() const {
		auto kf = level.GetNow();

		Decimal s = glm::sin(Decimal(level.ticks) / kf.beatTicks);
		Decimal f = Decimal(1) - glm::pow(glm::abs(s), Decimal(1) / kf.beatPower);
		return kf.baseThreshold + f * kf.beatStrength;
	}

	bool Eval(const Point3 &coord) const {
		auto kf = level.GetNow();
		auto renderer = engine->GetSystem<Renderer>().lock();
		auto voxelFactor = renderer->GetUniformDecimal("u_voxelFactor");

		Point3 finalCoord = coord / (kf.worldScale / Decimal(WORLD_SCALE));

		if(voxelFactor > 1) {
			Decimal factor2 = glm::max(renderer->GetWidth(), renderer->GetHeight()) / glm::pow(voxelFactor + 1, Decimal(1.5));
			finalCoord = glm::floor(finalCoord * factor2) / factor2;
		}

		auto result = glm::simplex(finalCoord) * 0.5 + 0.5;
		return result > Threshold();
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
