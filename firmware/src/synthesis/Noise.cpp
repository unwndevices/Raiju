#include "Arduino.h"
#include "Noise.h"



void Noise::Init()
{
        randomSeed(127);
        for (uint16_t i = 0; i < TABLE_SIZE; i++)
        {
            noiseTable[i] = generateNoise();
        }

}

