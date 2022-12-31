/***********************************************************************
 *    _   _   _             _     __                                           
 *   /_\ | |_| |_ __ _  ___| | __/ _|_ __ ___  _ __ ___   /\/\   __ _ _ __ ___ 
 *  //_\\| __| __/ _` |/ __| |/ / |_| '__/ _ \| '_ ` _ \ /    \ / _` | '__/ __|
 * /  _  \ |_| || (_| | (__|   <|  _| | | (_) | | | | | / /\/\ \ (_| | |  \__ \
 * \_/ \_/\__|\__\__,_|\___|_|\_\_| |_|  \___/|_| |_| |_\/    \/\__,_|_|  |___/
 *
 *                              ____ ____ ___ 
 *                              |--< |__, |==]
 *
 *                      ____ ____ _  _ ____ ____ ____
 *                      ==== |--| |__| |___ |=== |--<
 *
 *  Copyright (c) 2022 bitfield labs
 * 
 ***********************************************************************
 *  This file is part of the Attack from Mars! RGB saucer project:
 *  https://github.com/bitfieldlabs/afm_saucer
 *
 *  The AfM RGB saucer is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  AfM RGB saucer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with afterglow.
 *  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
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

    // seed the random number generator
    uint32_t seed = eeprom_read_dword((uint32_t*)0);
    srand(seed);
    eeprom_write_dword((uint32_t*)0, rand());

    // enable interrupts
    sei();

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
    // shift new value into LED status
    svLEDState <<= 1;
    *((uint8_t*)&svLEDState) |= ((PIND >> 4) & 0x01);
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
