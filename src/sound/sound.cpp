/****************************************************************************************
* File:         sound.c
* Author:       Michiel Dirks
* Created on:   12-08-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "sound.h"
#include "tone.h"
#include "delay.h"
#include <SdFat_Adafruit_Fork.h>
#include <new>

extern SdFat32 SD;

static void update_sound_playback(void *arg);

// internal function: Load up to 5 notes into the buffer
static bool load_note_chunk(s_SoundReader *r) {
    r->buffer_index = 0;
    r->buffer_count = 0;

    SdFile* file = static_cast<SdFile*>(r->file_handle);
    if (!file || !file->isOpen()) {
        return false;
    }

    // Read up to NOTE_CHUNK_SIZE notes
    for (uint8_t i = 0; i < NOTE_CHUNK_SIZE; i++) {
        if (r->reader_note_index >= r->note_count) {
            break;
        }

        s_Note note;
        if (file->read(&note, sizeof(s_Note)) != sizeof(s_Note)) {
            break;
        }

        r->note_buffer[i] = note;
        r->reader_note_index++;
        r->buffer_count++;
    }

    return r->buffer_count > 0;
}

s_Sound register_sound(const char *filename) {
    s_Sound sound;
    sound.frequency_offset = 0;
    sound.looping = false;
    sound.playing_index = 0;
    sound.playing_start_time = 0;

    sound.reader.filename = filename;
    sound.reader.file_handle = nullptr;
    sound.reader.note_count = 0;
    sound.reader.reader_note_index = 0;
    sound.reader.buffer_count = 0;
    sound.reader.buffer_index = 0;


    // Open file
    if (!SD.exists(filename)) {
        return sound;
    }
    
    // Allocate and open file object
    SdFile* file = static_cast<SdFile*>(::operator new(sizeof(SdFile), std::nothrow));
    if (file) {
        new (file) SdFile();  // placement-new construct
    }
    if (!file || !file->open(filename, O_RDONLY)) {
        if (file) {
            file->~SdFile();
            ::operator delete(file);
        }
        return sound;
    }
    
    sound.reader.file_handle = static_cast<s_FileHandle>(file);

    // Read magic number
    char magic[SFD_MAGIC_LEN];
    if (file->read(magic, SFD_MAGIC_LEN) != SFD_MAGIC_LEN) {
        file->close();
        file->~SdFile();
        ::operator delete(file);
        sound.reader.file_handle = nullptr;
        return sound;
    }
    if (memcmp(magic, SFD_MAGIC, SFD_MAGIC_LEN) != 0) {
        file->close();
        file->~SdFile();
        ::operator delete(file);
        sound.reader.file_handle = nullptr;
        return sound;
    }

    // Read looping (8-bit)
    uint8_t looping_flag;
    if (file->read(&looping_flag, 1) != 1) {
        file->close();
        file->~SdFile();
        ::operator delete(file);
        sound.reader.file_handle = nullptr;
        return sound;
    }
    sound.looping = (looping_flag != 0);

    // Read note count (32-bit)
    if (file->read(&sound.reader.note_count, sizeof(uint32_t)) != sizeof(uint32_t)) {
        file->close();
        file->~SdFile();
        ::operator delete(file);
        sound.reader.file_handle = nullptr;
        return sound;
    }

    // Preload first chunk
    load_note_chunk(&sound.reader);

    return sound;
}

// reset playback to first note
void reset_sound(s_Sound *sound_ref) {
    if (!sound_ref) {
        return;
    }

    s_SoundReader *r = &sound_ref->reader;
    SdFile* file = static_cast<SdFile*>(r->file_handle);
    
    if (!file || !file->isOpen()) {
        return;
    }

    file->seekSet(SFD_MAGIC_LEN + 1 + 4);  // magic + looping + note_count
    r->reader_note_index = 0;
    r->buffer_index = 0;
    r->buffer_count = 0;

    load_note_chunk(r);

    sound_ref->playing_index = 0;
    sound_ref->playing_start_time = scheduler_millis();
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
        return;
    }

    // Current note
    s_Note *note = &reader->note_buffer[reader->buffer_index];

    sound_ref->playing_start_time = scheduler_millis();
    uint16_t freq = note->frequency + sound_ref->frequency_offset;
    playTone(freq, note->duration, update_sound_playback, sound_ref);
}

static void update_sound_playback(void *arg) {
    s_Sound *sound_ref = static_cast<s_Sound *>(arg);

    if (sound_ref == nullptr) {
        return;
    }

    s_SoundReader *reader = &sound_ref->reader;

    uint32_t now = scheduler_millis();

    reader->buffer_index++;
    sound_ref->playing_index++;
    sound_ref->playing_start_time = now;

    // Need next chunk?
    if (reader->buffer_index >= reader->buffer_count) {
        if (!load_note_chunk(reader)) {
            // End of song
            reset_sound(sound_ref);
            if (!sound_ref->looping) {
                return;
            }
        }
    }

    // After possible reload, ensure index within current buffer
    if (reader->buffer_index >= reader->buffer_count) {
        return;
    }

    s_Note *note = &reader->note_buffer[reader->buffer_index];

    uint16_t freq = note->frequency + sound_ref->frequency_offset;
    playTone(freq, note->duration, update_sound_playback, sound_ref);
}
