#include "common.h"
#include "AudioManager.h"

int StreamCallback(const void *input,
				   void *output,
				   unsigned long frameCount,
				   const PaStreamCallbackTimeInfo *timeInfo,
				   PaStreamCallbackFlags statusFlags,
				   void *userData) {
	static double frequency = 261.625565;
	static double phase = 0.0;

	auto audioM = static_cast<AudioManager *>(userData);
	auto buffer = static_cast<float *>(output);

	for(unsigned long frame = 0; frame < frameCount; ++frame) {
		auto sample = static_cast<float>(audioM->osc.Tick(audioM->sampleRate));
		buffer[frame * 2 + 0] = sample;
		buffer[frame * 2 + 1] = sample;
	}

	return paContinue;
}
