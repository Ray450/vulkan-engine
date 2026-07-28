#ifndef AUDIO_UTIL_H
#define AUDIO_UTIL_H


#include <cstdint>  // For int16_t

typedef enum {
    INSTRUMENT_SINE,
    INSTRUMENT_SQUARE,
    INSTRUMENT_TRIANGLE,
    INSTRUMENT_WHITENOISE,
    INSTRUMENT_SAWTOOTH,
    INSTRUMENT_GUITAR,
    INSTRUMENT_PIANO,
    INSTRUMENT_DRUM,
    INSTRUMENT_BELL,
    INSTRUMENT_SITAR,
    INSTRUMENT_KALIMBA,
} InstrumentType;

struct Guitar {
    char str[37];
    InstrumentType instrument[37];

    // handle into the generic audio backend for whichever voice is currently
    // playing this string, or -1 if none
    int activeVoice[37];
};

// Function prototypes
void init_sound();
void guitar_setup(struct Guitar* self);
void guitar_play_string(struct Guitar* self, int stringIndex);
void guitar_stop_sound(struct Guitar* self, int stringIndex);
void guitar_cleanup_finished_sounds(struct Guitar* self);
void guitar_cleanup(struct Guitar* self);

#endif // AUDIO_UTIL_H
