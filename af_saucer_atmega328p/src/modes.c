#include "modes.h"
#include "led.h"


//------------------------------------------------------------------------------
// definitions

#define NUM_LEDS 16
#define NUM_FLASHER 4


//------------------------------------------------------------------------------
// global variables

static uint16_t svLEDState = 0xffff;    // status for all 16 LEDs
static bool svFlashState = true;  // flasher status


//------------------------------------------------------------------------------
void updateLEDState(uint16_t newState)
{
    svLEDState = newState;
}

//------------------------------------------------------------------------------
void updateFlasherState(bool newState)
{
    svFlashState = newState;
}

//------------------------------------------------------------------------------
void updateLEDs()
{
    // update the 16 edge LEDs
    uint16_t state = svLEDState;
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
        if (svFlashState)
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
