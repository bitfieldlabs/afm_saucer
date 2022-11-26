#include "modes.h"
#include "led.h"
#include "utils.h"


//------------------------------------------------------------------------------
// definitions

#define NUM_LEDS 16
#define NUM_FLASHER 4

#define MODE_IND_ACCUM_STEPS 8

// saucer LED modes
typedef enum SAUCER_MODES_e 
{
    SM_BOOT = 0,        // boot up sequence, all LEDs on
    SM_ATTRACT,         // attract mode, alternating blinking pattern
    SM_GAMEIDLE,        // game mode, saucer idle, all LEDs off
    SM_ATTACK1,         // game mode, saucer attack, part 1
    SM_ATTACK2,         // game mode, saucer attack, part 2
    SM_ATTACK3,         // game mode, saucer attack, part 3
    SM_DESTROYED,       // game mode, saucer destroyed
    SM_TEST,            // saucer test mode, on LED at a time, starting with D16

    SM_NUM              // number of saucer modes
} SAUCER_MODES_t;

//------------------------------------------------------------------------------
// global variables

static uint16_t sLEDState = 0xffff;            // status for all 16 LEDs
static bool sFlashState = true;                // flasher status
static SAUCER_MODES_t sMode = SM_BOOT;          // auto detected saucer mode
static uint8_t sModeIndicators[SM_NUM]= {0};     // accumulated saucer mode indicators


//------------------------------------------------------------------------------
void updateLEDState(uint16_t newState)
{
    sLEDState = newState;

    // try to detect the current mode
    SAUCER_MODES_t mode = SM_BOOT;
    if (sLEDState == 0xffff)
    {
        // all LEDs off
        mode = SM_GAMEIDLE;
    }
    else if (sLEDState == 0x0000)
    {
        // boot up
        mode = SM_BOOT;
    }
    else if ((sLEDState == 0xaaaa) || (sLEDState == 0x5555))
    {
        // attract mode
        mode = SM_ATTRACT;
    }
    else
    {
        uint8_t b = bitsSet(sLEDState);
        if (b == 1)
        {
            mode = SM_TEST;
        }
        else
        {
            mode = SM_ATTACK1;
        }
    }

    // update all indicators
    for (uint8_t i=0; i<SM_NUM; i++)
    {
        if (i==mode)
        {
            if (sModeIndicators[i] < MODE_IND_ACCUM_STEPS)
            {
                sModeIndicators[i]++;
            }
        }
        else if (sModeIndicators[i])
        {
            sModeIndicators[i]--;
        }

        // set the mode to what is currently indicated the most
        if (sModeIndicators[i] > sModeIndicators[sMode])
        {
            sMode = i;
        }
    }
}

//------------------------------------------------------------------------------
void updateFlasherState(bool newState)
{
    sFlashState = newState;
}

//------------------------------------------------------------------------------
void updateLEDs()
{
    // update the 16 edge LEDs
    uint16_t state = sLEDState;
    for (uint8_t i=0; i<NUM_LEDS; i++)
    {
        if (state & 0x01)
        {
            // LED off
            sendPixel(0, 0, 0, true);
        }
        else
        {
            // LED on
            sendPixel(0xaa, 0, 0, true);
        }
        state >>= 1;
    }

    // update the 4 flashers
    for (uint8_t i=0; i<NUM_FLASHER; i++)
    {
        if (sFlashState)
        {
            // LED off
            sendPixel(0, 0, 0, false);
        }
        else
        {
            // LED on
            sendPixel(0xff, 0xff, 0xff, false);
        }
    }
}
