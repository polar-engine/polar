#include "common.h"
#include "AudioManager.h"

int StreamCallback(const void *input,
				   void *output,
				   unsigned long frameCount,
				   const PaStreamCallbackTimeInfo *timeInfo,
				   PaStreamCallbackFlags statusFlags,
				   void *userData) {
	auto audioM = static_cast<AudioManager *>(userData);
	auto buffer = static_cast<uint16_t *>(output);

	for(unsigned long frame = 0; frame < frameCount; ++frame) {
		auto sample = audioM->osc.Tick(audioM->sampleRate);
		buffer[frame * 2 + 0] = sample;
		buffer[frame * 2 + 1] = sample;
	}

	return paContinue;
}
