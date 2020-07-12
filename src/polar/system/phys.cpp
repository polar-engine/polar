#include <polar/component/phys.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/support/phys/detector/ball.h>
#include <polar/support/phys/detector/box.h>
#include <polar/support/sched/clock/integrator.h>
#include <polar/system/phys.h>
#include <polar/system/sched.h>

namespace polar::system {
	void phys::init() {
		auto sch = engine->get<sched>().lock();
		keep(sch->bind<support::sched::clock::integrator>([this](auto dt) { tick(dt); }));

		add<support::phys::detector::box, support::phys::detector::box>([](core::polar *engine, auto a, auto b) {
			math::point3 originA{0};
			math::point3 originB{0};
			if(auto p = engine->get<component::position>(a.object)) { originA += p->pos.get(); }
			if(auto p = engine->get<component::position>(b.object)) { originB += p->pos.get(); }

			math::point3 scaleA = a.detector->size;
			math::point3 scaleB = b.detector->size;
			if(auto s = engine->get<component::scale>(a.object)) { scaleA *= s->sc.get(); }
			if(auto s = engine->get<component::scale>(b.object)) { scaleB *= s->sc.get(); }

			auto minA = originA - scaleA;
			auto maxA = originA + scaleA;
			auto minB = originB - scaleB;
			auto maxB = originB + scaleB;

			return minA.x <= maxB.x && maxA.x >= minB.x && minA.y <= maxB.y && maxA.y >= minB.y && minA.z <= maxB.z &&
			       maxA.z >= minB.z;
		});

		add<support::phys::detector::ball, support::phys::detector::ball>([](core::polar *engine, auto a, auto b) {
			math::point3 originA{0};
			math::point3 originB{0};
			if(auto p = engine->get<component::position>(a.object)) { originA += p->pos.get(); }
			if(auto p = engine->get<component::position>(b.object)) { originB += p->pos.get(); }

			math::point3 scaleA = a.detector->size;
			math::point3 scaleB = b.detector->size;
			if(auto s = engine->get<component::scale>(a.object)) { scaleA *= s->sc.get(); }
			if(auto s = engine->get<component::scale>(b.object)) { scaleB *= s->sc.get(); }

			return glm::distance(originA, originB) <= scaleA.x + scaleB.x;
		});
	}

	void phys::tick(DeltaTicks dt) {
		auto seconds = dt.Seconds();

		auto range = engine->objects.get<core::tag::ti>().equal_range(typeid(component::phys));
		for(auto it1 = range.first; it1 != range.second; ++it1) {
			auto obj1  = it1->r;
			auto comp1 = it1->ptr.get();
			auto phys1 = static_cast<component::phys *>(comp1);
			for(auto it2 = std::next(it1); it2 != range.second; ++it2) {
				auto obj2  = it2->r;
				auto comp2 = it2->ptr.get();
				if(comp1 != comp2) {
					auto phys2  = static_cast<component::phys *>(comp2);
					auto &det1  = *phys1->detector;
					auto &det2  = *phys2->detector;
					auto pair   = std::make_pair(std::type_index(typeid(det1)), std::type_index(typeid(det2)));
					auto search = resolvers.find(pair);
					if(search != resolvers.cend()) {
						auto b = search->second->operator()(engine, {obj1, phys1->detector}, {obj2, phys2->detector});
						if(b) {
							// log()->info("phys", "collision!");
							for(auto &r : phys1->responders) {
								r->respond(engine, obj1, uint16_t(seconds) * ENGINE_TICKS_PER_SECOND);
							}
							for(auto &r : phys2->responders) {
								r->respond(engine, obj2, uint16_t(seconds) * ENGINE_TICKS_PER_SECOND);
							}
						}
					}
				}
			}
		}
	}
} // namespace polar::system
