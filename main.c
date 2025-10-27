#include <cpm.h>
#include <rand.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tms.h>
#include <tms_patterns.h>

#include "bombs.h"

#define MAX_BOMBS 6
#define MAX_SHELLS 3

extern void drawbomb(char *p, uint8_t f);

typedef struct bomb_s {
  bool active;
  uint8_t x;
  uint8_t y;
} bomb_t;

static bomb_t bombs[MAX_BOMBS];

static bool shells[MAX_SHELLS];

static char g1colors[32];
static uint8_t i = 0;
static uint8_t j = 0;
static uint16_t I = 0;
static uint8_t frame = 0;
static char c = 0;
static uint16_t score = 0;

static char tb[32];

void vidsetup() {
  tms_init_g1(BLACK, DARK_YELLOW, true, false);
  tms_load_pat(tms_patterns, 0x400);
  memset(g1colors, 0xC1, 32);
  g1colors[16] = 0xE1;
  g1colors[17] = 0xE1;
  g1colors[18] = 0x61;
  tms_load_col(g1colors, 32);
  tms_load_spr(sprsheet, 64);
  sprites[0].color = DARK_GREEN;
  sprites[0].pattern = 0;
  sprites[0].x = 128;
  sprites[0].y = 175;
  sprites[1].color = DARK_RED;
  sprites[1].pattern = 4;
  sprites[1].x = 128;
  sprites[1].y = 192;
  sprites[2].color = DARK_RED;
  sprites[2].pattern = 4;
  sprites[2].x = 128;
  sprites[2].y = 192;
  sprites[3].color = DARK_RED;
  sprites[3].pattern = 4;
  sprites[3].x = 128;
  sprites[3].y = 192;
  sprites[4].y = 0xD0;
}

void paint() {
  tms_wait();
  tms_g1flush(tms_buf);
  tms_flush_sprites();
}

// Loads the bomb patterns into name table positions: 0x80 - 0x97
void ldbomtiles() {
  char *p = bomb;
  uint8_t i;
  tms_w_addr(tms_patt_tbl + (0x80 * 8));
  for (i = 0; i < 240; ++i) {
    tms_put(*p++);
  }
}

void plotbomb(uint8_t x, uint8_t y, uint8_t f) {
  drawbomb(tms_buf + y * 32 + x, f);
}

static uint8_t x;
void new_bomb(uint8_t idx) {
  bool fnd = true;
  while (fnd) {
    x = (rand() % 10) * 3 + 1;
    for (j = 0; j < MAX_BOMBS; ++j) {
      if (bombs[j].active && bombs[j].x == x) {
        if (bombs[j].y < 6)
          fnd = false;
      }
    }
    if (fnd) {
      bombs[idx].x = x;
      bombs[idx].y = 0;
      bombs[idx].active = true;
      return;
    }
  }
}

void do_shoot() {
  for (i = 0; i < MAX_SHELLS; ++i) {
    if (!shells[i]) {
      shells[i] = true;
      sprites[i + 1].x = sprites[0].x;
      return;
    }
  }
}

void animshells() {
  for (i = 0; i < MAX_SHELLS; ++i) {
    if (shells[i]) {
      sprites[i + 1].y -= 2;
      if (sprites[i + 1].y < 8) {
        sprites[i + 1].y = 192;
        shells[i] = false;
        return;
      }
    }
  }
}

void clear_bomb(uint8_t j) {
  bombs[j].active = false;
  drawbomb(tms_buf + bombs[j].y * 32 + bombs[j].x, 4);
}

void bombhit() {
  for (i = 0; i < MAX_SHELLS; ++i) {
    if (shells[i]) {
      for (j = 0; j < MAX_BOMBS; ++j) {
        if (bombs[j].active) {
          if (bombs[j].x * 8 == sprites[i + 1].x) {
            if ((sprites[i + 1].y > bombs[j].y * 8) &&
                (sprites[i + 1].y < bombs[j].y * 8 + 24)) {
              clear_bomb(j);
              shells[i] = false;
              sprites[i + 1].y = 192;
              score++;
            }
          }
        }
      }
    }
  }
}

bool plyrhit() {
  for (j = 0; j < MAX_BOMBS; ++j) {
    if (bombs[j].active) {
      if (bombs[j].x * 8 == sprites[0].x) {
        if (bombs[j].y > 21) {
          return true;
        }
      }
    }
  }
  return false;
}

static uint8_t ff = 0;

bool gameover = false;
void main() {
  vidsetup();
  ldbomtiles();

  memset(bombs, 0, sizeof(bomb_t) * MAX_BOMBS);
  memset(shells, 0, MAX_SHELLS);

  do {
    ff = frame & 3;

    if (ff == 0) {
      bombhit();
      gameover = plyrhit();
    }

    for (i = 0; i < MAX_BOMBS; ++i) {
      if (ff == 0) {
        tms_put_char(bombs[i].x, bombs[i].y - 1, ' ');
        tms_put_char(bombs[i].x + 1, bombs[i].y - 1, ' ');
      }
      if (bombs[i].active) {
        plotbomb(bombs[i].x, bombs[i].y, ff);
      }
    }

    if (ff == 3) {
      for (i = 0; i < MAX_BOMBS; ++i) {
        if (bombs[i].active) {
          bombs[i].y++;
          if (bombs[i].y > 24) {
            bombs[i].active = false;
          }
        } else {
          if ((rand() & 0xFF) < 0x10) {
            new_bomb(i);
          }
        }
      }
    }

    c = cpm_rawio();
    switch (c) {
    case ',':
      if (sprites[0].x > 8)
        sprites[0].x -= 24;
      break;
    case '.':
      if (sprites[0].x < 224)
        sprites[0].x += 24;
      break;
    case ' ':
      do_shoot();
      break;
    }
    if (frame++ > 251)
      frame = 0;
    animshells();
    paint();
  } while (c != 0x1b && !gameover);
  sprintf(tb, "You scored: %d\r\n", score);
  tms_puts_xy(4, 11, tb);
  paint();
}
