; vim: ft=z80 ts=2 sw=2 et:

  .code
  .export _drawbomb

; extern _tmsbuf

f0: .byte 0x80
f1: .byte 0x86
f2: .byte 0x8c
f3: .byte 0x92

;void drawbomb(char *p, uint8_t frame);
_drawbomb:
  pop   bc      ; return
  pop   hl      ; char *p in HL - Pointer into buffer for A position of bomb graphic
  pop   de      ; uint8_t frame in E - The frame number we are drawing.
  push  de
  push  hl
  push  bc

  push  hl      ; save pointer so we can use hl

#if 0
  or    e       ; are we on frame 0?
  jr    nz, notf0
  push  de      ; save frame
  ld    de,32   ; need to clear cells in row above
  ld    a,0x20  ; with a space
  sbc   hl,de   ; move pointer to row above
  ld    (hl),a  ; draw sapce
  inc   hl      ; increment pointer
  ld    (hl),a  ; draw space

  pop   de      ; restore frame
#endif

notf0:
  ld    hl,f0   ; get A characater for bomb graphic from the fx table above
  ld    d,0
  add   hl,de

  ld    a,(hl)  ; A now has the A character for the bomb graphic
  pop   hl      ; restore pointer

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
