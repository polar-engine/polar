#pragma once

#include <atomic>
#if defined(_WIN32)
#include <boost/asio.hpp>
#endif
#include <glm/gtc/noise.hpp>
#include <polar/system/base.h>
#include <polar/system/renderer/base.h>
#include <polar/asset/level.h>
#include <polar/support/level/keyframe.h>

#define WORLD_SCALE 100000.0
#define WORLD_DECIMAL(X) (Decimal(X / WORLD_SCALE))

namespace polar { namespace system {
	class world : public base {
		using keyframe = support::level::keyframe;
	private:
		uint64_t globalTicks;
		polar::asset::level oldLevel;
		polar::asset::level level;
		float alpha = 1;

		void set_uniforms() {
			auto kf = get_kf();
			auto color = get_color(kf);

			auto renderer = engine->getsystem<renderer::base>().lock();

			renderer->setuniform("u_period",        float(double(globalTicks % (ENGINE_TICKS_PER_SECOND * 100)) / double(ENGINE_TICKS_PER_SECOND * 100)));
			renderer->setuniform("u_time",          uint32_t(level.ticks));
			renderer->setuniform("u_threshold",     threshold());
			renderer->setuniform("u_waveParams",    Point3(Decimal(kf.ticks) / glm::max(kf.waveTicks, Decimal(1)), Decimal(1) / glm::max(kf.wavePower, Decimal(1)), kf.waveStrength));
			renderer->setuniform("u_worldScale",    kf.worldScale / Decimal(WORLD_SCALE));
			renderer->setuniform("u_color",         color);
		}

		keyframe get_kf() {
			auto kf = level.get_now();
			if(alpha < 1) {
				auto oldKf = oldLevel.get_now();
				kf = kf * alpha + oldKf * (1 - alpha);
			}
			return kf;
		}

		Point3 get_color() {
			return get_color(get_kf());
		}

		Point3 get_color(keyframe &kf) {
			size_t iColor = 0;
			if(kf.colorTicks > 0) {
				iColor = kf.ticks / kf.colorTicks % kf.colors.size();
			}
			auto color = kf.colors[iColor];
			if(glm::length(color) < Decimal(1)) { color = Point3(Decimal(0.8)); }
			return color;
		}

		Point3 get_color(keyframe &&kf) {
			return get_color(kf);
		}

		void hue() {
			return;
	#if defined(_WIN32)
			auto color = get_color();
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

		void init() override final {
			set_uniforms();
		}

		void update(DeltaTicks &dt) override final {
			auto deltaTicks = dt.Ticks();
			if(turbo) { deltaTicks = uint64_t(deltaTicks * turboFactor); }

			globalTicks += deltaTicks;
			if(active) {
				level.ticks += deltaTicks;
			}

			if(alpha < 1) {
				alpha = glm::min(Decimal(1.0), alpha + dt.Seconds() * Decimal(8.0));
			}

			set_uniforms();

			accumulator += dt;
			if(accumulator.Seconds() > 0.25) {
				hue();
				accumulator = 0;
			}
		}
	public:
		bool active;
		bool turbo = false;
		const Decimal turboFactor = Decimal(1.5);

		static bool supported() { return true; }

		world(core::polar *engine, std::shared_ptr<polar::asset::level> level, bool active = true) : base(engine), level(*level), active(active) {}

		uint64_t get_ticks() const { return level.ticks; }

		void set_level(std::shared_ptr<polar::asset::level> lvl) {
			oldLevel = level;
			level = *lvl;
			alpha = 0;
			set_uniforms();
		}

		float threshold() const {
			auto kf = level.get_now();

			Decimal s = glm::sin(Decimal(level.ticks) / kf.beatTicks);
			Decimal f = Decimal(1) - glm::pow(glm::abs(s), Decimal(1) / kf.beatPower);
			return kf.baseThreshold + f * kf.beatStrength;
		}

		bool eval(const Point3 &coord) const {
			auto kf = level.get_now();
			auto renderer = engine->getsystem<renderer::base>().lock();
			auto voxelFactor = renderer->getuniform_decimal("u_voxelFactor");

			Point3 finalCoord = coord / (kf.worldScale / Decimal(WORLD_SCALE));

			if(voxelFactor > 1) {
				Decimal factor2 = glm::max(renderer->getwidth(), renderer->getheight()) / glm::pow(voxelFactor + 1, Decimal(1.5));
				finalCoord = glm::floor(finalCoord * factor2) / factor2;
			}

			auto result = glm::simplex(finalCoord) * 0.5 + 0.5;
			return result > threshold();
		}

		/* old logo generation matching */
		inline bool match(const Point3 &p) const {
			if(p.z == -25) {
				return true;
			} else if(p.z == -6) {
				if((p.y == -4 && p.x >=  1 && p.x <= 6) ||
				   (p.y == -5 && p.x ==  2) ||
				   (p.y == -3 && p.x ==  5)) { return true; } else { return false; }
			} else { return false; }
		}
	};
} }
