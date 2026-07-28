#include "sound_generator.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define CONCERT_A 220.0
#define SAMPLES_PER_SEC 44100
#define DURATION 16  // Duration in seconds
#define AMPLITUDE 32767 // Max amplitude for int16_t

// Global variables
static int t = 0;

//-----------------------------------------------------------------
//                                          Queue
//-----------------------------------------------------------------

struct Queue* Queue_createQueue(struct Queue* queue, unsigned capacity)
{
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int16_t*)malloc(
                       queue->capacity * sizeof(int16_t));
    return queue;
}
int16_t Queue_isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
int16_t Queue_isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
void Queue_enqueue(struct Queue* queue, int16_t item)
{
    if (Queue_isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
int16_t Queue_dequeue(struct Queue* queue)
{
    if (Queue_isEmpty(queue))
    {
        return 0;
    }
    int16_t item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
int16_t Queue_front(struct Queue* queue)
{
    if (Queue_isEmpty(queue))
    {
        return 0;
    }
    return queue->array[queue->front];
}
int16_t Queue_rear(struct Queue* queue)
{
    if (Queue_isEmpty(queue))
    {
        return 0;
    }
    return queue->array[queue->rear];
}
void Queue_sortQueue(struct Queue* orig_stack)
{

}
void Queue_print(struct Queue* queue)
{
    for (int16_t i = queue->rear; i <= queue->front; i++)
    {
        printf("%d ", queue->array[i]);
    }
    printf("\n");
}

//----------------------------------------------------------------------------------
//                              Karplus-Strong string model
//----------------------------------------------------------------------------------

// Tic functions for different instruments
void tic_string(struct Queue* q) {
    int16_t a = Queue_dequeue(q);
    int16_t b = Queue_front(q);
    Queue_enqueue(q, ((a + b) / 2) * 0.996f);
}

void tic_piano(struct Queue* q) {
    int16_t a = Queue_dequeue(q);
    int16_t b = Queue_front(q);
    Queue_enqueue(q, ((a + b) / 2) * 0.997f);
}

void tic_drum(struct Queue* q) {
    int16_t a = Queue_dequeue(q);
    int16_t b = Queue_front(q);
    float avg = 0.5f * (a + b);
    if ((rand() / (float)RAND_MAX) < 0.5f) avg = -avg;
    Queue_enqueue(q, avg * 0.99f);
}

void tic_bell(struct Queue* q) {
    static float ap_buf1 = 0, ap_buf2 = 0;
    int16_t a = Queue_dequeue(q);
    int16_t b = Queue_front(q);
    float out = 0.5f * (a + b);
    float a_coef = 0.8f;
    float ap_out = -a_coef * out + ap_buf1 + a_coef * ap_buf2;
    ap_buf2 = ap_buf1;
    ap_buf1 = out;
    Queue_enqueue(q, ap_out * 0.994f);
}

void tic_sitar(struct Queue* q) {
    int16_t a = Queue_dequeue(q);
    int16_t b = Queue_front(q);
    float avg = 0.5f * (a + b);
    if ((rand() / (float)RAND_MAX) < 0.005f) avg = -avg;
    Queue_enqueue(q, avg * 0.995f);
}

void tic_kalimba(struct Queue* q) {
    int16_t a = Queue_dequeue(q);
    int16_t b = Queue_front(q);
    float avg = 0.5f * (a + b);
    if ((rand() / (float)RAND_MAX) < 0.01f) avg = -avg;
    Queue_enqueue(q, avg * 0.998f);
}

// Initialize guitar string with Karplus-Strong algorithm
void StringSound_StringSound(struct Queue* queue, double frequency) {
    Queue_createQueue(queue, ceil(44100 / frequency));
}

// Pluck the string by filling the queue with random noise
void StringSound_pluck(struct Queue* queue, double frequency) {
    for (int i = 0; i < ceil(44100 / frequency) - 1; i++) {
        int16_t noise = (int16_t)((rand() % 65536) - 32768); // Range: -32768 to 32767
        Queue_enqueue(queue, noise);
    }
}

// Update the string state using the specified tic function
void StringSound_tic(struct Queue* queue, InstrumentType instrument) {
    switch (instrument) {
        case INSTRUMENT_GUITAR: tic_string(queue); break;
        case INSTRUMENT_PIANO: tic_piano(queue); break;
        case INSTRUMENT_DRUM: tic_drum(queue); break;
        case INSTRUMENT_BELL: tic_bell(queue); break;
        case INSTRUMENT_SITAR: tic_sitar(queue); break;
        case INSTRUMENT_KALIMBA: tic_kalimba(queue); break;
        default: tic_string(queue); break;
    }
    t++;
}

// Update the string state using Karplus-Strong algorithm
void StringSound_tic3(struct Queue* queue) {
    int16_t a = Queue_dequeue(queue);
    int16_t b = Queue_front(queue);
    Queue_enqueue(queue, ((a + b) / 2) * 0.996);
    t++;
}

// Alternative tic function with modified damping
void StringSound_tic1(struct Queue* queue) {
    int16_t a = Queue_dequeue(queue);
    int16_t b = Queue_front(queue);
    Queue_enqueue(queue, ((2 * a + 2 * b * a) / 1.97) * 0.996);
    t++;
}

// Get the current sample from the queue
int16_t StringSound_sample(struct Queue* queue) {
    return Queue_front(queue);
}

// Get the current time
int StringSound_time() {
    return t;
}

//----------------------------------------------------------------------------------
//                              Basic waveform generators
//----------------------------------------------------------------------------------

// Generate sine wave samples
int16_t* makeSineWave(double frequency) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Failed to allocate sine wave samples\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLES_PER_SEC;
        double value = sin(2.0 * M_PI * frequency * t);
        samples[i] = (int16_t)(value * AMPLITUDE * 0.8); // Scale to avoid clipping
    }

    return samples;
}

// Generate square wave samples
int16_t* makeSquareWave(double frequency) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Failed to allocate square wave samples\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLES_PER_SEC;
        double phase = fmod(frequency * t, 1.0);
        samples[i] = (phase < 0.5) ? (int16_t)(AMPLITUDE * 0.8) : (int16_t)(-AMPLITUDE * 0.8);
    }

    return samples;
}

// Generate triangle wave samples
int16_t* makeTriangleWave(double frequency) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Failed to allocate triangle wave samples\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLES_PER_SEC;
        double phase = fmod(frequency * t, 1.0);
        double value = (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
        samples[i] = (int16_t)(value * AMPLITUDE * 0.8);
    }

    return samples;
}

int16_t* makeSawtoothWave(double frequency) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Failed to allocate sawtooth wave samples\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLES_PER_SEC;
        double phase = fmod(frequency * t, 1.0);
        double value = 2.0 * phase - 1.0; // Linear ramp from -1 to 1
        samples[i] = (int16_t)(value * AMPLITUDE * 0.8); // Scale to avoid clipping
    }

    return samples;
}

// Generate white noise samples
int16_t* makeWhiteNoise(void) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Failed to allocate white noise samples\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; i++) {
        double value = (double)(rand() % 65536 - 32768) / 32768.0;
        samples[i] = (int16_t)(value * AMPLITUDE * 0.5); // Lower amplitude for noise
    }

    return samples;
}

//----------------------------------------------------------------------------------
//                              Dispatch
//----------------------------------------------------------------------------------

int16_t* makeSamples2(struct Queue* queue, double frequency, InstrumentType instrument) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Failed to allocate samples\n");
        return NULL;
    }
    StringSound_pluck(queue, frequency);
    for (int i = 0; i < num_samples; i++) {
        StringSound_tic(queue, instrument);
        samples[i] = StringSound_sample(queue);
    }
    return samples;
}

int16_t* makeSamples(struct Queue* queue, double frequency, InstrumentType instrument) {
    int num_samples = SAMPLES_PER_SEC * DURATION;
    int16_t* samples = NULL;

    switch (instrument) {
        case INSTRUMENT_SINE:
            samples = makeSineWave(frequency);
            break;
        case INSTRUMENT_SQUARE:
            samples = makeSquareWave(frequency);
            break;
        case INSTRUMENT_TRIANGLE:
            samples = makeTriangleWave(frequency);
            break;
        case INSTRUMENT_SAWTOOTH:
            samples = makeSawtoothWave(frequency);
            break;
        case INSTRUMENT_WHITENOISE:
            samples = makeWhiteNoise();
            break;
        case INSTRUMENT_GUITAR:
        case INSTRUMENT_PIANO:
        case INSTRUMENT_DRUM:
        case INSTRUMENT_BELL:
        case INSTRUMENT_SITAR:
        case INSTRUMENT_KALIMBA:
            samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
            if (!samples) {
                fprintf(stderr, "Failed to allocate samples\n");
                return NULL;
            }
            StringSound_pluck(queue, frequency);
            for (int i = 0; i < num_samples; i++) {
                StringSound_tic(queue, instrument);
                samples[i] = StringSound_sample(queue);
            }
            break;
        default:
            fprintf(stderr, "Unknown instrument type\n");
            return NULL;
    }

    return samples;
}
