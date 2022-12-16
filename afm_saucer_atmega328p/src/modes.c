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

#include <stdlib.h>
#include <avr/io.h>
#include "modes.h"
#include "led.h"
#include "utils.h"
#include "patterns.h"


//------------------------------------------------------------------------------
// definitions

#define NUM_LEDS 16
#define NUM_FLASHER 4

#define FLASH_DURATION 4                    // flasher duration [frames]
#define SHAKER_DURATION 64                  // shaker duration [frames]

#define MODE_IND_ACCUM_STEPS 64


//------------------------------------------------------------------------------
// global variables

static uint16_t sLEDState = 0;                 // status for all 16 LEDs, 0 means LED off
static uint8_t sFlashState = 0;                // flasher countdown [frames]
static uint8_t sShakerState = 0;               // shaker countdown [frames]
static SAUCER_MODES_t sMode = SM_NUM;          // auto detected saucer mode
static uint8_t sModeIndicators[SM_NUM]= {0};   // accumulated saucer mode indicators
static LED_MODE_t sFGMode;                     // foreground color mode
static LED_MODE_t sBGMode;                     // background color mode
static LED_MODE_VALUES_t sFGModeValues[NUM_LEDS];        // current foreground mode values
static LED_MODE_VALUES_t sBGModeValues[NUM_LEDS];        // current background mode values
static uint8_t sLEDActive[NUM_LEDS] = { 0 };   // LED active status (afterglow counter)
static uint8_t sBGAnimCount = 0;               // background animation countdown
static uint8_t sCfg = 0;                       // pattern configuration id
static uint32_t sFrameCnt = 0;                  // frame counter


//------------------------------------------------------------------------------
// declarations

void setMode(SAUCER_MODES_t mode);


//------------------------------------------------------------------------------
void updateLEDState(uint16_t newState)
{
    sLEDState = ~newState;

    // initialize the modes
    if (sMode >= SM_NUM)
    {
        srand(ADC);
        setMode(SM_BOOT);
    }

    // try to detect the current mode
    SAUCER_MODES_t mode = SM_BOOT;
    if (sLEDState == 0x0000)
    {
        // game started, UFO idle
        mode = SM_GAMEIDLE;

    }
    else if (sLEDState == 0xffff)
    {
        // boot up
        mode = SM_BOOT;
    }
    else if ((sLEDState == 0xcccc) || (sLEDState == 0x3333) ||
             (sLEDState == 0x6666) || (sLEDState == 0x9999))
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
    SAUCER_MODES_t newMode = sMode;
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
        if (sModeIndicators[i] > sModeIndicators[newMode])
        {
            newMode = i;
        }
    }

    // switch mode if indicated
    if (newMode != sMode)
    {
        setMode(newMode);
    }
}

//------------------------------------------------------------------------------
void triggerFlasher()
{
    sFlashState = FLASH_DURATION;
}

//------------------------------------------------------------------------------
void triggerShaker()
{
    sShakerState = SHAKER_DURATION;
}


//------------------------------------------------------------------------------
void getColor(uint8_t pos, uint8_t agStep, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (agStep)
    {
        if (agStep >= sFGMode.afterglow)
        {
            // full foreground color
            LED_MODE_VALUES_t *mv = &sFGModeValues[pos];

            // randomly add sparkles when shaken
            if (sShakerState && ((rand() % 4) == 0))
            {
                mv->currH = ((uint8_t)rand() << 4);
                mv->currV = 255*VSCALE;
            }

            hsv2rgb((mv->currH>>4), 255, (mv->currV>>4), r, g, b);
        }
        else
        {
            // afterglow -> mix foreground and background color
            uint8_t rf, gf, bf, rb, gb, bb;
            const LED_MODE_VALUES_t *mv = &sFGModeValues[pos];
            hsv2rgb((mv->currH>>4), 255, (mv->currV>>4), &rf, &gf, &bf);
            mv = &sBGModeValues[pos];
            hsv2rgb((mv->currH>>4), 255, (mv->currV>>4), &rb, &gb, &bb);

            uint8_t ratio = (agStep * (256/sFGMode.afterglow));
            *r = blend8(rb, rf, ratio);
            *g = blend8(gb, gf, ratio);
            *b = blend8(bb, bf, ratio);
        }
    }
    else
    {
        // full background color
        const LED_MODE_VALUES_t *mv = &sBGModeValues[pos];

        // blinking
        if ((sBGMode.blinkInt) && ((sFrameCnt >> sBGMode.blinkInt) % 2))
        {
            // off
            *r = *g = *b = 0;
        }
        else
        {
            // on
            hsv2rgb((mv->currH>>4), 255, (mv->currV>>4), r, g, b);
        }
    }
}

//------------------------------------------------------------------------------
void nextValue(uint16_t *v, int16_t *inc, int16_t min, int16_t max)
{
    int16_t nv = ((int16_t)*v + *inc);
    if (nv > max)
    {
        nv = max;
        *inc = -(*inc);
    }
    else if (nv < min)
    {
        nv = min;
        *inc = -(*inc);
    }
    *v = (uint16_t)nv;
}

//------------------------------------------------------------------------------
void advanceMode(const LED_MODE_t *ledMode, LED_MODE_VALUES_t *ledModeValues)
{
    // advance for all LEDs
    for (uint8_t i=0; i<NUM_LEDS; i++)
    {
        nextValue(&ledModeValues->currH, &ledModeValues->currSpeedH, ledMode->startH, ledMode->endH);
        nextValue(&ledModeValues->currV, &ledModeValues->currSpeedV, ledMode->startV, ledMode->endV);
        ledModeValues++;
    }
}

//------------------------------------------------------------------------------
void rotateBGLEDs(bool dir)
{
    if (dir)
    {
        // rotate clockwise
        LED_MODE_VALUES_t tmp = sBGModeValues[0];
        for (uint8_t i=0; i<(NUM_LEDS-1); i++)
        {
            sBGModeValues[i] = sBGModeValues[i+1];
        }
        sBGModeValues[NUM_LEDS-1] = tmp;
    }
    else
    {
        // rotate counterclockwise
        LED_MODE_VALUES_t tmp = sBGModeValues[NUM_LEDS-1];
        for (uint8_t i=NUM_LEDS-1; i>0; i--)
        {
            sBGModeValues[i] = sBGModeValues[i-1];
        }
        sBGModeValues[0] = tmp;
    }

    // reset the animation counter
    sBGAnimCount = sBGMode.animSpeed;
}

//------------------------------------------------------------------------------
void updateLEDs()
{
    // advance the mode
    advanceMode(&sFGMode, &sFGModeValues[0]);
    advanceMode(&sBGMode, &sBGModeValues[0]);
    if (sBGMode.animSpeed)
    {
        sBGAnimCount--;
        if (sBGAnimCount == 0)
        {
            // rotate the background LEDs
            rotateBGLEDs(sBGMode.animDir);
        }
    }

    uint8_t r[NUM_LEDS];
    uint8_t g[NUM_LEDS];
    uint8_t b[NUM_LEDS];

    // prepare all LEDs
    uint16_t state = sLEDState;
    for (uint8_t i=0; i<NUM_LEDS; i++)
    {
        // activate the LED for the afterglow duration
        if (state & 0x01)
        {
            sLEDActive[i] = sFGMode.afterglow;
        }
        else
        {
            if (sLEDActive[i])
            {
                sLEDActive[i]--;
            }
        }

        // determine the current LED color
        getColor(i, sLEDActive[i], &r[i], &g[i], &b[i]);
        state >>= 1;
    }

    // now update all 16 LEDs
    for (uint8_t i=0; i<NUM_LEDS; i++)
    {
        sendPixel(r[i], g[i], b[i], true);
    }

    // update the 4 flashers
    for (uint8_t i=0; i<NUM_FLASHER; i++)
    {
        if (!sFlashState)
        {
            // LED off
            sendPixel(0, 0, 0, false);
        }
        else
        {
            // LED on
            //sendPixel(0xff, 0xff, 0xff, false);
            sendPixel(174, 255, 171, false);
        }
    }
    if (sFlashState)
    {
        sFlashState--;
    }
    if (sShakerState)
    {
        sShakerState--;
    }

    // check for configuration changes
    uint8_t cfg = (~PINC & 0x0f);
    if (cfg != sCfg)
    {
        sCfg = cfg;
        setMode(sMode);
    }

    sFrameCnt++;
}

//------------------------------------------------------------------------------
void initValues(const LED_MODE_t *ledMode, LED_MODE_VALUES_t *ledModeValues)
{
    int16_t ofsH = ledMode->ofsH;
    int16_t ofsV = ledMode->ofsV;
    uint16_t h = ledMode->startH;
    uint16_t v = ledMode->startV;
    for (uint8_t i=0; i<NUM_LEDS; i++)
    {
        ledModeValues->currH = h;
        ledModeValues->currSpeedH = ((ofsH < 0) == (ledMode->ofsH < 0)) ? ledMode->speedH : -ledMode->speedH;
        nextValue(&h, &ofsH, ledMode->startH, ledMode->endH);
        ledModeValues->currV = v;
        ledModeValues->currSpeedV = ((ofsV < 0) == (ledMode->ofsV < 0)) ? ledMode->speedV : -ledMode->speedV;
        nextValue(&v, &ofsV, ledMode->startV, ledMode->endV);
        ledModeValues++;
    }
}

//------------------------------------------------------------------------------
void setMode(SAUCER_MODES_t mode)
{
    // set the mode parameters
    if ((sCfg < NUM_COLOR_PATTERN) && (mode < SM_NUM))
    {
        sFGMode = *(skColorPatterns[sCfg].fgLEDModes[mode]);
        sBGMode = *(skColorPatterns[sCfg].bgLEDModes[mode]);

        // apply mode to all LED values
        initValues(&sFGMode, &sFGModeValues[0]);
        initValues(&sBGMode, &sBGModeValues[0]);

        // initialize the animation counter
        sBGAnimCount = sBGMode.animSpeed;

        sMode = mode;
    }
}
