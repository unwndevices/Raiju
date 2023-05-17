#ifndef NOISE_H
#define NOISE_H
#include "Arduino.h"

#define TABLE_SIZE 4096

class Noise
{
public:
    void Init();
    inline float Process();

private:
    inline float generateNoise();
    PROGMEM float noiseTable[TABLE_SIZE];
    uint16_t playbackPos = 0;
};

inline float Noise::Process()
{
    float output = 0.0f;
    if (playbackPos < TABLE_SIZE)
    {
        output = noiseTable[playbackPos];
        playbackPos++;
    }
    else
    {
        playbackPos = 0;
        output = noiseTable[playbackPos];
        playbackPos++;
    }
    return output;
}

inline float Noise::generateNoise()
{
    return (float)((random(0, 1024) / 512.0) - 1.0);
}

#endif