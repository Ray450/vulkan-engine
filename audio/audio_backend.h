#ifndef AUDIO_BACKEND_H
#define AUDIO_BACKEND_H

#include <cstdint>
#include <cstddef>

#define MAX_VOICES 64

// Starts the underlying audio engine. Call once at startup.
void audio_backend_init(void);

// Shuts down the underlying audio engine and frees all voices.
void audio_backend_shutdown(void);

// Plays any block of 16-bit mono PCM sample data at the given sample rate.
// data must remain valid for as long as the sound is playing (it is not copied).
// Returns a voice handle you can pass to audio_stop_sound, or -1 if no free voice
// was available.
int audio_play_sound(const int16_t* data, size_t sampleCount, uint32_t sampleRate, float volume);

// Stops and frees the voice identified by the handle returned from audio_play_sound.
// Safe to call with -1 or a handle that already finished on its own.
void audio_stop_sound(int voiceHandle);

// Call once per frame. Frees any voices that finished playing on their own
// (as opposed to being stopped explicitly).
void audio_cleanup_finished_sounds(void);

// Returns true if the given voice handle is still playing.
// Useful for callers who track their own mapping (e.g. instrument -> voice).
bool audio_is_voice_active(int voiceHandle);

#endif // AUDIO_BACKEND_H
