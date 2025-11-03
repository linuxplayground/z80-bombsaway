;  Z80-Bombs away game
;  Copyright (C) 2025 David Latham
;
;  This library is free software; you can redistribute it and/or
;  modify it under the terms of the GNU Lesser General Public
;  License as published by the Free Software Foundation; either
;  version 2.1 of the License, or (at your option) any later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;  Lesser General Public License for more details.
;
;  You should have received a copy of the GNU Lesser General Public
;  License along with this library; if not, write to the Free Software
;  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
;  USA
;

; vim: ft=z80 ts=2 sw=2 et:

  .code
  .export _drawbomb

; extern _tmsbuf

;void drawbomb(char *p, uint8_t frame);
_drawbomb:
  pop   bc      ; return
  pop   hl      ; char *p in HL - Pointer into buffer for A position of bomb graphic
  pop   de      ; uint8_t frame in E - The frame number we are drawing.
  push  de
  push  hl
  push  bc

  ld    a,e
  add   a,a
  ld    c,a
  add   a,a
  add   a,c
  add   a,0x80

  ld    de,31
  ld    (hl),a  ; A character
  inc   a
  inc   hl
  ld    (hl),a  ; B
  inc   a
  add   hl,de
  ld    (hl),a  ; C
  inc   a
  inc   hl
  ld    (hl),a  ; D
  inc   a
  add   hl,de
  ld    (hl),a  ; E
  inc   a
  inc   hl
  ld    (hl),a  ; F

  ret
