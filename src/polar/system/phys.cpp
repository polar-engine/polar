#include <polar/component/phys.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/support/phys/detector/box.h>
#include <polar/system/event.h>
#include <polar/system/phys.h>

namespace polar::system {
	void phys::init() {
		using arg_t = support::event::arg;
		auto eventM = engine->get<system::event>().lock();
		auto f      = [this](arg_t arg) {
            auto seconds = arg.float_;
            (void)seconds;

            auto range =
                engine->objects.right.equal_range(&typeid(component::phys));
            for(auto it1 = range.first; it1 != range.second; ++it1) {
                auto id1   = it1->get_left();
                auto comp1 = it1->info.get();
                auto phys1 = static_cast<component::phys *>(comp1);
                for(auto it2 = std::next(it1); it2 != range.second; ++it2) {
                    auto id2   = it2->get_left();
                    auto comp2 = it2->info.get();
                    if(comp1 != comp2) {
                        auto phys2 = static_cast<component::phys *>(comp2);
                        auto &det1 = *phys1->detector;
                        auto &det2 = *phys2->detector;
                        auto pair =
                            std::make_pair(&typeid(det1), &typeid(det2));
                        auto search = resolvers.find(pair);
                        if(search != resolvers.cend()) {
                            auto b = search->second->operator()(
                                engine, {id1, phys1->detector},
                                {id2, phys2->detector});
                            if(b) { debugmanager()->info("collision!"); }
                        }
                    }
                }
            }
		};
		dtors.emplace_back(eventM->listenfor("integrator", "ticked", f));

		add<support::phys::detector::box, support::phys::detector::box>(
		    [](core::polar *engine, auto a, auto b) {
			    Point3 originA, originB;
			    if(auto p = engine->get<component::position>(a.id)) {
				    originA += p->pos.get();
			    }
			    if(auto p = engine->get<component::position>(b.id)) {
				    originB += p->pos.get();
			    }

			    Point3 scaleA = a.detector->size;
			    Point3 scaleB = b.detector->size;
			    if(auto s = engine->get<component::scale>(a.id)) {
				    scaleA *= s->sc.get();
			    }
			    if(auto s = engine->get<component::scale>(b.id)) {
				    scaleB *= s->sc.get();
			    }

			    auto minA = originA - scaleA;
			    auto maxA = originA + scaleA;
			    auto minB = originB - scaleB;
			    auto maxB = originB + scaleB;

			    return minA.x <= maxB.x && maxA.x >= minB.x &&
			           minA.y <= maxB.y && maxA.y >= minB.y &&
			           minA.z <= maxB.z && maxA.z >= minB.z;
		    });
	}
} // namespace polar::system
