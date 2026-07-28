#ifndef MIDI_H
#define MIDI_H

#include "audio_util.h"
#include "../graphics/logger.h"

// Map keys 0-9 to string indices (C4 to E5)
extern const int note_strings[];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern uint8_t midi_data2[];


extern uint8_t midi_data[];


#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define NUM_STRINGS 37
#define MAX_ACTIVE_NOTES 10

// extern int16_t* samples[NUM_STRINGS];
// extern void Guitar_playString(struct Guitar* guitar, int stringIndex);
// extern void Guitar_cleanup_finished_sounds(struct Guitar* guitar);

// static const int note_strings[] = {
//    0, 1, 2, 3, 4, 5, 6, 7, 8, 9
//};

// Active note tracking
typedef struct {
    uint8_t midi_note;
    int string_index;
    bool active;
} ActiveNote;

// static ActiveNote active_notes[MAX_ACTIVE_NOTES];
extern size_t get_midi_data_size(void);

// Convert MIDI note to string index with full chromatic support
static int midi_note_to_string_index(uint8_t midi_note);

// Find active note slot
static int find_active_note(uint8_t midi_note);

// Add active note
static int add_active_note(uint8_t midi_note, int string_index);

// Remove active note
static void remove_active_note(uint8_t midi_note);

// Improved delay function with error handling
static void delay_seconds(float seconds);

// Parse variable-length quantity (delta time)
static uint32_t parse_vlq(const uint8_t* data, size_t* pos, size_t max_len);

// Main MIDI processing function with improved error handling and logging
void process_midi(struct Guitar* guitar, const uint8_t* midi_data, size_t midi_data_len);


#endif  // MIDI_H