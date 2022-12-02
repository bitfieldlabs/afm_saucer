#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "modes.h"


//------------------------------------------------------------------------------
// definitions

#define LED_UPDATE_INT 20                   // LED update interval [ms]


//------------------------------------------------------------------------------
// global variables

static volatile uint16_t svLEDState = 0xffff;    // status for all 16 LEDs
static volatile bool svFlashState = true;  // flasher status


//------------------------------------------------------------------------------
// declarations



//------------------------------------------------------------------------------
int main(void)
{
    // setup
    DDRD = 0b01100010;                      // all inputs except pins PD1, PD5, PD6
    EICRA |= (1 << ISC00) | (1 << ISC01);   // trigger INT0 on rising edge
    EIMSK |= (1 << INT0);                   // turn on INT0
    EICRA |= (1 << ISC10);                  // trigger INT1 on any edge
    EIMSK |= (1 << INT1);                   // turn on INT1
    sei();                                  // enable interrupts

    // to infinity and beyond
    uint16_t lastLEDState = 0;
    bool lastFlashState = false;
    while (true)
    {
        // update the LED state upon change
        if (svLEDState != lastLEDState)
        {
            updateLEDState(svLEDState);
            lastLEDState = svLEDState;
        }
        if (svFlashState != lastFlashState)
        {
            updateFlasherState(svFlashState);
            lastFlashState = svFlashState;
        }

        // update all LEDs
        updateLEDs();

        // wait a bit
        _delay_ms(LED_UPDATE_INT);
    }
    return 0;
}

//------------------------------------------------------------------------------
ISR (INT0_vect)
{
    // read data from PD4
    uint8_t data = (PIND & 0b00010000);
    //static bool sOn = true;
    //uint8_t data = (sOn) ? 0b00010000 : 0b00000000;
    //sOn = ! sOn;

    // shift new value into LED status
    svLEDState <<= 1;
    svLEDState |= (data >> 4);
}

//------------------------------------------------------------------------------
ISR (INT1_vect)
{
    svFlashState = !svFlashState;
}
