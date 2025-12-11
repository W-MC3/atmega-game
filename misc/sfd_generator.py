#!/usr/bin/env python3
import struct
import json
import csv
import sys
from pathlib import Path

MAGIC = b"sfdV1"  # 5 bytes


def load_notes_from_json(path: Path):
    with open(path, "r") as f:
        data = json.load(f)

    if "looping" not in data or "notes" not in data:
        raise ValueError("JSON must contain 'looping' and 'notes' fields")

    notes = []
    for entry in data["notes"]:
        freq = entry.get("frequency")
        dur = entry.get("duration")

        if freq is None or dur is None:
            raise ValueError("Each note must include 'frequency' and 'duration'")

        notes.append((int(freq), int(dur)))

    return bool(data["looping"]), notes


def load_notes_from_csv(path: Path):
    notes = []
    looping = False

    with open(path, newline="") as csvfile:
        reader = csv.DictReader(csvfile)

        if not {"frequency", "duration"} <= set(reader.fieldnames):
            raise ValueError("CSV must contain 'frequency' and 'duration' headers")

        for row in reader:
            freq = int(row["frequency"])
            dur = int(row["duration"])
            notes.append((freq, dur))

        # Optional column
        if "looping" in reader.fieldnames:
            looping = bool(int(row["looping"]))

    return looping, notes


def write_sfd(path: Path, looping: bool, notes):
    with open(path, "wb") as f:
        # Header
        f.write(MAGIC)                                 # 5 bytes
        f.write(struct.pack("B", 1 if looping else 0)) # 1 byte
        f.write(struct.pack("<I", len(notes)))         # 4 bytes, little-endian note count

        # Notes
        for (freq, dur) in notes:
            f.write(struct.pack("<HH", freq, dur))     # frequency, duration (uint16, uint16)

    print(f"✓ Wrote {len(notes)} notes to {path}")


def guess_input_file(path: Path):
    if path.suffix.lower() == ".json":
        return load_notes_from_json(path)

    if path.suffix.lower() == ".csv":
        return load_notes_from_csv(path)

    raise ValueError("Unsupported file type. Use JSON or CSV.")


def main():
    if len(sys.argv) < 3:
        print("Usage:")
        print("  python make_sfd.py input.json output.sfd")
        print("  python make_sfd.py input.csv output.sfd")
        print()
        print("JSON format:")
        print('{\n  "looping": true,\n  "notes": [ {"frequency":440,"duration":500}, ... ]\n}')
        sys.exit(1)

    in_path = Path(sys.argv[1])
    out_path = Path(sys.argv[2])

    looping, notes = guess_input_file(in_path)
    write_sfd(out_path, looping, notes)


if __name__ == "__main__":
    main()
