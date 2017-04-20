#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <glm/gtc/noise.hpp>
#include "System.h"
#include "Renderer.h"
#include "Level.h"

class World : public System {
private:
	Level level;
	void SetUniforms() {
		auto kf = level.GetNow();
		auto renderer = engine->GetSystem<Renderer>().lock();
		renderer->SetUniform("u_baseThreshold", kf.baseThreshold);
		renderer->SetUniform("u_beatTicks",     kf.beatTicks);
		renderer->SetUniform("u_beatPower",     kf.beatPower);
		renderer->SetUniform("u_beatStrength",  kf.beatStrength);
		renderer->SetUniform("u_waveTicks",     kf.waveTicks);
		renderer->SetUniform("u_wavePower",     kf.wavePower);
		renderer->SetUniform("u_waveStrength",  kf.waveStrength);
		renderer->SetUniform("u_worldScale",    kf.worldScale);
		size_t iColor = kf.ticks / kf.colorTicks % kf.colors.size();
		renderer->SetUniform("u_color",         kf.colors[iColor]);
	}
protected:
	void Init() override final {
		SetUniforms();
	}

	void Update(DeltaTicks &dt) override final {
		auto renderer = engine->GetSystem<Renderer>().lock();
		renderer->SetUniform("u_time", (uint32_t)level.ticks);

		if(active) {
			level.ticks += dt.Ticks();
			SetUniforms();
		}
	}
public:
	bool active;

	static bool IsSupported() { return true; }

	World(Polar *engine, std::shared_ptr<Level> level, bool active = true) : System(engine), level(*level), active(active) {}

	uint64_t GetTicks() const { return level.ticks; }

	float Threshold() {
		auto kf = level.GetNow();

		float s = glm::sin(Decimal(level.ticks) / kf.beatTicks);
		float f = 1.0 - glm::pow(glm::abs(s), Decimal(1.0) / kf.beatPower);
		return kf.baseThreshold + f * kf.beatStrength;
	}

	bool Eval(Point3 coord) {
		auto kf = level.GetNow();
		auto renderer = engine->GetSystem<Renderer>().lock();
		auto voxelFactor = renderer->GetUniformDecimal("u_voxelFactor");

		Point3 finalCoord = coord / kf.worldScale;

		if(voxelFactor > 1) {
			float factor2 = glm::max(renderer->GetWidth(), renderer->GetHeight()) / glm::pow(voxelFactor + 1, Decimal(1.5));
			finalCoord = glm::floor(finalCoord * factor2) / factor2;
		}

		auto result = glm::simplex(finalCoord) * 0.5 + 0.5;
		return result > Threshold();
	}

	inline bool Eval(const Point3 &p) const {
		Decimal eval = glm::simplex(p / Decimal(20)) * Decimal(0.5) + Decimal(0.5);
		return eval >= Decimal(0.7);
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
