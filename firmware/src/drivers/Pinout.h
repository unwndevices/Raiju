/*************************
Raiju's pins definitions
Revision B
*************************/
#ifndef PINOUT_H
#define PINOUT_H

#define STAGE_OUT 12

/// DAC
#define DAC_DIN 23
#define DAC_BCLK 18
#define DAC_LRCLK 5

/// COMMS
#define SDA 21
#define SCL 22
#define TX 1
#define RX 3

/// LEDS
#define LED_TRIG 13
#define LED_ATTACK 27
#define LED_DECAY 33
#define LED_SUSTAIN 32
#define LED_RELEASE 19

/// MUX

#define S0 17
#define S1 16
#define S2 4
#define MUX_A 35
#define MUX_D 2

/// DIGITAL INPUTS

#define GATE 39
#define TRIG 34
#define TRIG_SW (uint8_t)0
#define TOGGLE_H (uint8_t)1
#define TOGGLE_L (uint8_t)2
#define STAGE_SW (uint8_t)3

/// ANALOG INPUTS

#define CV_SUSTAIN 36
#define POT_ATTACK 0
#define CV_ATTACK 1
#define POT_DECAY 2
#define CV_DECAY 3
#define POT_SUSTAIN 4
#define POT_RELEASE 5
#define CV_RELEASE 6
#define POT_SHAPE 7

/// DAC OUT

#define OFFSET_CV 25
#define OFFSET_ENV 26

#endif // !PINOUT_H