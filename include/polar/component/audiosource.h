#pragma once

#include <polar/asset/audio.h>
#include <polar/component/base.h>
#include <polar/support/audio/loop.h>
#include <polar/support/audio/sourcetype.h>

namespace polar {
namespace component {
	class audiosource : public base {
		using loopin     = support::audio::loopin;
		using loopout    = support::audio::loopout;
		using sourcetype = support::audio::sourcetype;

	  protected:
		std::shared_ptr<asset::audio> asset;
		bool looping;
		size_t loopIn;
		size_t loopOut;
		size_t currentSample = 0;

	  public:
		sourcetype type = sourcetype::none;

		audiosource(std::shared_ptr<asset::audio> as, sourcetype type,
		            const bool looping = false)
		    : asset(as), type(type), looping(looping), loopIn(0),
		      loopOut(as->samples.size()) {}
		audiosource(std::shared_ptr<asset::audio> as, sourcetype type,
		            const loopin &in)
		    : asset(as), type(type), looping(true), loopIn(in.value),
		      loopOut(as->samples.size()) {}
		audiosource(std::shared_ptr<asset::audio> as, sourcetype type,
		            const loopout &out)
		    : asset(as), type(type), looping(true), loopIn(0),
		      loopOut(out.value) {}
		audiosource(std::shared_ptr<asset::audio> as, sourcetype type,
		            const loopin &in, const loopout &out)
		    : asset(as), type(type), looping(true), loopIn(in.value),
		      loopOut(out.value) {}

		void advance(double sampleRate, int delta) {
			currentSample += size_t(double(delta) * (44100.0 / sampleRate));
		}

		bool tick(double sampleRate, double volume, int16_t &left,
		          int16_t &right) {
			if(looping) {
				if(currentSample == loopOut ||
				   currentSample >= asset->samples.size()) {
					currentSample = loopIn;
				}
			} else if(currentSample >= asset->samples.size()) {
				return false;
			}

			if(asset->stereo) {
				left += asset->samples[currentSample] * volume;
				right += asset->samples[currentSample + 1] * volume;
				advance(sampleRate, 2);
			} else {
				left += asset->samples[currentSample] * volume;
				right += asset->samples[currentSample] * volume;
				advance(sampleRate, 1);
			}
			return true;
		}
	};
}
}
