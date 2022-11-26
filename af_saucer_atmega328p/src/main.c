#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "led.h"


//------------------------------------------------------------------------------
// definitions

#define NUM_LEDS 16
#define NUM_FLASHER 4


//------------------------------------------------------------------------------
// global variables

static volatile uint16_t svLEDState = 0xffff;    // status for all 16 LEDs
static volatile bool svFlashState = true;  // flasher status


//------------------------------------------------------------------------------
// declarations

void updateLEDs();
void updateFlasher();


//------------------------------------------------------------------------------
int main(void)
{
    // setup
    DDRD = 0b01100010;                      // all inputs except pins PD1, PD5, PD6
    EICRA |= (1 << ISC00) | (1 << ISC01);   // trigger INT0 on rising edge
    EIMSK |= (1 << INT0);                   // turn on INT0
    EICRA |= (1 << ISC10);                  // trigger INT1 on any edge
    EIMSK |= (1 << INT1);                   // turn on INT1
    sei();


    // to infinity and beyond
    uint16_t lastLEDState = 0;
    bool lastFlashState = false;
    while(1)
    {
        // update all LEDS upon change
        if (svLEDState != lastLEDState)
        {
            updateLEDs();
            lastLEDState = svLEDState;
        }
        if (svFlashState != lastFlashState)
        {
            updateFlasher();
            lastFlashState = svFlashState;
        }
        _delay_ms(20);
    }
    return 0;
}

//------------------------------------------------------------------------------
ISR (INT0_vect)
{
    // read data from PD4
    uint8_t data = (PIND & 0b00010000);

    // shift new value into LED status
    svLEDState <<= 1;
    svLEDState |= (data >> 4);
}

//------------------------------------------------------------------------------
ISR (INT1_vect)
{
    svFlashState = !svFlashState;
}

//------------------------------------------------------------------------------
void updateLEDs()
{
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
}

//------------------------------------------------------------------------------
void updateFlasher()
{
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
