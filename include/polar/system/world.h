#pragma once

#include <atomic>
#if defined(_WIN32)
#include <boost/asio.hpp>
#endif-
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
		auto kf = GetKF();
		auto color = GetColor(kf);

		auto renderer = engine->GetSystem<Renderer>().lock();

		renderer->SetUniform("u_period",        float(double(globalTicks % (ENGINE_TICKS_PER_SECOND * 100)) / double(ENGINE_TICKS_PER_SECOND * 100)));
		renderer->SetUniform("u_time",          uint32_t(level.ticks));
		renderer->SetUniform("u_threshold",     Threshold());
		renderer->SetUniform("u_waveParams",    Point3(Decimal(kf.ticks) / glm::max(kf.waveTicks, Decimal(1)), Decimal(1) / glm::max(kf.wavePower, Decimal(1)), kf.waveStrength));
		renderer->SetUniform("u_worldScale",    kf.worldScale / Decimal(WORLD_SCALE));
		renderer->SetUniform("u_color",         color);
	}

	Keyframe GetKF() {
		auto kf = level.GetNow();
		if(alpha < 1) {
			auto oldKf = oldLevel.GetNow();
			kf = kf * alpha + oldKf * (1 - alpha);
		}
		return kf;
	}

	Point3 GetColor() {
		return GetColor(GetKF());
	}

	Point3 GetColor(Keyframe &kf) {
		size_t iColor = 0;
		if(kf.colorTicks > 0) {
			iColor = kf.ticks / kf.colorTicks % kf.colors.size();
		}
		auto color = kf.colors[iColor];
		if(glm::length(color) < Decimal(1)) { color = Point3(Decimal(0.8)); }
		return color;
	}

	Point3 GetColor(Keyframe &&kf) {
		return GetColor(kf);
	}

	void Hue() {
		return;
#if defined(_WIN32)
		auto color = GetColor();
		auto min = glm::min(glm::min(color.r, color.g), color.b);
		auto max = glm::max(glm::max(color.r, color.g), color.b);

		Decimal bri = 0, sat = 0, hue = 0;

		if(active) {
			bri = (max + min) / Decimal(2);

			if(bri < 0.5) {
				sat = (max - min) / (max + min);
			} else {
				sat = (max - min) / (2 - max - min);
			}

			if(max == color.r) {
				hue = (color.g - color.b) / (max - min);
			} else if(max == color.g) {
				hue = 2 + (color.b - color.r) / (max - min);
			} else {
				hue = 4 + (color.r - color.g) / (max - min);
			}
			hue *= 60;
			if(hue < 0) { hue += 360; }
			hue /= 360;
		}

		std::ostringstream oss;
		oss << "{\"sat\":" << int(sat * 255) << ",\"bri\":" << int(bri * 255) << ",\"hue\":" << int(hue * 65535) << '}';
		std::string data = oss.str();

		boost::asio::io_service io_service;
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query("192.168.1.168", "http");
		boost::asio::ip::tcp::resolver::iterator endpoint_it = resolver.resolve(query);

		for(auto light : { "7", "8", "9" }) {
			boost::asio::ip::tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_it);

			boost::asio::streambuf request;
			std::ostream request_s(&request);
			request_s << "PUT /api/PaugQhX2KUHA6krplR06PYFk5k8PMBqlkZtmBT4k/lights/" << light << "/state HTTP/1.1\r\n";
			request_s << "Connection: close\r\n";
			request_s << "Content-Length: " << data.size() << "\r\n";
			request_s << "Content-Type: text/plain\r\n";
			request_s << "\r\n";
			request_s << data;

			boost::asio::write(socket, request);
		}

		/*boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		std::istream response_s(&response);
		std::string http_version;
		response_s >> http_version;
		unsigned int status_code;
		response_s >> status_code;
		std::string status_msg;
		std::getline(response_s, status_msg);

		DebugManager()->Debug(http_version, ' ', status_code, ' ', status_msg);

		boost::asio::read_until(socket, response, "\r\n\r\n");

		std::string header;
		while(std::getline(response_s, header) && header != "\r") {
			DebugManager()->Debug(header);
		}

		boost::system::error_code err;
		while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), err)) {
			DebugManager()->Debug(&response);
		}*/
#endif
	}
protected:
	DeltaTicks accumulator;

	void Init() override final {
		SetUniforms();
	}

	void Update(DeltaTicks &dt) override final {
		auto deltaTicks = dt.Ticks();
		if(turbo) { deltaTicks = uint64_t(deltaTicks * turboFactor); }

		globalTicks += deltaTicks;
		if(active) {
			level.ticks += deltaTicks;
		}

		if(alpha < 1) {
			alpha = glm::min(Decimal(1.0), alpha + dt.Seconds() * Decimal(8.0));
		}

		SetUniforms();

		accumulator += dt;
		if(accumulator.Seconds() > 0.25) {
			Hue();
			accumulator = 0;
		}
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
