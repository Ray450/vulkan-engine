#include "midi.h"

// Map keys 0-9 to string indices (C4 to E5)
const int note_strings[] = {
    0,  // C4  (MIDI 60)
    1,  // C#4 (MIDI 61) 
    2,  // D4  (MIDI 62)
    3,  // D#4 (MIDI 63)
    4,  // E4  (MIDI 64)
    5,  // F4  (MIDI 65)
    6,  // F#4 (MIDI 66)
    7,  // G4  (MIDI 67)
    8,  // G#4 (MIDI 68)
    9,  // A4  (MIDI 69)
    10, // A#4 (MIDI 70)
    11, // B4  (MIDI 71)
    12, // C5  (MIDI 72)
    13, // C#5 (MIDI 73)
    14, // D5  (MIDI 74)
    15, // D#5 (MIDI 75)
    16  // E5  (MIDI 76)
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t midi_data2[] = {
    0x00,                   // Delta time 0
    0xC0, 0x00,             // Program Change to Acoustic Piano (patch 0)

    0x00,                   // Delta time 0
    0x90, 0x3C, 0x64,       // **Note On: C4 (0x3C), velocity 100 (0x64)**

    0x00,                   // Delta time 0
    0xB0, 0x40, 0x7F,       // Sustain Pedal ON (CC 64, value 127)

    0x82, 0x68,             // Delta time 360 ticks (variable-length quantity)
    0x90, 0x40, 0x60,       // **Note On: E4 (0x40), velocity 96 (0x60)**

    0x81, 0x78,             // Delta time 120 ticks
    0x80, 0x3C, 0x40,       // **Note Off: C4 (0x3C), velocity 64 (0x40)**

    0x81, 0x78,             // Delta time 120 ticks
    0x80, 0x40, 0x40,       // **Note Off: E4 (0x40), velocity 64 (0x40)**

    0x00,                   // Delta time 0
    0xB0, 0x40, 0x00,       // Sustain Pedal OFF (CC 64, value 0)

    0x00,                   // Delta time 0
    0xFF, 0x2F, 0x00        // End of Track meta event
};


uint8_t midi_data[] = {
    0x00, 0xC0, 0x00,                   // Delta time 0, Program Change to Acoustic Piano (patch 0)

    // C4 (MIDI 60)
    0x00, 0x90, 0x3C, 0x64,             // Delta time 0, Note On: C4, velocity 100
    0x81, 0x60, 0x80, 0x3C, 0x40,       // Delta time 480, Note Off: C4, velocity 64

    // D4 (MIDI 62)
    0x00, 0x90, 0x3E, 0x64,             // Delta time 0, Note On: D4, velocity 100
    0x81, 0x60, 0x80, 0x3E, 0x40,       // Delta time 480, Note Off: D4, velocity 64

    // E4 (MIDI 64)
    0x00, 0x90, 0x40, 0x64,             // Delta time 0, Note On: E4, velocity 100
    0x81, 0x60, 0x80, 0x40, 0x40,       // Delta time 480, Note Off: E4, velocity 64

    // F4 (MIDI 65)
    0x00, 0x90, 0x41, 0x64,             // Delta time 0, Note On: F4, velocity 100
    0x81, 0x60, 0x80, 0x41, 0x40,       // Delta time 480, Note Off: F4, velocity 64

    // G4 (MIDI 67)
    0x00, 0x90, 0x43, 0x64,             // Delta time 0, Note On: G4, velocity 100
    0x81, 0x60, 0x80, 0x43, 0x40,       // Delta time 480, Note Off: G4, velocity 64

    // A4 (MIDI 69)
    0x00, 0x90, 0x45, 0x64,             // Delta time 0, Note On: A4, velocity 100
    0x81, 0x60, 0x80, 0x45, 0x40,       // Delta time 480, Note Off: A4, velocity 64

    // B4 (MIDI 71)
    0x00, 0x90, 0x47, 0x64,             // Delta time 0, Note On: B4, velocity 100
    0x81, 0x60, 0x80, 0x47, 0x40,       // Delta time 480, Note Off: B4, velocity 64

    // C5 (MIDI 72)
    0x00, 0x90, 0x48, 0x64,             // Delta time 0, Note On: C5, velocity 100
    0x81, 0x60, 0x80, 0x48, 0x40,       // Delta time 480, Note Off: C5, velocity 64

    // D5 (MIDI 74)
    0x00, 0x90, 0x4A, 0x64,             // Delta time 0, Note On: D5, velocity 100
    0x81, 0x60, 0x80, 0x4A, 0x40,       // Delta time 480, Note Off: D5, velocity 64

    // E5 (MIDI 76)
    0x00, 0x90, 0x4C, 0x64,             // Delta time 0, Note On: E5, velocity 100
    0x81, 0x60, 0x80, 0x4C, 0x40,       // Delta time 480, Note Off: E5, velocity 64

    0x00, 0xFF, 0x2F, 0x00              // Delta time 0, End of Track
};



#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define NUM_STRINGS 37
#define MAX_ACTIVE_NOTES 10

// extern int16_t* samples[NUM_STRINGS];
// extern void guitar_play_string(struct Guitar* guitar, int stringIndex);
// extern void guitar_cleanup_finished_sounds(struct Guitar* guitar);

// static const int note_strings[] = {
//    0, 1, 2, 3, 4, 5, 6, 7, 8, 9
//};

// Active note tracking
/*typedef struct {
    uint8_t midi_note;
    int string_index;
    bool active;
} ActiveNote;*/

static ActiveNote active_notes[MAX_ACTIVE_NOTES];

size_t get_midi_data_size(void) {
    return sizeof(midi_data);
}

// Convert MIDI note to string index with full chromatic support
static int midi_note_to_string_index(uint8_t midi_note) {
    if (midi_note < 60 || midi_note > 76) {
        return -1; // Out of range
    }
    return note_strings[midi_note - 60];
}

/*

static int midi_note_to_string_index(uint8_t midi_note) {
    // Fixed: Map MIDI note 33 (A1) to string 0, up to note 69 (A4) to string 36
    int string_index = midi_note - 33;
    if (string_index < 0 || string_index >= NUM_STRINGS) {
        logMessage(LOG_LEVEL_WARNING, "MIDI note %d out of range (33–69)", midi_note);
        return -1;
    }
    return string_index;
}

*/

// Find active note slot
static int find_active_note(uint8_t midi_note) {
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        if (active_notes[i].active && active_notes[i].midi_note == midi_note) {
            return i;
        }
    }
    return -1;
}

// Add active note
static int add_active_note(uint8_t midi_note, int string_index) {
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        if (!active_notes[i].active) {
            active_notes[i].midi_note = midi_note;
            active_notes[i].string_index = string_index;
            active_notes[i].active = true;
            return i;
        }
    }
    return -1; // No free slots
}

// Remove active note
static void remove_active_note(uint8_t midi_note) {
    int idx = find_active_note(midi_note);
    if (idx >= 0) {
        active_notes[idx].active = false;
    }
}

// Improved delay function with error handling
static void delay_seconds(float seconds) {
    if (seconds <= 0.0f) return;
    
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - ts.tv_sec) * 1e9);
    
    struct timespec remaining;
    while (nanosleep(&ts, &remaining) == -1) {
        ts = remaining; // Continue with remaining time if interrupted
    }
}

// Parse variable-length quantity (delta time)
static uint32_t parse_vlq(const uint8_t* data, size_t* pos, size_t max_len) {
    uint32_t value = 0;
    uint8_t byte;
    
    do {
        if (*pos >= max_len) {
            fprintf(stderr, "VLQ parsing error: reached end of data\n");
            return 0;
        }
        
        byte = data[(*pos)++];
        value = (value << 7) | (byte & 0x7F);
        
        // Prevent overflow
        if (value > 0x0FFFFFFF) {
            fprintf(stderr, "VLQ overflow detected\n");
            return 0;
        }
        
    } while (byte & 0x80);
    
    return value;
}

// Main MIDI processing function with improved error handling and logging
void process_midi(struct Guitar* guitar, const uint8_t* midi_data, size_t midi_data_len) {
    if (!guitar || !midi_data || midi_data_len == 0) {
        logMessage(LOG_LEVEL_ERROR, "Invalid parameters for MIDI processing");
        return;
    }
    
    size_t pos = 0;
    float seconds_per_tick = 0.00104167f; // 120 BPM, 480 PPQN
    
    // Initialize active notes tracking
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        active_notes[i].active = false;
    }
    
    logMessage(LOG_LEVEL_INFO, "Starting MIDI playback (data length: %zu bytes)", midi_data_len);
    
    while (pos < midi_data_len) {
        // Parse delta time using VLQ
        uint32_t delta_time = parse_vlq(midi_data, &pos, midi_data_len);
        
        if (pos >= midi_data_len) {
            logMessage(LOG_LEVEL_ERROR, "No MIDI event after delta time at position %zu", pos);
            break;
        }
        
        uint8_t status = midi_data[pos++];
        
        // Debug timing information
        /*if (delta_time > 0) {
            float delay = delta_time * seconds_per_tick;
            float min_note_duration = 0.5f; // Minimum 0.5 seconds
            if (delay < min_note_duration) delay = min_note_duration;
            logMessage(LOG_LEVEL_DEBUG, "Delta time: %u ticks (%.3f seconds)", delta_time, delay);
            delay_seconds(delay);
        }*/

        // Apply MIDI delta time delay
        if (delta_time > 0) {
            float delay = delta_time * seconds_per_tick;
            logMessage(LOG_LEVEL_DEBUG, "Delta time: %u ticks (%.3f seconds)", delta_time, delay);
            delay_seconds(delay);
        }

        
        
        switch (status & 0xF0) {
            case 0x80: { // Note Off
                if (pos + 1 >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Note Off event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t note = midi_data[pos++];
                uint8_t velocity = midi_data[pos++];
                
                int string_index = midi_note_to_string_index(note);
                if (string_index >= 0 && string_index < NUM_STRINGS) {
                    logMessage(LOG_LEVEL_INFO, "Note Off: %d -> string %d (velocity %d)", 
                              note, string_index, velocity);
                    guitar_stop_sound(guitar, string_index); // Stop the sound
                    remove_active_note(note);
                } else {
                    logMessage(LOG_LEVEL_WARNING, "Cannot stop unsupported MIDI note: %d", note);
                }
                break;
            }
            
            case 0x90: { // Note On
                if (pos + 1 >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Note On event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t note = midi_data[pos++];
                uint8_t velocity = midi_data[pos++];
                
                if (velocity == 0) {
                    // Velocity 0 = Note Off
                    int string_index = midi_note_to_string_index(note);
                    if (string_index >= 0 && string_index < NUM_STRINGS) {
                        logMessage(LOG_LEVEL_INFO, "Note Off (velocity 0): %d -> string %d", 
                                  note, string_index);
                        guitar_stop_sound(guitar, string_index); // Stop the sound
                        remove_active_note(note);
                    } else {
                        logMessage(LOG_LEVEL_WARNING, "Cannot stop unsupported MIDI note: %d", note);
                    }
                } else {
                    int string_index = midi_note_to_string_index(note);
                    if (string_index >= 0 && string_index < NUM_STRINGS) {
                        logMessage(LOG_LEVEL_INFO, "Note On: %d -> string %d (velocity %d)", 
                                  note, string_index, velocity);
                        
                        if (add_active_note(note, string_index) >= 0) {
                            guitar_play_string(guitar, string_index);
                            // Could scale volume based on velocity here
                        } else {
                            logMessage(LOG_LEVEL_WARNING, "Too many active notes, ignoring note %d", note);
                        }
                    } else {
                        logMessage(LOG_LEVEL_WARNING, "Unsupported MIDI note: %d (outside expected range)", note);
                    }
                }
                break;
            }
            
            case 0xA0: { // Polyphonic Key Pressure
                if (pos + 1 >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Polyphonic Pressure event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t note = midi_data[pos++];
                uint8_t pressure = midi_data[pos++];
                logMessage(LOG_LEVEL_DEBUG, "Polyphonic pressure: note %d, pressure %d", note, pressure);
                break;
            }
            
            case 0xB0: { // Control Change
                if (pos + 1 >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Control Change event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t controller = midi_data[pos++];
                uint8_t value = midi_data[pos++];
                
                if (controller == 64) { // Sustain pedal
                    logMessage(LOG_LEVEL_INFO, "Sustain pedal: %s", value >= 64 ? "ON" : "OFF");
                    // Could implement sustain logic here
                } else {
                    logMessage(LOG_LEVEL_DEBUG, "Control Change: controller %d, value %d", controller, value);
                }
                break;
            }
            
            case 0xC0: { // Program Change
                if (pos >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Program Change event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t program = midi_data[pos++];
                logMessage(LOG_LEVEL_INFO, "Program Change: %d", program);
                break;
            }
            
            case 0xD0: { // Channel Pressure
                if (pos >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Channel Pressure event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t pressure = midi_data[pos++];
                logMessage(LOG_LEVEL_DEBUG, "Channel pressure: %d", pressure);
                break;
            }
            
            case 0xE0: { // Pitch Bend
                if (pos + 1 >= midi_data_len) {
                    logMessage(LOG_LEVEL_ERROR, "Incomplete Pitch Bend event at position %zu", pos - 1);
                    goto cleanup;
                }
                uint8_t lsb = midi_data[pos++];
                uint8_t msb = midi_data[pos++];
                int16_t bend_value = ((msb << 7) | lsb) - 8192; // Convert to signed
                logMessage(LOG_LEVEL_DEBUG, "Pitch bend: %d", bend_value);
                break;
            }
            
            case 0xF0: { // System messages
                if (status == 0xFF) { // Meta event
                    if (pos >= midi_data_len) {
                        logMessage(LOG_LEVEL_ERROR, "Incomplete Meta event at position %zu", pos - 1);
                        goto cleanup;
                    }
                    uint8_t meta_type = midi_data[pos++];
                    
                    if (meta_type == 0x2F) { // End of Track
                        if (pos >= midi_data_len || midi_data[pos] != 0x00) {
                            logMessage(LOG_LEVEL_ERROR, "Invalid End of Track event at position %zu", pos - 1);
                        } else {
                            pos++; // Skip length byte
                            logMessage(LOG_LEVEL_INFO, "End of Track reached");
                            goto cleanup;
                        }
                    } else {
                        logMessage(LOG_LEVEL_WARNING, "Unsupported meta event: 0x%02X", meta_type);
                        goto cleanup;
                    }
                } else {
                    logMessage(LOG_LEVEL_WARNING, "Unsupported system message: 0x%02X", status);
                    goto cleanup;
                }
                break;
            }
            
            default:
                logMessage(LOG_LEVEL_ERROR, "Unknown MIDI event: 0x%02X at position %zu", status, pos - 1);
                goto cleanup;
        }
        
        // Apply timing delay
        if (delta_time > 0) {
            // delay_seconds(delta_time * seconds_per_tick);
        }
        
        // Clean up finished sounds
        guitar_cleanup_finished_sounds(guitar);
    }
    
cleanup:
    logMessage(LOG_LEVEL_INFO, "MIDI playback finished");
    
    // Clean up any remaining active notes
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        active_notes[i].active = false;
    }
}