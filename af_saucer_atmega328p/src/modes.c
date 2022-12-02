#include "modes.h"
#include "led.h"
#include "utils.h"


//------------------------------------------------------------------------------
// definitions

#define NUM_LEDS 16
#define NUM_FLASHER 4

#define MODE_IND_ACCUM_STEPS 8

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
    int8_t speedH;      // hue animation speed
    int8_t speedV;      // value animation speed
    int8_t ofsH;        // per-LED hue offset
    int8_t ofsV;        // per-LED value offset
    uint8_t afterglow;  // LED afterglow [steps]   ** CHOOSE A POWER OF 2 **
    uint8_t animSpeed;  // animation frame delay
    bool animDir;       // animation direction, true means clockwise
} LED_MODE_t;

typedef struct LED_MODE_VALUES_s
{
    uint16_t currH;     // current hue*VSCALE for first LED
    uint16_t currV;     // current value*VSCALE for first LED
    int8_t currSpeedH;  // current hue speed (signed)
    int8_t currSpeedV;  // current value speed (signed)
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

static const LED_MODE_t skCMRed =
{
    .startH = 0*VSCALE,
    .endH = 8*VSCALE,
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

static const LED_MODE_t skCMBlue =
{
    .startH = 140*VSCALE,
    .endH = 140*VSCALE,
    .startV = 155*VSCALE,
    .endV = 155*VSCALE,
    .speedH = 0,
    .speedV = 0,
    .ofsH = 0,
    .ofsV = 0,
    .afterglow = 8,
    .animSpeed = 0,
    .animDir = false
};

static const LED_MODE_t skCMIdlePulse =
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

//------------------------------------------------------------------------------
// global variables

static uint16_t sLEDState = 0;                 // status for all 16 LEDs, 0 means LED off
static bool sFlashState = true;                // flasher status
static SAUCER_MODES_t sMode = SM_NUM;          // auto detected saucer mode
static uint8_t sModeIndicators[SM_NUM]= {0};   // accumulated saucer mode indicators
static LED_MODE_t sFGMode;                     // foreground color mode
static LED_MODE_t sBGMode;                     // background color mode
static LED_MODE_VALUES_t sFGModeValues[NUM_LEDS];        // current foreground mode values
static LED_MODE_VALUES_t sBGModeValues[NUM_LEDS];        // current background mode values
static uint8_t sLEDActive[NUM_LEDS] = { 0 };   // LED active status (afterglow counter)
static uint8_t sBGAnimCount = 0;               // background animation countdown


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
        setMode(SM_BOOT);
    }

    // try to detect the current mode
    SAUCER_MODES_t mode = SM_BOOT;
    if (sLEDState == 0x0000)
    {
        // all LEDs off
        mode = SM_GAMEIDLE;
    }
    else if (sLEDState == 0xffff)
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
void updateFlasherState(bool newState)
{
    sFlashState = newState;
}

//------------------------------------------------------------------------------
void getColor(uint8_t pos, uint8_t agStep, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (agStep)
    {
        if (agStep >= sFGMode.afterglow)
        {
            // full foreground color
            const LED_MODE_VALUES_t *mv = &sFGModeValues[pos];
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
        hsv2rgb((mv->currH>>4), 255, (mv->currV>>4), r, g, b);
    }
}

//------------------------------------------------------------------------------
void nextValue(uint16_t *v, int8_t *inc, int16_t min, int16_t max)
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

//------------------------------------------------------------------------------
void initValues(const LED_MODE_t *ledMode, LED_MODE_VALUES_t *ledModeValues)
{
    int8_t ofsH = ledMode->ofsH;
    int8_t ofsV = ledMode->ofsV;
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
    switch (mode)
    {
        case SM_BOOT: sFGMode = skCMOff; sBGMode = skCMIdlePulse; break;
        case SM_ATTRACT: sFGMode = skCMRed; sBGMode = skCMIdlePulse; break;
        case SM_TEST: sFGMode = skCMBlue; sBGMode = skCMOff; break;
        case SM_GAMEIDLE: sFGMode = skCMBlue; sBGMode = skCMIdlePulse; break;
        default: sFGMode = skCMRed; sBGMode = skCMOff; break;
    }

    // apply mode to all LED values
    initValues(&sFGMode, &sFGModeValues[0]);
    initValues(&sBGMode, &sBGModeValues[0]);

    // initialize the animation counter
    sBGAnimCount = sBGMode.animSpeed;

    sMode = mode;
}
