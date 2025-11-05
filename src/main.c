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

#include "bombs.h"
#include "audio.h"

#include <cpm.h>
#include <io.h>
#include <joy.h>
#include <rand.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <tms.h>
#include <tms_patterns.h>
#include <ay-3-8910.h>
#include <ay-notes.h>


#define MAX_BOMBS 16

extern void drawbomb(char *p, uint8_t f);

typedef struct bomb_s {
  bool active;
  uint16_t yx;
} bomb_t;

static bomb_t bombs[MAX_BOMBS];
static bool shellactive = false;

static char g1colors[32];
static uint8_t i;
static uint8_t j;
static uint16_t I;
static char c;
static uint16_t score;
static uint8_t shellsleft;
static uint8_t maxbombs;
static uint8_t lvlctr;
static uint8_t ctr;
static uint8_t last; // Can't shoot twice in same position.
static uint8_t beatctr; // beat counter for music
static uint8_t beattmr; // beat timer for music

// variable to record the frame count
static uint8_t ff = 0;

static char tb[32];

void paint() {
  tms_wait();
  tms_g1flush(tms_buf);
  tms_flush_sprites();
}

void resetgame() {
  i = 0;
  j = 0;
  I = 0;
  c = 0;
  score = 0;
  shellsleft = 12;
  maxbombs = 4;
  lvlctr = 10;
  ctr = 0;
  ff = 0;
  last = 0;
  beatctr = 0;
  beattmr = 0;

  sprites[0].color = LIGHT_GREEN;
  sprites[0].pattern = 0;
  sprites[0].x = 128;
  sprites[0].y = 175;
  sprites[1].color = DARK_RED;
  sprites[1].pattern = 4;
  sprites[1].x = 128;
  sprites[1].y = 192;
  sprites[2].y = 0xD0;
  memset(bombs, 0, sizeof(bomb_t) * MAX_BOMBS);
  memset(tms_buf, 0x20, tms_n_tbl_len);
  paint();
  paint();
}

void vidsetup() {
  char *p = bomb;
  tms_init_g1(BLACK, DARK_YELLOW, true, false);

  // default font
  tms_load_pat(tms_patterns, 0x400);

  // load the bomb tile patterns
  tms_w_addr(tms_patt_tbl + (0x80 * 8));
  for (i = 0; i < 240; ++i) {
    tms_put(*p++);
  }

  // white tiles on black
  memset(g1colors, 0xF1, 32);

  // bomb tiles are gray
  g1colors[16] = 0xE1;
  g1colors[17] = 0xE1;
  g1colors[18] = 0xE1;
  tms_load_col(g1colors, 32);

  // load sprite patterns and set up SAT
  tms_load_spr(sprsheet, 64);
}

// This routine searches through the inactive bombs
// and finds one where the Y position is > 6.  This is to avoid bombs
// overlapping each other.
static uint8_t x;
void new_bomb(uint8_t idx) {
  bool fnd = true;
  while (fnd) {
    x = (fastrand() % 10) * 3 + 1;
    for (j = 0; j < maxbombs; ++j) {
      if (bombs[j].active && (bombs[j].yx & 0x1F) == x) {
        if ((bombs[j].yx >> 5) < 6)
          fnd = false;
      }
    }
    if (fnd) {
      bombs[idx].yx = 64+x;
      bombs[idx].active = true;
      return;
    }
  }
}

// add a new shell sprite if there is not one already active.
// Decrements the shells left counter.
bool do_shoot() {
  if (sprites[0].x == last)
    return true; // no shoot!
  if (!shellactive) {
    if (shellsleft > 0) {
      shellsleft--;
      shellactive = true;
      sprites[1].x = sprites[0].x;
      last = sprites[0].x;
      uitoa(shellsleft, tb);
      tms_puts_xy(1, 0, "SHELLS    ");
      tms_puts_xy(9, 0, tb);
 return true;
    } else {
      return false;
    }
  }
  return true;
}

// Moves the shells along their paths.  Hides those that pass through the
// top of the screen and deactivates them.
void animshells() {
  if (shellactive) {
    sprites[1].y -= 2;
    if (sprites[1].y < 16) {
      sprites[1].y = 192;
      shellactive = false;
      return;
    }
  }
}

// Deactivates the bomb - freeing up it's slot in memory.
// plots the blank bomb (frame 4)
void clear_bomb(uint8_t j) {
  bombs[j].active = false;
  drawbomb(tms_buf + bombs[j].yx - 32, 4);
}


// For every active shell, search through the active bombs
// If the coordinates of the shell overlap the tile position of any of the bombs
// then clear the bomb, increase the score and clear the shell.
void bombhit() {
  if (shellactive) {
    for (j = 0; j < maxbombs; ++j) {
      if (bombs[j].active) {
        if (((bombs[j].yx & 0x1F) << 3) == sprites[1].x) {
          if ((sprites[1].y > ((bombs[j].yx >> 5) * 8)) &&
              (sprites[1].y < ((bombs[j].yx >> 5) * 8) + 24)) {
            clear_bomb(j);
            shellactive = false;
            sprites[1].y = 192;
            lvlctr--;
            ay_play_note_delay(8, 2, 1000);
            if (lvlctr == 0) {
              if (maxbombs < MAX_BOMBS)
                maxbombs +=2;
              shellsleft += 11;
              lvlctr = 10;
            }
            // only need to update the score if we actually hit something.
            score++;
            uitoa(score, tb);
            tms_puts_xy(15, 0, "SCORE       ");
            tms_puts_xy(22, 0, tb);
          }
        }
      }
    }
  }
}

// For every active shell, check if the X position of the shell is aligned with
// the player and that the y position overlaps the bottom of the screen.
void delay(uint8_t d) {
  while (d-- > 0)
    tms_wait();
}

bool plyrhit() {
  for (j = 0; j < maxbombs; ++j) {
    if (bombs[j].active) {
      if ((bombs[j].yx >> 5) > 21) {
        if (((bombs[j].yx & 0x1F)<<3) == sprites[0].x) {
          ay_write(AY_VOLUME_A, 0x00);
          ay_write(AY_VOLUME_C, 0x1F);
          ay_write(AY_PERIOD_NOISE, 0x1F);
          ay_write(AY_ENVELOPE_F, 0x00);
          ay_write(AY_ENVELOPE_C, 0x20);
          ay_write(AY_ENVELOPE_SHAPE, AY_ENV_SHAPE_FADE_OUT);
          delay(90);

          return true;
        }
      }
    }
  }
  return false;
}

void gameloop() {
  while (1) {

#if 0
    c = cpm_dc_in();
    switch (c) {
    case 0x1b:
      return;
      break;
    case ',':
      if (sprites[0].x > 8)
        sprites[0].x -= 24;
      break;
    case '.':
      if (sprites[0].x < 224)
        sprites[0].x += 24;
      break;
    case ' ':
      if (!do_shoot())
        return;
      break;
    }
#else
    c = cpm_dc_in();
    if (c == 0x1b) return;

    c = joy(0);
    if ((c & JOY_MAP_BUTTON) == 0) {
      if (!do_shoot())
        return;
    }
    if (ctr == 6) {
      ctr = 0;
      if ((c & JOY_MAP_LEFT) == 0) {
        if (sprites[0].x > 8)
          sprites[0].x -= 24;
      } else if ((c & JOY_MAP_RIGHT) == 0) {
        if (sprites[0].x < 224)
          sprites[0].x += 24;
      }
    }
#endif
    // we only check for bomb and player collisions at frame 0
    if (ff == 0) {
      if (plyrhit()) return;
    }

    // do bomb collision at frame 1
    if (ff == 1) bombhit();

    // Bomb animation
    for (i = 0; i < maxbombs; ++i) {

      // At frame zero we need to paint the two cells above the bomb black.
      if (bombs[i].active) {
        if (ff == 0) {
          tms_buf[bombs[i].yx - 32] = ' ';
          tms_buf[bombs[i].yx - 31] = ' ';
        }
        drawbomb(tms_buf + bombs[i].yx, ff);
      }
    }

    // when we have finished painting frame 3 of the bombs, we need to move them
    // all down one cell.  Here is where we check if the bombs have passed off
    // the bottom of the screen. We also create new bombs for all inactive ones.
    if (ff == 3) {
      for (i = 0; i < maxbombs; ++i) {
        if (bombs[i].active) {
          bombs[i].yx += 32;
          if ((bombs[i].yx >> 5) > 24) {
            bombs[i].active = false;
          }
        } else {
          // if the bomb is inactive, then decide to make a new one.
          if ((fastrand() & 0xFF) < 0x80) {
            new_bomb(i);
          }
        }
      }
    }

    // indicate that the player can not shoot in this position.
    if (sprites[0].x == last)
      sprites[0].color = LIGHT_RED;
    else
      sprites[0].color = LIGHT_GREEN;

    animshells();

    // we roll the frame back to 0 at 4
    ff++;
    ff &= 3;
    // paint right at the end.  Will wait for next vdp interrupt
    ctr++;
    // Music
    beattmr ++;
    beattmr &= 15;
    if (beattmr == 0) {
      ay_play_note_delay(music[beatctr], 0, 0);
      beatctr ++;
      beatctr &= 7;
    }
    paint();
  }
}

void center(uint8_t y, char *txt) {
  uint8_t x = 15 - (strlen(txt) / 2);
  tms_puts_xy(x, y, txt);
}


bool menu() {
  memset(tms_buf + 32, 0x20, tms_n_tbl_len - 32);
  center(8, "Bombs Away");
  center(10, "By productiondave");
  center(11, "(c) 2025");
  center(13, "v 1.0.1");
  center(23, "Press button to play");
  sprites[0].y = 192;
  paint();

  while (1) {
#if 0
    if (c > 0) {
      if (c == 0x1b) {
        return false;
      } else {
        return true;
      }
    }
#else
      c = cpm_dc_in();
      if (c == 0x1b) return false;
      c = joy(0);
      if ((c & JOY_MAP_BUTTON) == 0) {
        delay(30);
        return true;
      }
#endif
  }
}

// Main entry function of the game.
void main() {
  bool play = true;
  // Init the display and tiles etc.
  vidsetup();
  ay_write(AY_MIXER, 0x5A);
  ay_write(AY_VOLUME_A, 0x00);
  ay_write(AY_VOLUME_C, 0x1F);
  ay_write(AY_PERIOD_NOISE, 0x1F);
  resetgame();

  // main game loop
  while (play) {
    play = menu();
    if (!play) break;
    resetgame();
    paint();
    ay_write(AY_VOLUME_A, 0x07);
    ay_write(AY_PERIOD_NOISE, 0x04);
    gameloop();
    ay_write(AY_VOLUME_A, 0x00);
    ay_write(AY_VOLUME_C, 0x00);
  }
  resetgame();
  sprites[0].y = 192;
  center(11, "Goodbye");
  paint();
}
