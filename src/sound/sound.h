/****************************************************************************************
* File:         sound.h
* Author:       Michiel Dirks
* Created on:   12-08-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef ATMEGA_GAME_SOUND_H
#define ATMEGA_GAME_SOUND_H

#ifdef __cplusplus
#define SOUND_EXTERN_C extern "C"
#else
#define SOUND_EXTERN_C
#endif

#include <stdint.h>
#include <stdbool.h>

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
#define NOTE_CHUNK_SIZE 3
#define SFD_MAGIC "sfdV1"
#define SFD_MAGIC_LEN 5

typedef struct {
    uint16_t frequency;
    uint16_t duration;
} s_Note;

typedef struct {
    const char *filename;
    uint32_t file_reader_pos;

    uint32_t note_count;
    uint32_t reader_note_index;

    s_Note note_buffer[NOTE_CHUNK_SIZE];
    uint8_t buffer_count;
    uint8_t buffer_index;

    bool needs_loading;
} s_SoundReader;

typedef struct {
    uint16_t frequency_offset;
    bool looping;

    s_SoundReader reader;
} s_Sound;

SOUND_EXTERN_C void play_sound(const char *filename, uint16_t frequncy_offset);

SOUND_EXTERN_C void stop_sound_playback(void);

SOUND_EXTERN_C void update_sound_chunks();

#endif //ATMEGA_GAME_SOUND_H