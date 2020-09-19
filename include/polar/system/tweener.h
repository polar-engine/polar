#pragma once

#include <polar/core/id.h>
#include <polar/system/base.h>
#include <unordered_map>
#include <vector>

namespace polar::system {
	template<typename T> class tweener : public base {
	  public:
		typedef std::function<void(core::polar &, const T &)> tween_handler;
		struct tween_desc {
			T from;
			T to;
			double in;
			double pause;
			bool loop;
			tween_handler fn;
			double accumulator = 0;

			tween_desc(T from, T to, double in, double pause, bool loop,
			           tween_handler fn, double acc)
			    : from(from), to(to), in(in), pause(pause), loop(loop), fn(fn),
			      accumulator(acc) {}
		};

	  private:
		std::unordered_map<core::id, tween_desc> tweens;
		core::id nextID = 1;

	  protected:
		void update(DeltaTicks &dt) override {
			std::vector<core::id> toRemove;
			for(auto &tween : tweens) {
				tween.second.accumulator += dt.Seconds();
				if(tween.second.accumulator >
				   tween.second.in + tween.second.pause) {
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
					float alpha = static_cast<float>(
					    (tween.second.accumulator - tween.second.pause) /
					    tween.second.in);
					auto x = tween.second.to * alpha +
					         tween.second.from * (1 - alpha);
					tween.second.fn(engine, x);
				}
			}

			for(auto id : toRemove) { tweens.erase(id); }
		}

	  public:
		static bool supported() { return true; }
		tweener(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "tweener"; }

		inline auto tween(T from, T to, double in, bool loop, tween_handler fn,
		                  double pause, T initial) {
			auto id    = nextID++;
			double acc = (initial - from) / (to - from) * in;
			tweens.emplace(id, tween_desc(from, to, in, pause, loop, fn, acc));
			return core::ref(
			    [this, id]() { tweens.erase(id); });
		}

		inline auto tween(T from, T to, double in, bool loop, tween_handler fn,
		                  double pause = 0.0) {
			return tween(from, to, in, loop, fn, pause, from);
		}
	};
} // namespace polar::system
