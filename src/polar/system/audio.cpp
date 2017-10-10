#include <polar/system/audio.h>

namespace polar {
namespace system {
	int audio_stream_cb(const void *input, void *output,
	                    unsigned long frameCount,
	                    const PaStreamCallbackTimeInfo *timeInfo,
	                    PaStreamCallbackFlags statusFlags, void *userData) {
		auto audioM = static_cast<audio *>(userData);
		auto buffer = static_cast<int16_t *>(output);
		return audioM->tick(buffer, frameCount);
	}
}
}
