//
// Created by michiel on 12/3/25.
//

#include "multi_tone.h"

uint16_t get_phase_increment(s_Audio_Channel sample);
uint8_t get_audio_sample_output(s_Audio_Channel sample);

const uint8_t square50_lu[] = { 0, 0, 0, 0, 0, 0, 0, 0, 42, 42, 42, 42, 42, 42, 42, 42 };
const uint16_t phase_increment_table[] = { 275, 291, 308, 327, 346, 367, 388, 412, 436, 462, 489, 518 }; //tuning_words

s_Audio_Channel channels[CHANNEL_COUNT];

// The audio mixing is done here
uint8_t generate_audio_sample() {
    uint8_t out = 0;
    for (uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel) {
        if (channels[channel].playing) {
            channels[channel].phase_acc -= get_phase_increment(channels[channel]);
            if (channels[channel].phase_acc <= 0) {
                channels[channel].phase_acc += INDEX_SIZE;
                channels[channel].phase_idx %= LUT_SIZE;
            }
            out += get_audio_sample_output(channels[channel]);
        }
    }

    return out;
}

void init_channel(uint8_t audio_channel_index, int16_t note) {
    s_Audio_Channel channel = channels[audio_channel_index];
    channel.phase_acc = INDEX_SIZE;
    channel.phase_idx = 0;
    channel.note = (uint8_t) (note % NOTES_COUNT);
    channel.octave = (uint8_t) (note / NOTES_COUNT);
    channel.playing = true;
}

void set_note(uint8_t audio_channel_index, int16_t note) {
    s_Audio_Channel channel = channels[audio_channel_index];
    channel.note = 0;
    if (note >= 0) {
        channel.note = (uint8_t) (note % NOTES_COUNT);
        channel.octave = (uint8_t) (note / NOTES_COUNT);
        channel.playing = true;
    }
    else {
        channel.playing = false;
    }
}

uint16_t get_phase_increment(s_Audio_Channel channel) {
    if (channel.note < NOTES_COUNT) {
        return phase_increment_table[channel.note] << channel.octave;
    }
    return 0;
}

uint8_t get_audio_sample_output(s_Audio_Channel channel) {
    return square50_lu[channel.phase_idx];
}
