#ifndef CONVERSIONS_H
#define CONVERSIONS_H
#include <Arduino.h>
#include <cmath>

/*
  This sets the MIDI note that corresponds to 0 volts. Typically, this is
  either C0 (MIDI note 12) or C1 (MIDI note 24).
*/
static const uint32_t midi_note_at_zero_volts = 12;
static const float semitones_per_octave = 12.0f;
static const float volts_per_semitone = 0.08333333333f;
static const float a4_frequency = 440.0f;
static const float a4_frequency_reciprocal = 0.002272727273f;
static const uint32_t a4_midi_note = 69;

float midi_note_to_frequency(uint32_t midi_note);
uint32_t frequency_to_midi_note(float frequency);
float midi_note_to_volts(uint32_t midi_note);
uint32_t volts_to_midi_note(float volts);
float volts_to_frequency(float volts);
float frequency_to_volts(float frequency);

#endif // !CONVERSIONS_H