/****************************************************************************************
* File:         sound.cpp
* Author:       Michiel Dirks
* Created on:   12-08-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "sound.h"
#include "tone.h"
#include <SdFat_Adafruit_Fork.h>
#include <avr/interrupt.h>

#include "hardware/uart/uart.h"

extern SdFat32 SD;

// Global reader instance used for loading sound data from SD
static File32 fileReader;

static void update_sound_playback(void *arg);

// internal function: Load up to 5 notes into the buffer
static bool load_note_chunk(s_SoundReader *reader) {
    reader->buffer_index = 0;
    reader->buffer_count = 0;

    cli();
    if (fileReader.isBusy()) {
        sei();
        return false;
    }

    if (!fileReader.open(reader->filename, O_RDONLY)) {
        sei();
        return false;
    }

    fileReader.seekSet(reader->file_reader_pos);

    // Read up to NOTE_CHUNK_SIZE notes
    for (uint8_t i = 0; i < NOTE_CHUNK_SIZE; i++) {
        if (reader->reader_note_index >= reader->note_count) {
            break;
        }

        s_Note note;
        if (fileReader.read(&note, sizeof(s_Note)) != sizeof(s_Note)) {
            break;
        }

        reader->note_buffer[i] = note;
        reader->reader_note_index++;
        reader->buffer_count++;
    }

    reader->file_reader_pos = fileReader.curPosition();
    fileReader.close();
    sei();
    return reader->buffer_count > 0;
}

s_Sound register_sound(const char *filename) {

    s_Sound sound;
    sound.frequency_offset = 0;
    sound.looping = false;

    sound.reader.filename = filename;
    sound.reader.file_reader_pos = 0;
    sound.reader.note_count = 0;
    sound.reader.reader_note_index = 0;
    sound.reader.buffer_count = 0;
    sound.reader.buffer_index = 0;
    sound.reader.needs_loading = false;

    // Open file
    if (!SD.exists(filename)) {
        return sound;
    }

    if (!fileReader.open(filename, O_RDONLY)) {
        return sound;
    }

    // Read magic number
    char magic[SFD_MAGIC_LEN];
    if (fileReader.read(magic, SFD_MAGIC_LEN) != SFD_MAGIC_LEN) {
        fileReader.close();
        return sound;
    }

    // Read looping (8-bit)
    uint8_t looping_flag;
    if (fileReader.read(&looping_flag, 1) != 1) {
        fileReader.close();
        return sound;
    }
    sound.looping = (looping_flag != 0);

    // Read note count (32-bit)
    if (fileReader.read(&sound.reader.note_count, sizeof(uint32_t)) != sizeof(uint32_t)) {
        fileReader.close();
        return sound;
    }

    sound.reader.file_reader_pos = fileReader.curPosition();
    fileReader.close();

    // Preload first chunk
    load_note_chunk(&sound.reader);

    return sound;
}

// reset playback to first note
void reset_sound(s_Sound *sound_ref) {
    if (!sound_ref) {
        return;
    }

    s_SoundReader *reader = &sound_ref->reader;
    
    // Reset reading position back to start of note data
    reader->file_reader_pos = SFD_MAGIC_LEN + 1 + 4;  // magic + looping + note_count
    reader->reader_note_index = 0;
    reader->buffer_index = 0;
    reader->buffer_count = 0;
    reader->needs_loading = false;

    load_note_chunk(reader);
}

// sets frequency offset
void set_frequency_offset(s_Sound *sound_ref, uint16_t offset) {
    sound_ref->frequency_offset = offset;
}

// ---------------------------------------------------------------------------
// @param: s_Sound *sound_ref sound to start playing
// ---------------------------------------------------------------------------
void play_sound(s_Sound *sound_ref) {
    if (!sound_ref) {
        return;
    }

    s_SoundReader *reader = &sound_ref->reader;

    // No notes available
    if (reader->buffer_count == 0) {
        reader->needs_loading = true;
        return;
    }

    // Kickoff sound playback
    playTone(0, 1, update_sound_playback, sound_ref);
}

void stop_sound_playback(void) {
    // Play tone of 0 hz and remove sound callbacks
    playTone(0, 1, nullptr, nullptr);
}

static void update_sound_playback(void *arg) {
    s_Sound *sound_ref = static_cast<s_Sound *>(arg);

    if (sound_ref == nullptr) {
        return;
    }

    s_SoundReader *reader = &sound_ref->reader;

    // Need next chunk?
    if (reader->buffer_index >= reader->buffer_count) {
        if (reader->reader_note_index >= reader->note_count && !sound_ref->looping) {
            playTone(0, 0, nullptr, nullptr);
            return;
        }
        reader->needs_loading = true;
        // Play the same note as long as new notes have not been loaded instead of overflowing
        reader->buffer_index = reader->buffer_count;
    }

    s_Note *note = &reader->note_buffer[reader->buffer_index];
    reader->buffer_index++;

    uint16_t freq = note->frequency + sound_ref->frequency_offset;
    playTone(freq, note->duration, update_sound_playback, sound_ref);
}

// Poll function to check if sound needs chunk reloading (call from main loop)
void update_sound_chunks(s_Sound *sound_ref) {
    s_SoundReader *reader = &sound_ref->reader;

    if (reader->needs_loading) {
        if (!load_note_chunk(reader)) {
            // Distinguish between read fault or end of song
            if (reader->reader_note_index >= reader->note_count) {
                // End of song
                reset_sound(sound_ref);
                if (sound_ref->looping) {
                    // trigger new sound update
                    playTone(0, 1, update_sound_playback, sound_ref);
                }
            }
        }
        else {
            reader->needs_loading = false;
        }
    }
}
