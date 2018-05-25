/* 
 * soudpipeal.h
 * Play Soundpipe through the OpenAL API.
 */

#pragma once

extern "C" {
#include "soundpipe.h"
}

#include <AL/al.h>
#include <AL/alc.h>
#include <assert.h>
#include <stdint.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable: 4996)
#define SLEEP Sleep
#else
#ifndef __EMSCRIPTEN__
#include <unistd.h>
#define SLEEP usleep
#endif
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define NUM_BUFFERS 2

class OpenALState
{
public:
	ALCdevice* device;
	ALCcontext* context;
	ALuint source;
	int16_t *loadbuf;
	unsigned samplerate;
	unsigned bufsize;
	bool started;
};

class SoundpipeALState
{
public:
	OpenALState *al;
	void *userData;
	sp_data *sp;
	void(*callback)(sp_data *, void *);
};

void checkALError()
{
	ALenum err = alGetError();
	assert(alGetError() == AL_NO_ERROR);
}

void fillALBuffer(SoundpipeALState *spal, ALuint buffer) {
	for (unsigned i = 0; i < spal->al->bufsize; ++i) {
		spal->callback(spal->sp, spal->userData);
		spal->al->loadbuf[i] = spal->sp->out[0] * 32760.f;
	}
	alBufferData(buffer, AL_FORMAT_MONO16, spal->al->loadbuf, sizeof(int16_t) * spal->al->bufsize, spal->al->samplerate);
}

SoundpipeALState *soundpipeal_init(sp_data *sp, void *ud, void(*callback)(sp_data *, void *))
{
	SoundpipeALState *spal = new SoundpipeALState();
	spal->sp = sp;
	spal->userData = ud;
	spal->callback = callback;

	OpenALState *al = new OpenALState();
	spal->al = al;
	al->bufsize = 4000;
	al->samplerate = 44100;
	al->loadbuf = new int16_t[al->bufsize]; // stereo: *2
	al->device = alcOpenDevice(NULL);
	al->context = alcCreateContext(al->device, NULL);
	al->started = false;
	alcMakeContextCurrent(al->context);

	ALuint buffers[NUM_BUFFERS];
	alGenBuffers(NUM_BUFFERS, buffers);
	for (int i = 0; i < NUM_BUFFERS; ++i) {
		fillALBuffer(spal, buffers[i]);
	}

	alGenSources(1, &al->source);
	alSourceQueueBuffers(al->source, NUM_BUFFERS, buffers);
	return spal;
}

void soundpipeal_iter(void *emdata)
{
	SoundpipeALState *spal = (SoundpipeALState*)emdata;
	OpenALState *al = spal->al;
	checkALError();
	if (!al->started) {
		alSourcePlay(al->source);
		al->started = true;
		return;
	}
	ALint val;
	alGetSourcei(al->source, AL_BUFFERS_PROCESSED, &val);
	if (val == 0) { return; }
	ALuint unqueued[NUM_BUFFERS];
	alSourceUnqueueBuffers(al->source, val, unqueued);
	for (int i = 0; i < val; ++i) {
		fillALBuffer(spal, unqueued[i]);
	}
	alSourceQueueBuffers(al->source, val, unqueued);
	ALint sourceState;
	alGetSourcei(al->source, AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PLAYING) {
		printf("resuming dropout\n");
		alSourcePlay(al->source);
	}
}


#ifdef __EMSCRIPTEN__
void soundpipeal_start(SoundpipeALState *spal)
{
	emscripten_set_main_loop_arg(soundpipeal_iter, spal, 0, 0);
}
void soundpipeal_stop()
{
	emscripten_cancel_main_loop();
}
#else

void soundpipeal_play(SoundpipeALState *spal)
{
	while (true) {
		soundpipeal_iter(spal);
		// prevent freezing if OpenAL functions decide to return immediately:
		SLEEP(1);
	}
	alcDestroyContext(spal->al->context);
	alcCloseDevice(spal->al->device);
}
#endif




