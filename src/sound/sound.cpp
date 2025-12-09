/****************************************************************************************
* File:         sound.c
* Author:       Michiel Dirks
* Created on:   12-08-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "sound.h"
#include <SdFat_Adafruit_Fork.h>
#include <stdbool.h>

extern SdFat32 SD;

// ---------------------------------------------------------------------------
// Internal: Load up to 5 notes into the buffer
// ---------------------------------------------------------------------------
static bool load_note_chunk(s_SoundReader *r) {
    r->buffer_index = 0;
    r->buffer_count = 0;

    if (!r->file) return false;

    // Read up to NOTE_CHUNK_SIZE notes
    for (uint8_t i = 0; i < NOTE_CHUNK_SIZE; i++) {

        if (r->reader_note_index >= r->note_count)
            break;

        s_Note note;
        if (r->file.read(&note, sizeof(s_Note)) != sizeof(s_Note))
            break;

        r->note_buffer[i] = note;
        r->reader_note_index++;
        r->buffer_count++;
    }

    return r->buffer_count > 0;
}

// ---------------------------------------------------------------------------
// register_sound(): loads header and prepares reader, but does NOT start playback
// ---------------------------------------------------------------------------
s_Sound register_sound(char *filename) {
    s_Sound sound = {0};

    sound.reader.filename = filename;

    // Open file
    if (!SD.exists(filename)) {
        Serial.println("SFD file does not exist!");
        return sound;
    }
    if (!sound.reader.file.open(filename, O_RDONLY)) {
        Serial.println("Failed to open SFD file!");
        return sound;
    }

    // Read magic number
    char magic[SFD_MAGIC_LEN];
    if (sound.reader.file.read(magic, SFD_MAGIC_LEN) != SFD_MAGIC_LEN) {
        Serial.println("SFD: Could not read magic.");
        return sound;
    }
    if (memcmp(magic, SFD_MAGIC, SFD_MAGIC_LEN) != 0) {
        Serial.println("SFD: Invalid magic number.");
        return sound;
    }

    // Read looping (8-bit)
    uint8_t looping_flag;
    if (sound.reader.file.read(&looping_flag, 1) != 1) {
        Serial.println("SFD: Could not read looping flag.");
        return sound;
    }
    sound.looping = (looping_flag != 0);

    // Read note count (32-bit)
    if (sound.reader.file.read(&sound.reader.note_count, sizeof(uint32_t)) != sizeof(uint32_t)) {
        Serial.println("SFD: Could not read note count.");
        return sound;
    }

    // Prepare reader
    sound.reader.reader_note_index = 0;
    sound.reader.buffer_count = 0;
    sound.reader.buffer_index = 0;

    // Preload first chunk
    load_note_chunk(&sound.reader);

    // Start in a reset state
    sound.playing_index = 0;
    sound.playing_start_time = 0;
    sound.frequency_offset = 0;

    return sound;
}

// ---------------------------------------------------------------------------
// reset_sound(): resets playback to first note
// ---------------------------------------------------------------------------
void reset_sound(s_Sound *sound_ref) {
    if (!sound_ref) return;

    s_SoundReader *r = &sound_ref->reader;

    r->file.seekSet(SFD_MAGIC_LEN + 1 + 4);  // magic + looping + note_count
    r->reader_note_index = 0;
    r->buffer_index = 0;
    r->buffer_count = 0;

    load_note_chunk(r);

    sound_ref->playing_index = 0;
    sound_ref->playing_start_time = millis();
}

// ---------------------------------------------------------------------------
// set_frequency_offset(): sets pitch offset
// ---------------------------------------------------------------------------
void set_frequency_offset(s_Sound *sound_ref) {
    // This seems incomplete in your prototype. Probably intended:
    sound_ref->frequency_offset = sound_ref->frequency_offset;
}

// ---------------------------------------------------------------------------
// play_sound(): non-blocking — should be called each frame/tick
// ---------------------------------------------------------------------------
void play_sound(s_Sound *sound_ref) {
    if (!sound_ref) return;

    uint32_t now = millis();
    s_SoundReader *r = &sound_ref->reader;

    // No notes available
    if (r->buffer_count == 0)
        return;

    // Current note
    s_Note *note = &r->note_buffer[r->buffer_index];

    // Has the note finished?
    if (now - sound_ref->playing_start_time >= note->duration) {

        r->buffer_index++;
        sound_ref->playing_index++;
        sound_ref->playing_start_time = now;

        // Need next chunk?
        if (r->buffer_index >= r->buffer_count) {

            if (!load_note_chunk(r)) {
                // End of song
                if (sound_ref->looping) {
                    reset_sound(sound_ref);
                }
                return;
            }
        }

        note = &r->note_buffer[r->buffer_index];
    }

    // Play frequency (user may replace with PWM/etc)
    uint16_t freq = note->frequency + sound_ref->frequency_offset;
    tone(3, freq);  // Example. Replace with your buzzer function.
}

