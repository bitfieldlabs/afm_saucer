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

uint8_t bitsSet(uint16_t v);
void hsv2rgb(uint8_t H, uint8_t S, uint8_t V, uint8_t *R, uint8_t *G, uint8_t *B);
uint8_t blend8( uint8_t a, uint8_t b, uint8_t amountOfB);