#pragma once

#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "System.h"

template<typename T> class Tweener : public System {
public:
	typedef std::function<void(Polar *, const T &)> TweenHandler;
	struct TweenDescriptor {
		T from;
		T to;
		double in;
		bool loop;
		TweenHandler fn;
		double accumulator = 0;

		TweenDescriptor(T from, T to, double in, bool loop, TweenHandler fn, double acc)
			: from(from), to(to), in(in), loop(loop), fn(fn), accumulator(acc) {}
	};
private:
	boost::unordered_map<IDType, TweenDescriptor> tweens;
	IDType nextID = 1;
protected:
	void Update(DeltaTicks &dt) override final {
		boost::container::vector<IDType> toRemove;
		for(auto &tween : tweens) {
			tween.second.accumulator += dt.Seconds();
			if(tween.second.accumulator > tween.second.in) {
				if(tween.second.loop) {
					tween.second.accumulator = 0;
					std::swap(tween.second.from, tween.second.to);
				} else {
					tween.second.accumulator = tween.second.in;
					toRemove.emplace_back(tween.first);
				}
			}

			/* only linear tweening at the moment */
			float alpha = static_cast<float>(tween.second.accumulator / tween.second.in);
			auto x = tween.second.to * alpha + tween.second.from * (1 - alpha);
			tween.second.fn(engine, x);
		}

		for(auto id : toRemove) { tweens.erase(id); }
	}
public:
	static bool IsSupported() { return true; }
	Tweener(Polar *engine) : System(engine) {}

	inline boost::shared_ptr<Destructor> Tween(T from, T to, double in, bool loop, TweenHandler fn) {
		return Tween(from, to, in, loop, fn, from);
	}

	inline boost::shared_ptr<Destructor> Tween(T from, T to, double in, bool loop, TweenHandler fn, T initial) {
		auto id = nextID++;
		double acc = (initial - from) / (to - from) * in;
		tweens.emplace(id, TweenDescriptor(from, to, in, loop, fn, acc));
		return boost::make_shared<Destructor>([this, id] () {
			tweens.erase(id);
		});
	}
};
