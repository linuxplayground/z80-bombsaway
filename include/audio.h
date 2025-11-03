/*  Z80-Bombs away game
 *  Copyright (C) 2025 David Latham

 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.

 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
*/

#ifndef _AUDIO_H
#define _AUDIO_H

#include <stdint.h>

/*
 * Melody
 * C4, D#4, F#4, G4, C5, G4, F#4, D#4
*/

uint8_t music[8] = {
  40, 43, 46, 47, 52, 47, 46, 43
};

#endif //_AUDIO_H
