#ifndef __sample_source_h__
#define __sample_source_h__

#include <Arduino.h>

typedef struct
{
    int16_t valueLeft;
    int16_t valueRight;
} Frame_t;

/**
 * Base class for our sample generators
 **/
class SampleSource
{
public:
    virtual inline void init(float sampleRate) = 0;
    virtual int getSampleRate() = 0;
    // This should fill the samples buffer with the specified number of frames
    // A frame contains a LEFT and a RIGHT sample. Each sample should be signed 16 bits
    virtual void getFrames(Frame_t *frames, int number_frames) = 0;
};

#endif