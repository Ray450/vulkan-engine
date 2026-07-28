#include "audio_util.h"
#include "audio_backend.h"
#include "sound_generator.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../graphics/logger.h"

#define CONCERT_A 220.0
#define SAMPLES_PER_SEC 44100
#define DURATION 16  // Duration in seconds

static int16_t* samples[37] = {0};

void init_sound(void) {
    audio_backend_init();
}

void guitar_setup(struct Guitar* self) {
    memset(samples, 0, sizeof(samples));
    memset(self->str, '0', sizeof(self->str));
    for (int i = 0; i < 37; i++) self->activeVoice[i] = -1;

    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    if (!queue) {
        fprintf(stderr, "Failed to allocate queue\n");
        return;
    }

    for (int i = 0; i < 37; i++) {
        self->str[i] = '0';
        self->instrument[i] = INSTRUMENT_GUITAR;
        double freq = CONCERT_A * pow(2.0, (i - 24) / 12.0);

        // Only initialize queue for string-based instruments
        if (self->instrument[i] >= INSTRUMENT_GUITAR) {
            StringSound_StringSound(queue, freq);
        }

        samples[i] = makeSamples(queue, freq, self->instrument[i]);
        if (!samples[i]) {
            fprintf(stderr, "Failed to generate samples for string %d\n", i);
            continue;
        }
    }
    free(queue);
}

void guitar_play_string(struct Guitar* self, int stringIndex) {
    int16_t* sample = samples[stringIndex];
    if (!sample) {
        fprintf(stderr, "guitar_play_string: no samples generated for string %d\n", stringIndex);
        return;
    }

    int handle = audio_play_sound(sample, SAMPLES_PER_SEC * DURATION, SAMPLES_PER_SEC, 6000.0f / 32768.0f);
    if (handle == -1) {
        return;
    }

    self->activeVoice[stringIndex] = handle;
}

void guitar_stop_sound(struct Guitar* self, int stringIndex) {
    logMessage(LOG_LEVEL_INFO, "Calling stop function");

    int handle = self->activeVoice[stringIndex];
    if (handle == -1) {
        logMessage(LOG_LEVEL_WARNING, "No active sound found for string %d", stringIndex);
        return;
    }

    logMessage(LOG_LEVEL_INFO, "Stopping sound for string %d", stringIndex);
    audio_stop_sound(handle);
    self->activeVoice[stringIndex] = -1;
}

void guitar_cleanup_finished_sounds(struct Guitar* self) {
    audio_cleanup_finished_sounds();

    for (int i = 0; i < 37; i++) {
        if (self->activeVoice[i] != -1 && !audio_is_voice_active(self->activeVoice[i])) {
            self->activeVoice[i] = -1;
            printf("String %d: sound finished and cleaned up\n", i);
        }
    }
}

void guitar_cleanup(struct Guitar* self) {
    audio_backend_shutdown();

    for (int i = 0; i < 37; i++) {
        if (samples[i]) {
            free(samples[i]);
            samples[i] = NULL;
        }
    }
}
