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
static volatile bool svFlashState = false;       // flasher status
static volatile bool svShakerState = false;      // shaker status


//------------------------------------------------------------------------------
int main(void)
{
    // setup
    DDRD = 0b01100010;                      // all inputs except pins PD1, PD5, PD6
    DDRC = 0b00000000;                      // config DIPs on PC0-PC3
    PORTC |= 0b00001111;                    // enable DIP switch pullups
    EICRA |= (1 << ISC00) | (1 << ISC01);   // trigger INT0 on rising edge
    EIMSK |= (1 << INT0);                   // turn on INT0
    EICRA |= (1 << ISC11);                  // trigger INT1 on falling edge
    EIMSK |= (1 << INT1);                   // turn on INT1
    PCICR |= (1 << PCIE2);                  // enable pin interrupt
    PCMSK2 |= (1 << PCINT23);               // pin change interrupt on PD7 (PCINT23)
    sei();                                  // enable interrupts

    // to infinity and beyond
    while (true)
    {
        // update the LED state upon change
        updateLEDState(svLEDState);
        if (svFlashState)
        {
            triggerFlasher();
            svFlashState = false;
        }
        if (svShakerState)
        {
            triggerShaker();
            svShakerState = false;
        }

        // update all LEDs
        updateLEDs();

        // wait a bit
        _delay_ms(LED_UPDATE_INT);
    }
    return 0;
}

//------------------------------------------------------------------------------
// LED data interrupt
ISR(INT0_vect)
{
    // read data from PD4
    uint8_t data = (PIND & 0b00010000);

    // shift new value into LED status
    svLEDState <<= 1;
    svLEDState |= (data >> 4);
}

//------------------------------------------------------------------------------
// flasher interrupt
ISR(INT1_vect)
{
    svFlashState = true;
}

//------------------------------------------------------------------------------
// shaker interrupt
ISR(PCINT2_vect)
{
    svShakerState = true;
}
