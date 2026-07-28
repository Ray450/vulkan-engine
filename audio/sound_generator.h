#ifndef SOUND_GENERATOR_H
#define SOUND_GENERATOR_H

#include <cstdint>
#include "audio_util.h"  // for InstrumentType

//-----------------------------------------------------------------
//                                          Queue
//-----------------------------------------------------------------

struct Queue
{
    int16_t front, rear, size;
    unsigned capacity;
    int16_t* array;
};

struct Queue* Queue_createQueue(struct Queue* queue, unsigned capacity);
int16_t Queue_isFull(struct Queue* queue);
int16_t Queue_isEmpty(struct Queue* queue);
void Queue_enqueue(struct Queue* queue, int16_t item);
int16_t Queue_dequeue(struct Queue* queue);
int16_t Queue_front(struct Queue* queue);
int16_t Queue_rear(struct Queue* queue);
void Queue_sortQueue(struct Queue* orig_stack);
void Queue_print(struct Queue* queue);

//----------------------------------------------------------------------------------
//                              Karplus-Strong string model
//----------------------------------------------------------------------------------

void tic_string(struct Queue* q);
void tic_piano(struct Queue* q);
void tic_drum(struct Queue* q);
void tic_bell(struct Queue* q);
void tic_sitar(struct Queue* q);
void tic_kalimba(struct Queue* q);

void StringSound_StringSound(struct Queue* queue, double frequency);
void StringSound_pluck(struct Queue* queue, double frequency);
void StringSound_tic(struct Queue* queue, InstrumentType instrument);
void StringSound_tic3(struct Queue* queue);
void StringSound_tic1(struct Queue* queue);
int16_t StringSound_sample(struct Queue* queue);
int StringSound_time();

//----------------------------------------------------------------------------------
//                              Basic waveform generators
//----------------------------------------------------------------------------------

int16_t* makeSineWave(double frequency);
int16_t* makeSquareWave(double frequency);
int16_t* makeTriangleWave(double frequency);
int16_t* makeSawtoothWave(double frequency);
int16_t* makeWhiteNoise(void);

// Dispatches to the right generator/model based on InstrumentType
int16_t* makeSamples(struct Queue* queue, double frequency, InstrumentType instrument);
int16_t* makeSamples2(struct Queue* queue, double frequency, InstrumentType instrument);

#endif // SOUND_GENERATOR_H
