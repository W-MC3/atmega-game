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

#include "hardware/uart/uart.h"

s_Sound playing_sound;

extern SdFat32 SD;

// Global reader instance used for loading sound data from SD
static File32 fileReader;

static void update_sound_playback();

// internal function: Load up to 5 notes into the buffer
static bool load_note_chunk() {
    playing_sound.reader.buffer_index = 0;
    playing_sound.reader.buffer_count = 0;

    if (fileReader.isBusy()) {
        return false;
    }

    if (!fileReader.open(playing_sound.reader.filename, O_RDONLY)) {
        return false;
    }

    fileReader.seekSet(playing_sound.reader.file_reader_pos);

    // Read up to NOTE_CHUNK_SIZE notes
    for (uint8_t i = 0; i < NOTE_CHUNK_SIZE; i++) {
        if (playing_sound.reader.reader_note_index >= playing_sound.reader.note_count) {
            break;
        }

        s_Note note;
        if (fileReader.read(&note, sizeof(s_Note)) != sizeof(s_Note)) {
            break;
        }

        playing_sound.reader.note_buffer[i] = note;
        playing_sound.reader.reader_note_index++;
        playing_sound.reader.buffer_count++;
    }

    playing_sound.reader.file_reader_pos = fileReader.curPosition();
    fileReader.close();
    return playing_sound.reader.buffer_count > 0;
}

void play_sound(const char *filename, uint16_t frequncy_offset) {
    playing_sound.frequency_offset = frequncy_offset;
    playing_sound.looping = false;

    playing_sound.reader.filename = filename;
    playing_sound.reader.file_reader_pos = 0;
    playing_sound.reader.note_count = 0;
    playing_sound.reader.reader_note_index = 0;
    playing_sound.reader.buffer_count = 0;
    playing_sound.reader.buffer_index = 0;
    playing_sound.reader.needs_loading = false;

    // Open file
    if (!SD.exists(filename)) {
        return;
    }

    if (!fileReader.open(filename, O_RDONLY)) {
        return;
    }

    // Read magic number
    char magic[SFD_MAGIC_LEN];
    if (fileReader.read(magic, SFD_MAGIC_LEN) != SFD_MAGIC_LEN) {
        fileReader.close();
        return;
    }

    // Read looping (8-bit)
    uint8_t looping_flag;
    if (fileReader.read(&looping_flag, 1) != 1) {
        fileReader.close();
        return;
    }
    playing_sound.looping = (looping_flag != 0);

    // Read note count (32-bit)
    if (fileReader.read(&playing_sound.reader.note_count, sizeof(uint32_t)) != sizeof(uint32_t)) {
        fileReader.close();
        return;
    }

    playing_sound.reader.file_reader_pos = fileReader.curPosition();
    fileReader.close();

    // Preload first chunk
    load_note_chunk();

    // Kickoff sound playback
    playTone(0, 1, update_sound_playback);
}

void stop_sound_playback(void) {
    // Play tone of 0 hz and remove sound callbacks
    playTone(0, 0, nullptr);
}

static void update_sound_playback() {
    s_SoundReader *reader = &playing_sound.reader;

    // Check if we need to load more notes BEFORE accessing the buffer
    if (reader->buffer_index >= reader->buffer_count) {
        if (reader->reader_note_index >= reader->note_count && !playing_sound.looping) {
            playTone(0, 0, nullptr);
            return;
        }
        reader->needs_loading = true;
        playTone(0, 10, update_sound_playback);
        return;
    }

    // Access note at current index (guaranteed to be valid at this point)
    s_Note *note = &reader->note_buffer[reader->buffer_index];
    reader->buffer_index++;

    uint16_t freq = note->frequency + playing_sound.frequency_offset;
    playTone(freq, note->duration, update_sound_playback);
}

// Poll function to check if sound needs chunk reloading
void update_sound_chunks() {
    s_SoundReader *reader = &playing_sound.reader;

    if (reader->needs_loading) {
        if (!load_note_chunk()) {
            // Distinguish between read fault or end of song
            if (reader->reader_note_index >= reader->note_count) {
                // End of song
                if (playing_sound.looping) {
                    // Restart playing sound, this will go pretty deep in the call stack and may become a problem in the future.
                    // Tough looping sounds are not used in the game itself, so it will probably be fine
                    play_sound(playing_sound.reader.filename, playing_sound.frequency_offset);
                }
            }
        }
        else {
            reader->needs_loading = false;
        }
    }
}