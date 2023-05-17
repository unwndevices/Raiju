#pragma once
#ifndef DSY_FM2_H
#define DSY_FM2_H

#include <stdint.h>
#include "synthesis/oscillator.h"
#include "synthesis/Noise.h"
#include "synthesis/filters/svf.h"
#include "utils/dsp.h"

#ifdef __cplusplus

namespace daisysp
{
  /** Simple 2 operator FM synth voice.

      Date: November, 2020

      Author: Ben Sergentanis
  */

  class Fm2
  {
  public:
    Fm2() {}
    ~Fm2() {}

    /** Initializes the FM2 module.
        \param samplerate - The sample rate of the audio engine being run.
    */
    void Init(float samplerate);

    /**  Returns the next sample
     */
    float Process();

    /** Carrier freq. setter
        \param freq Carrier frequency in Hz
    */
    void SetFreq(float freq);

    /** Set modulator freq. relative to carrier
        \param ratio New modulator freq = carrier freq. * ratio
    */
    void SetRatio(float ratio);

    /** Index setter
      \param FM depth, 5 = 2PI rads
  */
    void SetIndex(float index);

    /** Returns the current FM index. */
    float GetIndex();

    /** Resets both oscillators */
    void Reset();

    void SetShape(float amount);
    inline void SetModWaveform(bool wave)
    {
      if (!wave)
        mod_.SetWaveform(Oscillator::WAVE_NOISE);
      else
        mod_.SetWaveform(Oscillator::WAVE_SIN);
    }
    inline void SetFilterAmt(float amt) { famt_ = fclamp(amt, 0.0f, 1.0f); }

    Oscillator car_, mod_;

  private:
    static constexpr float kIdxScalar = 0.2f;
    static constexpr float kIdxScalarRecip = 1.f / kIdxScalar;

    Svf filter;
    float idx_;
    float freq_, lfreq_, ratio_, lratio_, famt_, out_, fout_, noiseval_;
    bool halfSampling = true;
  };
} // namespace daisysp
#endif
#endif
