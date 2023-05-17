#include "conversions.h"

/*
  Converts a MIDI note number to note frequency.

  The relationship betweens notes and frequencies is exponential-
  A4 (440 Hz) is twice the frequency of A3 (220 Hz). This uses
  an exponential function to determine the frequency using
  A4 (440 Hz, MIDI note 69) as the "center".

  See en.wikipedia.org/wiki/MIDI_tuning_standard
*/
float midi_note_to_frequency(uint32_t midi_note)
{
  float semitones_away_from_a4 = (float)(midi_note) - (float)(a4_midi_note);
  return powf(2.0f, semitones_away_from_a4 * volts_per_semitone) * a4_frequency; // reciprocal used
}

/*
  Converts a note frequency to the closest corresponding MIDI
  note number.

  This is the inverse of the previous function- keep in mind
  that if the frequency falls inbetween two MIDI notes this
  will discard the "remainder". This method could be extended
  to return the remainder as the number of cents above or below
  the MIDI note.

*/
uint32_t frequency_to_midi_note(float frequency)
{
  float note = 69.0f + logf(frequency * a4_frequency_reciprocal) / logf(2.0f) * semitones_per_octave;
  return ceil(note);
}

/*
  Converts a MIDI note number to Volts/octave.

  Since the note number represents the number of semitones, there is a
  simple linear relationship between note number and Volts/octave. The
  note number is multiplied by the number of Volts per semitone
  (1/12th of a Volt, or about 83.3 mV) to obtain the corresponding
  voltage.
*/
float midi_note_to_volts(uint32_t midi_note)
{
  if (midi_note <= midi_note_at_zero_volts)
  {
    midi_note = 0;
  }
  else
  {
    midi_note -= midi_note_at_zero_volts;
  }
  return midi_note * volts_per_semitone;
}

/*
  Converts Volts/octave to its corresponding MIDI note number.

  Similar to the frequncy to MIDI note number conversion, this
  only returns the nearest MIDI note, so any remainder will be
  discarded.
*/
uint32_t volts_to_midi_note(float volts)
{
  return ceil(midi_note_at_zero_volts + ceil(volts * semitones_per_octave));
}

/*
  Converts Volts/octave to frequency.

  Just like when converting MIDI notes to frequency, there is an
  exponential relationship between Volts/octave and note frequency.
  Helpfully, the same formula used there can be used here.
*/
float volts_to_frequency(float volts)
{
  float semitones = volts * semitones_per_octave;
  float adjusted_semitones = semitones + midi_note_at_zero_volts;
  float semitones_away_from_a4 = adjusted_semitones - (float)(a4_midi_note);
  return powf(2.0f, semitones_away_from_a4 * volts_per_semitone) * a4_frequency;
}

/*
  Converts a note frequency to Volts/octave.

  Just like when converting from frequency to MIDI note, this is
  the inverse of the preceeding function. The formula for
  reversing frequency to MIDI note can be largely reused here.
*/
float frequency_to_volts(float frequency)
{
  float semitones = (float)(a4_midi_note) + logf(frequency * a4_frequency_reciprocal) / logf(2.0f) * semitones_per_octave;
  float adjusted_semitones = semitones - midi_note_at_zero_volts;
  return adjusted_semitones * volts_per_semitone;
}