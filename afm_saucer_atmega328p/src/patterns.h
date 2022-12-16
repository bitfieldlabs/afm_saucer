#include <avr/io.h>

#define VSCALE 16       // HSV scale for LED modes   ** CHOOSE A POWER OF 2 **

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
    .animDir = false
};

static const LED_MODE_t skCMBoot =
{
    .startH = 40*VSCALE,
    .endH = 52*VSCALE,
    .startV = 0*VSCALE,
    .endV = 16*VSCALE,
    .speedH = 1,
    .speedV = 2,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 0,
    .animSpeed = 0,
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
    .animDir = false
};

// Definition of all color patterns
#define NUM_COLOR_PATTERN 16
static const COLOR_PATTERNS_t skColorPatterns[NUM_COLOR_PATTERN] =
{
    // SM_BOOT, SM_ATTRACT, SM_GAMEIDLE, SM_ATTACK1, SM_ATTACK2, SM_ATTACK3, SM_DESTROYED, SM_TEST

    // COLOR PATTERN 0
    {
        // foreground modes
        { &skCMOff, &skCMRed, &skCMBrightRedOrange, &skCMBrightPinkRed, &skCMBrightPinkRed, &skCMBrightPinkRed, &skCMBrightPinkRed, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMTealPulse, &skCMTealPulse, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 1
    {
        // foreground modes
        { &skCMOff, &skCMRed, &skCMBrightRedOrange, &skCMBrightLightBlue, &skCMBrightLightBlue, &skCMBrightLightBlue, &skCMBrightLightBlue, &skCMRainbow },
        // background modes
        { &skCMBoot, &skCMYellowPulse, &skCMYellowPulse, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 2
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 3
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 4
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 5
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 6
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 7
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 8
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 9
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 10
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 11
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 12
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 13
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 14
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    },

    // COLOR PATTERN 15
    {
        // foreground modes
        { &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed, &skCMRed },
        // background modes
        { &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff, &skCMOff }
    }
};
