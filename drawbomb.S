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
