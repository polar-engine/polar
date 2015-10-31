#pragma once

#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "System.h"

struct InDescriptor { double seconds; };
inline InDescriptor In(double x) { return InDescriptor{x}; }

template<typename T> class Tweener : public System {
public:
	struct FromDescriptor { T value; };
	struct ToDescriptor { T value; };

	typedef std::function<void(Polar *, const T &)> TweenHandler;
	struct TweenDescriptor {
		T from;
		T to;
		double in;
		TweenHandler fn;
		double accumulator = 0;

		TweenDescriptor(T from, T to, double in, TweenHandler fn) : from(from), to(to), in(in), fn(fn) {}
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
				tween.second.accumulator = tween.second.in;
				toRemove.emplace_back(tween.first);
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

	inline void Tween(const FromDescriptor from, const ToDescriptor to, const InDescriptor in, const TweenHandler &fn) {
		tweens.emplace(nextID++, TweenDescriptor(from.value, to.value, in.seconds, fn));
	}

	inline void Tween(const ToDescriptor to, const InDescriptor in, const TweenHandler &fn) {
		Tween(From<T>(0), to, in, fn);
	}
};

template<typename T> inline typename Tweener<T>::FromDescriptor From(T x) { return Tweener<T>::FromDescriptor{x}; }
template<typename T> inline typename Tweener<T>::ToDescriptor To(T x) { return Tweener<T>::ToDescriptor{x}; }
