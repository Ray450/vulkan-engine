#define MA_IMPLEMENTATION
#include "audio_backend.h"
#include "miniaudio/miniaudio.h"
#include <stdio.h>
#include <stdlib.h>

static ma_engine engine;
static int engine_initialized = 0;

struct Voice {
    ma_sound* sound;
    ma_audio_buffer* buffer;
    bool inUse;
};

static Voice voices[MAX_VOICES] = {};

void audio_backend_init(void) {
    if (engine_initialized) return;

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.periodSizeInFrames = 256;

    ma_result result = ma_engine_init(&engineConfig, &engine);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize Miniaudio engine: %d\n", result);
        exit(EXIT_FAILURE);
    }
    engine_initialized = 1;

    result = ma_engine_start(&engine);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to start engine: %d\n", result);
    }
}

void audio_backend_shutdown(void) {
    if (!engine_initialized) return;

    for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].inUse) {
            audio_stop_sound(i);
        }
    }

    ma_engine_uninit(&engine);
    engine_initialized = 0;
}

int audio_play_sound(const int16_t* data, size_t sampleCount, uint32_t sampleRate, float volume) {
    int freeIndex = -1;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!voices[i].inUse) {
            freeIndex = i;
            break;
        }
    }

    if (freeIndex == -1) {
        fprintf(stderr, "audio_play_sound: too many sounds playing\n");
        return -1;
    }

    ma_audio_buffer* buffer = (ma_audio_buffer*)malloc(sizeof(ma_audio_buffer));
    ma_sound* sound = (ma_sound*)malloc(sizeof(ma_sound));

    ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
        ma_format_s16, 1, sampleCount, data, NULL);
    bufferConfig.sampleRate = sampleRate;

    if (ma_audio_buffer_init(&bufferConfig, buffer) != MA_SUCCESS) {
        fprintf(stderr, "audio_play_sound: failed to init buffer\n");
        free(buffer);
        free(sound);
        return -1;
    }

    if (ma_sound_init_from_data_source(&engine, buffer, MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, sound) != MA_SUCCESS) {
        fprintf(stderr, "audio_play_sound: failed to init sound\n");
        ma_audio_buffer_uninit(buffer);
        free(buffer);
        free(sound);
        return -1;
    }

    ma_sound_set_volume(sound, volume);
    ma_sound_start(sound);

    voices[freeIndex].sound = sound;
    voices[freeIndex].buffer = buffer;
    voices[freeIndex].inUse = true;

    return freeIndex;
}

void audio_stop_sound(int voiceHandle) {
    if (voiceHandle < 0 || voiceHandle >= MAX_VOICES || !voices[voiceHandle].inUse) {
        return;
    }

    ma_sound* sound = voices[voiceHandle].sound;
    if (ma_sound_is_playing(sound)) {
        ma_sound_stop(sound);
    }
    ma_sound_uninit(sound);
    free(sound);

    ma_audio_buffer_uninit(voices[voiceHandle].buffer);
    free(voices[voiceHandle].buffer);

    voices[voiceHandle].sound = nullptr;
    voices[voiceHandle].buffer = nullptr;
    voices[voiceHandle].inUse = false;
}

bool audio_is_voice_active(int voiceHandle) {
    if (voiceHandle < 0 || voiceHandle >= MAX_VOICES || !voices[voiceHandle].inUse) {
        return false;
    }
    return ma_sound_is_playing(voices[voiceHandle].sound);
}

void audio_cleanup_finished_sounds(void) {
    for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].inUse && !ma_sound_is_playing(voices[i].sound)) {
            audio_stop_sound(i);
        }
    }
}
