#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <tms.h>
#include <tms_patterns.h>
#include <stdio.h>
#include <cpm.h>
#include <rand.h>

#include "bombs.h"

#define MAX_BOMBS 10

extern void drawbomb(char *p, uint8_t f);

typedef struct bomb_s {
  bool active;
  uint8_t x;
  uint8_t y;
} bomb_t;

static bomb_t bombs[MAX_BOMBS];

static char g1colors[32];
static uint8_t i = 0;
static uint8_t j = 0;
static uint16_t I = 0;
static uint8_t frame = 0;
static char c = 0;

void vidsetup()
{
  tms_init_g1(BLACK, DARK_YELLOW, true, false);
  tms_load_pat(tms_patterns, 0x400);
  memset(g1colors, 0x71, 32);
  tms_load_col(g1colors, 32);
}

void paint()
{
  tms_wait();
  tms_g1flush(tms_buf);
}

// Loads the bomb patterns into name table positions: 0x80 - 0x97
void ldbomtiles() {
  char *p = bomb;
  uint8_t i;
  tms_w_addr(tms_patt_tbl + (0x80*8));
  for (i=0; i<192; ++i)
  {
    tms_put(*p++);
  }
}

void plotbomb(uint8_t x, uint8_t y, uint8_t f)
{
  drawbomb(tms_buf + y*32 + x, f);
}

static uint8_t x;
void new_bomb(uint8_t idx)
{
  bool fnd = true;
  while (fnd) {
    x = (rand() % 10) * 3 + 1;
    for (j=0; j<MAX_BOMBS; ++j) {
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

static uint8_t ff = 0;
void main()
{
  vidsetup();
  ldbomtiles();
  memset(bombs, 0, sizeof(bomb_t) * MAX_BOMBS);
  do {
    ff = frame & 3;

    for (i=0; i<MAX_BOMBS; ++i)
    {
      if (ff == 0) {
        tms_put_char(bombs[i].x, bombs[i].y-1, ' ');
        tms_put_char(bombs[i].x+1, bombs[i].y-1, ' ');
      }
      if (bombs[i].active) {
        plotbomb(bombs[i].x, bombs[i].y, ff);
      }
    }

    if (ff == 3) {
      for (i=0; i<MAX_BOMBS; ++i) {
        if (bombs[i].active) {
          bombs[i].y ++;
          if (bombs[i].y > 24) {
            bombs[i].active = false;
          }
        } else {
          if ( (rand() & 0xFF) < 0x10) {
            new_bomb(i);
          }
        }
      }
    }

    c = cpm_rawio();
    if (frame ++ > 252) frame = 0;
    paint();
  } while (c != 0x1b);
}
