#pragma once

#include <polar/system/base.h>
#include <polar/system/world.h>
#include <polar/component/position.h>
#include <polar/component/orientation.h>
#include <polar/component/playercamera.h>

namespace polar { namespace system { namespace player {
	class title : public base {
	private:
		IDType object;
		Point2 orientVel;
		Decimal accum;
		const Decimal timestep = Decimal(0.02);
	protected:
		inline void init() override {
			engine->addcomponent<component::playercamera>(object);
		}

		inline void update(DeltaTicks &dt) override {
			accum += dt.Seconds();

			auto pos    = engine->getcomponent<component::position>(object);
			auto orient = engine->getcomponent<component::orientation>(object);
			auto camera = engine->getcomponent<component::playercamera>(object);
			auto wld = engine->getsystem<world>().lock();

			if(accum > timestep * 10) { accum = timestep * 10; }
			while(accum >= timestep) {
				accum -= timestep;

				orientVel *= static_cast<Decimal>(glm::pow(0.999, timestep * 1000.0));

				unsigned int i = 0;
				auto average = Point2(0);
				if(wld) {
					for(Decimal x = -fieldOfView; x < fieldOfView; x += 0.5f) {
						for(Decimal y = -fieldOfView; y < fieldOfView; y += 0.5f) {
							for(Decimal d = 1; d < viewDistance; d += 0.5f) {
								auto p = Point4(x, y, -d, 1);
								auto abs = glm::inverse(orient->orient) * glm::inverse(camera->orientation) * p;
								auto eval = wld->eval(pos->pos.get() + Point3(abs) / Decimal(WORLD_SCALE));
								if(eval) {
									average.x += Decimal(0.0005) * ((y >= 0) ? 1 : -1) / (glm::max(Decimal(1), d - 2) * 2 / viewDistance);
									average.y += Decimal(0.0005) * ((x <= 0) ? 1 : -1) / (glm::max(Decimal(1), d - 2) * 2 / viewDistance);
									++i;
									break;
								}
							}
						}
					}
				}

				if(i > 0) {
					average /= static_cast<Decimal>(i);
					if(average.length() < 0.1f && average.length() >= 0) { average = Point2(0,  1); }
					if(average.length() > 0.1f && average.length() <= 0) { average = Point2(0, -1); }
					orientVel += average;
				}
			}

			orient->orient = Quat(Point3(orientVel.x, 0, 0)) * Quat(Point3(0, orientVel.y, 0)) * orient->orient;

			const auto forward = glm::normalize(Point4(0, 0, -1, 1));
			auto abs = (glm::inverse(orient->orient) * glm::inverse(camera->orientation) * forward) * velocity;

			pos->pos.derivative()->x = abs.x;
			pos->pos.derivative()->y = abs.y;
			pos->pos.derivative()->z = abs.z;
		}
	public:
		const Decimal fieldOfView = 2;
		const Decimal viewDistance = 10.0f;
		const Decimal velocity = WORLD_DECIMAL(5.0);

		static bool supported() { return true; }
		title(core::polar *engine, const IDType object) : base(engine), object(object) {}
	};
} } }
