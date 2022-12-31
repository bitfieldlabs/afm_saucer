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

#define VSCALE 16       // HSV scale for LED modes   ** CHOOSE A POWER OF 2 **

// saucer LED modes
typedef enum SAUCER_MODES_e 
{
    SM_BOOT = 0,        // boot up sequence, all LEDs on
    SM_ATTRACT,         // attract mode, alternating blinking pattern
    SM_GAMEIDLE,        // game mode, saucer idle, all LEDs off
    SM_ATTACK,          // game mode, saucer attack
    SM_TEST,            // saucer test mode, on LED at a time, starting with D16

    SM_NUM              // number of saucer modes
} SAUCER_MODES_t;

typedef struct LED_MODE_s
{
    uint16_t startH;    // start hue * VSCALE
    uint16_t endH;      // end hue * VSCALE
    uint16_t startV;    // start value * VSCALE
    uint16_t endV;      // end value * VSCALE
    int16_t speedH;     // hue animation speed * VSCALE
    int16_t speedV;     // value animation speed * VSCALE
    int16_t ofsH;       // per-LED hue offset * VSCALE
    int16_t ofsV;       // per-LED value offset * VSCALE
    uint8_t afterglow;  // LED afterglow [steps]   ** CHOOSE A POWER OF 2 **
    uint8_t animSpeed;  // animation frame delay
    uint8_t blinkInt;   // blinking interval [2^n frames], only applied for background patterns!
    bool animDir;       // animation direction, true means clockwise
} LED_MODE_t;

// complete color patterns, defining the behavior for all saucer modes
typedef struct COLOR_PATTERNS_s
{
    const LED_MODE_t * fgLEDModes[SM_NUM];    // Foreground LED modes for all saucer modes
    const LED_MODE_t * bgLEDModes[SM_NUM];    // Background LED modes for all saucer modes
} COLOR_PATTERNS_t;

typedef struct LED_MODE_VALUES_s
{
    uint16_t currH;     // current hue*VSCALE for first LED
    uint16_t currShakerH;     // current hue*VSCALE for first LED when shaking
    uint16_t currV;     // current value*VSCALE for first LED
    int16_t currSpeedH; // current hue speed (signed)
    int16_t currSpeedV; // current value speed (signed)
} LED_MODE_VALUES_t;

static const LED_MODE_t skCMOff =
{
    .startH = 0*VSCALE,
    .endH = 0*VSCALE,
    .startV = 0*VSCALE,
    .endV = 0*VSCALE,
    .speedH = 0,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 0,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMBoot =
{
    .startH = 0*VSCALE,
    .endH = 255*VSCALE,
    .startV = 0*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 8,
    .speedV = 40,
    .ofsH = 16*VSCALE,
    .ofsV = 0,
    .afterglow = 0,
    .animSpeed = 2,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMRed =
{
    .startH = 0*VSCALE,
    .endH = 16*VSCALE,
    .startV = 255*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 1,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 8,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMGreen =
{
    .startH = 82*VSCALE,
    .endH = 86*VSCALE,
    .startV = 255*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 1,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 8,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMBlue =
{
    .startH = 160*VSCALE,
    .endH = 166*VSCALE,
    .startV = 255*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 1,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 8,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMRedOrig =
{
    .startH = 0*VSCALE,
    .endH = 0*VSCALE,
    .startV = 255*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 0,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 1,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMBrightRedOrange =
{
    .startH = 2*VSCALE,
    .endH = 24*VSCALE,
    .startV = 222*VSCALE,
    .endV = 222*VSCALE,
    .speedH = 1,
    .speedV = 0,
    .ofsH = 2,
    .ofsV = 0,
    .afterglow = 16,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMRainbow =
{
    .startH = 0*VSCALE,
    .endH = 255*VSCALE,
    .startV = 155*VSCALE,
    .endV = 155*VSCALE,
    .speedH = 2,
    .speedV = 0,
    .ofsH = 16*VSCALE,
    .ofsV = 0,
    .afterglow = 8,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMTealPulse =
{
    .startH = 103*VSCALE,
    .endH = 180*VSCALE,
    .startV = 0*VSCALE,
    .endV = 4*VSCALE,
    .speedH = 0,
    .speedV = 0,
    .ofsH = 4,
    .ofsV = 4,
    .afterglow = 4,
    .animSpeed = 4,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMYellowPulse =
{
    .startH = 30*VSCALE,
    .endH = 38*VSCALE,
    .startV = 0*VSCALE,
    .endV = 6*VSCALE,
    .speedH = 0,
    .speedV = 0,
    .ofsH = 2,
    .ofsV = 4,
    .afterglow = 4,
    .animSpeed = 4,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMBlueBreathe =
{
    .startH = 153*VSCALE,
    .endH = 170*VSCALE,
    .startV = 0*VSCALE,
    .endV = 14*VSCALE,
    .speedH = 1,
    .speedV = 1,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 4,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMGreenBreathe =
{
    .startH = 78*VSCALE,
    .endH = 84*VSCALE,
    .startV = 0*VSCALE,
    .endV = 14*VSCALE,
    .speedH = 1,
    .speedV = 1,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 4,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMYellowGreenBreathe =
{
    .startH = 50*VSCALE,
    .endH = 58*VSCALE,
    .startV = 0*VSCALE,
    .endV = 24*VSCALE,
    .speedH = 1,
    .speedV = 1,
    .ofsH = 2,
    .ofsV = 2,
    .afterglow = 4,
    .animSpeed = 8,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMRedGreenBreathe =
{
    .startH = 2*VSCALE,
    .endH = 82*VSCALE,
    .startV = 4*VSCALE,
    .endV = 24*VSCALE,
    .speedH = 1,
    .speedV = 8,
    .ofsH = 80,
    .ofsV = 0,
    .afterglow = 4,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMRedGreenPulse =
{
    .startH = 2*VSCALE,
    .endH = 182*VSCALE,
    .startV = 0*VSCALE,
    .endV = 24*VSCALE,
    .speedH = 1,
    .speedV = 24,
    .ofsH = 80,
    .ofsV = 24,
    .afterglow = 8,
    .animSpeed = 8,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMRainbowPulse =
{
    .startH = 0*VSCALE,
    .endH = 255*VSCALE,
    .startV = 8*VSCALE,
    .endV = 24*VSCALE,
    .speedH = 8,
    .speedV = 2,
    .ofsH = 16*VSCALE,
    .ofsV = 4,
    .afterglow = 4,
    .animSpeed = 4,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMYellowBlink =
{
    .startH = 30*VSCALE,
    .endH = 38*VSCALE,
    .startV = 10*VSCALE,
    .endV = 10*VSCALE,
    .speedH = 0,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 4,
    .animSpeed = 0,
    .blinkInt = 4,  // 2^8
    .animDir = false
};

static const LED_MODE_t skCMBrightPinkRed =
{
    .startH = 230*VSCALE,
    .endH = 255*VSCALE,
    .startV = 255*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 4,
    .speedV = 0,
    .ofsH = 4,
    .ofsV = 0,
    .afterglow = 2,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMBrightLightBlue =
{
    .startH = 130*VSCALE,
    .endH = 140*VSCALE,
    .startV = 255*VSCALE,
    .endV = 255*VSCALE,
    .speedH = 4,
    .speedV = 0,
    .ofsH = 2,
    .ofsV = 0,
    .afterglow = 2,
    .animSpeed = 0,
    .blinkInt = 0,
    .animDir = false
};

static const LED_MODE_t skCMAlternate =
{
    .startH = 2*VSCALE,
    .endH = 72*VSCALE,
    .startV = 04*VSCALE,
    .endV = 44*VSCALE,
    .speedH = 0,
    .speedV = 4,
    .ofsH = 72*VSCALE,
    .ofsV = 0*VSCALE,
    .afterglow = 4,
    .animSpeed = 10,
    .blinkInt = 0,
    .animDir = false
};


// Definition of all color patterns
#define NUM_COLOR_PATTERN 16
static const COLOR_PATTERNS_t skColorPatterns[NUM_COLOR_PATTERN] =
{
    // SM_BOOT, SM_ATTRACT, SM_GAMEIDLE, SM_ATTACK, SM_TEST

    // *********** DUMMY PATTERN 0 - SELECTS RANDOM PATTERN FROM 1-15 *********

    // COLOR PATTERN 0
    {
        // foreground modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // ****************** FANCY BACKGROUND PATTERNS *********************

    // COLOR PATTERN 1
    {
        // foreground modes
        { &skCMOff, &skCMRed, &skCMBrightRedOrange, &skCMBrightPinkRed, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMTealPulse, &skCMTealPulse, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 2
    {
        // foreground modes
        { &skCMOff, &skCMRed, &skCMBrightRedOrange, &skCMBrightLightBlue, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMRainbowPulse, &skCMRainbowPulse, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 3
    {
        // foreground modes
        { &skCMOff, &skCMRed, &skCMBrightRedOrange, &skCMBrightLightBlue, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMYellowBlink, &skCMYellowBlink, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 4
    {
        // foreground modes
        { &skCMOff, &skCMRed, &skCMBrightRedOrange, &skCMGreen, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMGreenBreathe, &skCMGreenBreathe, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 5
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 6
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMGreen, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMRedGreenBreathe, &skCMRedGreenBreathe, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 7
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMBlue, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMRedGreenPulse, &skCMRedGreenPulse, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 8
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMBlue, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMYellowGreenBreathe, &skCMYellowGreenBreathe, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 9
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMGreen, &skCMRed, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMAlternate, &skCMAlternate, &skCMOff, &skCMOff }
    },

    // ****************** NO BACKGROUND PATTERNS *********************

    // COLOR PATTERN 10
    {
        // foreground modes
        { &skCMRainbow, &skCMRainbow, &skCMRainbow, &skCMRainbow, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 11
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMGreen, &skCMRainbow, &skCMRed },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 12
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMGreen, &skCMBlue, &skCMRed },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 13
    {
        // foreground modes
        { &skCMBlue, &skCMBlue, &skCMBlue, &skCMBlue, &skCMBlue },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 14
    {
        // foreground modes
        { &skCMGreen, &skCMGreen, &skCMGreen, &skCMGreen, &skCMGreen },
        // background modes
        { &skCMBoot, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 15
    {
        // foreground modes
        { &skCMRedOrig, &skCMRedOrig, &skCMRedOrig, &skCMRedOrig, &skCMRedOrig },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },
};
