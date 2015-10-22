#include "common.h"
#include "AudioManager.h"

/*
const void *input, void *output,
unsigned long frameCount,
const PaStreamCallbackTimeInfo* timeInfo,
PaStreamCallbackFlags statusFlags,
void *userData );
*/

int StreamCallback(const void *input,
				   void *output,
				   unsigned long frameCount,
				   const PaStreamCallbackTimeInfo *timeInfo,
				   PaStreamCallbackFlags statusFlags,
				   void *userData) {
	return paContinue;
}
