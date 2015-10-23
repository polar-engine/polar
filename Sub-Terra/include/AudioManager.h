#pragma once

#include <portaudio.h>
#include "System.h"
#include "Oscillator.h"

int StreamCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);

class AudioManager : public System {
private:
	PaStream *stream;
protected:
	virtual void Init() override {
		Pa_StartStream(stream);
	}

	virtual void Update(DeltaTicks &) override {

	}
public:
	Oscillator osc;
	const double sampleRate = 44100.0f;
	const unsigned long framesPerBuffer = 256;

	static bool IsSupported() { return true; }

	AudioManager(Polar *engine) : System(engine) {
		WaveShape waveShape;
		waveShape.preferredFrequency = 261.625565;
		for(double i = 0.0; i < 64.0; ++i) {
			waveShape.buffer.emplace_back(sin((double)i * 2.0 * 3.14159265358979 / 256.0));
		}
		osc = Oscillator(waveShape);

		Pa_Initialize();
		Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sampleRate, framesPerBuffer, StreamCallback, this);
	}

	~AudioManager() {
		Pa_CloseStream(stream);
		Pa_Terminate();
	}
};
