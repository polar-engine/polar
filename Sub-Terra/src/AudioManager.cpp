#include "common.h"
#include "AudioManager.h"

int StreamCallback(const void *input,
				   void *output,
				   unsigned long frameCount,
				   const PaStreamCallbackTimeInfo *timeInfo,
				   PaStreamCallbackFlags statusFlags,
				   void *userData) {
	auto audioM = static_cast<AudioManager *>(userData);
	auto buffer = static_cast<int16_t *>(output);
	return audioM->Tick(buffer, frameCount);
}
