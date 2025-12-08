//
// Created by michiel on 12/3/25.
//

#ifndef ATMEGA_GAME_MULTI_TONE_H
#define ATMEGA_GAME_MULTI_TONE_H

#include <stdint.h>
#include <stdbool.h>

#define CHANNEL_COUNT 3
#define LUT_SIZE 16
#define INDEX_SIZE 2047
#define NOTES_COUNT 12

typedef struct
{
    int16_t phase_acc;
    uint8_t phase_idx;
    uint8_t note;
    uint8_t octave;
    bool playing;
} s_Audio_Channel;

void init_channel(uint8_t audio_channel_index, int16_t note);
void set_note(uint8_t audio_channel_index, int16_t note);
uint8_t generate_audio_sample();

#endif //ATMEGA_GAME_MULTI_TONE_H