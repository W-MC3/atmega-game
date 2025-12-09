/****************************************************************************************
* File:         sound.h
* Author:       Michiel Dirks
* Created on:   12-08-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef ATMEGA_GAME_SOUND_H
#define ATMEGA_GAME_SOUND_H

#include <stdint.h>
#include <stdbool.h>
#include <SdFat_Adafruit_Fork.h>

/*
 * Custom file format
 * File extension: .sfd (sound frequency duration format)
 *
 * Every sfd V1 file starts with a magic number: 0x73 0x66 0x64 0x56 0x31, which is sfdV1 in ascii
 * Followed by:
 * If the sound should be looping (8 bit)
 * The amount of notes in the file
 * And then the notes. Where each note consists of 2 16 bit integers
 * The first being the frequency, and the second the duration of the frequency in ms
*/

// How many notes to read per chunk (reduce to save RAM)
#define NOTE_CHUNK_SIZE 5
#define SFD_MAGIC "sfdV1"
#define SFD_MAGIC_LEN 5

typedef struct {
    uint16_t frequency;
    uint16_t duration;
} s_Note;

typedef struct {
    const char *filename;
    SdFile file;

    uint32_t note_count;
    uint32_t reader_note_index;

    s_Note note_buffer[NOTE_CHUNK_SIZE];
    uint8_t buffer_count;
    uint8_t buffer_index;
} s_SoundReader;

typedef struct {
    uint16_t frequency_offset;
    bool looping;
    uint16_t playing_index;
    uint32_t playing_start_time;

    s_SoundReader reader;
} s_Sound;

s_Sound register_sound(char *filename);

void reset_sound(s_Sound *sound_ref);

void play_sound(s_Sound *sound_ref);

void set_frequency_offset(s_Sound *sound_ref);

#endif //ATMEGA_GAME_SOUND_H