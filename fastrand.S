  .code
  .export _fastrand

_rdata: .word 0x6969

_fastrand:
  ld    hl,(_rdata)
  ld    a,r
  ld    d,a
  ld    e,(hl)
  add   hl,de
  add   a,l
  xor   h
  ld    (_rdata),hl
  ret
