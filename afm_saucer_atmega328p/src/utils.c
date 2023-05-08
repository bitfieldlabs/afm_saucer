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

#include "utils.h"

// using the principle from https://graphics.stanford.edu/~seander/bithacks.html


//------------------------------------------------------------------------------
static const unsigned char skBitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

//------------------------------------------------------------------------------
uint8_t bitsSet(uint16_t v)
{
    return (skBitsSetTable256[v & 0xff] + skBitsSetTable256[(v >> 8) & 0xff]);
}

//------------------------------------------------------------------------------
void hsv2rgb(uint8_t H, uint8_t S, uint8_t V, uint8_t *R, uint8_t *G, uint8_t *B)
{
  const uint32_t s = (6 * (uint32_t)H) >> 8;               /* the segment 0..5 (360/60 * [0..255] / 256) */
  const uint32_t t = (6 * (uint32_t)H) & 0xff;             /* within the segment 0..255 (360/60 * [0..255] % 256) */
  const uint32_t l = ((uint32_t)V * (255 - (uint32_t)S)) >> 8; /* lower level */
  const uint32_t r = ((uint32_t)V * (uint32_t)S * t) >> 16;    /* ramp */
  switch (s)
  {
    case 0: *R = (uint8_t)V;        *G = (uint8_t)(l + r);    *B = (uint8_t)l;       break;
    case 1: *R = (uint8_t)(V - r);  *G = (uint8_t)V;          *B = (uint8_t)l;       break;
    case 2: *R = (uint8_t)l;        *G = (uint8_t)V;          *B = (uint8_t)(l + r); break;
    case 3: *R = (uint8_t)l;        *G = (uint8_t)(V - r);    *B = (uint8_t)V;       break;
    case 4: *R = (uint8_t)(l + r);  *G = (uint8_t)l;          *B = (uint8_t)V;       break;
    case 5: *R = (uint8_t)V;        *G = (uint8_t)l;          *B = (uint8_t)(V - r); break;
  }
}

//------------------------------------------------------------------------------
// Taken from the FastLED library:
// https://github.com/FastLED/FastLED
// MIT license
uint8_t blend8( uint8_t a, uint8_t b, uint8_t amountOfB)
{
    // The BLEND_FIXED formula is
    //
    //   result = (  A*(amountOfA) + B*(amountOfB)              )/ 256
    //
    // …where amountOfA = 255-amountOfB.
    //
    // This formula will never return 255, which is why the BLEND_FIXED + SCALE8_FIXED version is
    //
    //   result = (  A*(amountOfA) + A + B*(amountOfB) + B      ) / 256
    //
    // We can rearrange this formula for some great optimisations.
    //
    //   result = (  A*(amountOfA) + A + B*(amountOfB) + B      ) / 256
    //          = (  A*(255-amountOfB) + A + B*(amountOfB) + B  ) / 256
    //          = (  A*(256-amountOfB) + B*(amountOfB) + B      ) / 256
    //          = (  A*256 + B + B*(amountOfB) - A*(amountOfB)  ) / 256  // this is the version used in SCALE8_FIXED AVR below
    //          = (  A*256 + B + (B-A)*(amountOfB)              ) / 256  // this is the version used in SCALE8_FIXED C below

    uint16_t partial;
    uint8_t result;

    // 1 or 2 cycles depending on how the compiler optimises
    partial = (a << 8) | b;

    // 7 cycles
    asm volatile (
        "  mul %[a], %[amountOfB]        \n\t"
        "  sub %A[partial], r0           \n\t"
        "  sbc %B[partial], r1           \n\t"
        "  mul %[b], %[amountOfB]        \n\t"
        "  add %A[partial], r0           \n\t"
        "  adc %B[partial], r1           \n\t"
        "  clr __zero_reg__              \n\t"
        : [partial] "+r" (partial)
        : [amountOfB] "r" (amountOfB),
          [a] "r" (a),
          [b] "r" (b)
        : "r0", "r1"
    );
   
    result = partial >> 8;
   
    return result;
}