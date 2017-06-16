#pragma once

#include <vector>
#include <unordered_map>
#include "System.h"

template<typename T> class Tweener : public System {
public:
	typedef std::function<void(Polar *, const T &)> TweenHandler;
	struct TweenDescriptor {
		T from;
		T to;
		double in;
		double pause;
		bool loop;
		TweenHandler fn;
		double accumulator = 0;

		TweenDescriptor(T from, T to, double in, double pause, bool loop, TweenHandler fn, double acc)
			: from(from), to(to), in(in), pause(pause), loop(loop), fn(fn), accumulator(acc) {}
	};
private:
	std::unordered_map<IDType, TweenDescriptor> tweens;
	IDType nextID = 1;
protected:
	void Update(DeltaTicks &dt) override final {
		std::vector<IDType> toRemove;
		for(auto &tween : tweens) {
			tween.second.accumulator += dt.Seconds();
			if(tween.second.accumulator > tween.second.in + tween.second.pause) {
				if(tween.second.loop) {
					tween.second.accumulator = 0;
					std::swap(tween.second.from, tween.second.to);
				} else {
					tween.second.accumulator = tween.second.in;
					toRemove.emplace_back(tween.first);
				}
			}

			if(tween.second.accumulator > tween.second.pause) {
				/* only linear tweening at the moment */
				float alpha = static_cast<float>((tween.second.accumulator - tween.second.pause) / tween.second.in);
				auto x = tween.second.to * alpha + tween.second.from * (1 - alpha);
				tween.second.fn(engine, x);
			}
		}

		for(auto id : toRemove) { tweens.erase(id); }
	}
public:
	static bool IsSupported() { return true; }
	Tweener(Polar *engine) : System(engine) {}

	inline std::shared_ptr<Destructor> Tween(T from, T to, double in, bool loop, TweenHandler fn, double pause = 0.0) {
		return Tween(from, to, in, loop, fn, pause, from);
	}

	inline std::shared_ptr<Destructor> Tween(T from, T to, double in, bool loop, TweenHandler fn, double pause, T initial) {
		auto id = nextID++;
		double acc = (initial - from) / (to - from) * in;
		tweens.emplace(id, TweenDescriptor(from, to, in, pause, loop, fn, acc));
		return std::make_shared<Destructor>([this, id] () {
			tweens.erase(id);
		});
	}
};
