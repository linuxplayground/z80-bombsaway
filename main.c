#include "bombs.h"
#include <cpm.h>
#include <io.h>
#include <joy.h>
#include <rand.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <tms.h>
#include <tms_patterns.h>

#define MAX_BOMBS 12

extern void drawbomb(char *p, uint8_t f);

typedef struct bomb_s {
  bool active;
  uint8_t x;
  uint8_t y;
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
  sprites[0].color = LIGHT_GREEN;
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

// calls to an assembly routine (drawbomb.S) to plot the 6 tiles for the
// bomb into the framebuffer.
// The frame is used as an offset to select which patterns to plot.
// Frame 4 is a set of 6 blank patterns used for removing a bomb when hit.
void plotbomb(uint8_t x, uint8_t y, uint8_t f) {
  drawbomb(tms_buf + y * 32 + x, f);
}

// This rouitne searches through the inactive bombs
// and finds one where the Y position is > 6.  This is to avoid bombs
// overlapping each other.
static uint8_t x;
void new_bomb(uint8_t idx) {
  bool fnd = true;
  while (fnd) {
    x = (rand() % 10) * 3 + 1;
    for (j = 0; j < maxbombs; ++j) {
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

// add a new shell sprite if there is not one already active.
// Decrements teh shellsleft counter.
bool do_shoot() {
  if (sprites[0].x == last)
    return true; // no shoot!
  if (!shellactive) {
    if (shellsleft > 0) {
      shellsleft--;
      shellactive = true;
      sprites[1].x = sprites[0].x;
      last = sprites[0].x;
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
    if (sprites[1].y < 8) {
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
  drawbomb(tms_buf + (bombs[j].y - 1) * 32 + bombs[j].x, 4);
}

// For every active shell, search through the active bombs
// If the coordinates of the shell overlap the tile position of any of the bombs
// then clear the bomb, increase the socre and clear the shell.
void bombhit() {
  if (shellactive) {
    for (j = 0; j < maxbombs; ++j) {
      if (bombs[j].active) {
        if (bombs[j].x * 8 == sprites[1].x) {
          if ((sprites[1].y > bombs[j].y * 8) &&
              (sprites[1].y < bombs[j].y * 8 + 24)) {
            clear_bomb(j);
            shellactive = false;
            sprites[1].y = 192;
            score++;
            lvlctr--;
            if (lvlctr == 0) {
              maxbombs += 2;
              if (maxbombs > MAX_BOMBS)
                maxbombs = MAX_BOMBS;
              shellsleft += 11;
              lvlctr = 10;
            }
          }
        }
      }
    }
  }
}

// For every active shell, check if the X position of the shell is aligned with
// the player and that the y position overlapps the bottom of the screen.
bool plyrhit() {
  for (j = 0; j < maxbombs; ++j) {
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

void gameloop() {
  while (1) {

    if (cpm_rawio() == 0x1b)
      return;

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

    // we only check for bomb and player collisions at frame 0
    if (ff == 0) {
      bombhit();
      if (plyrhit())
        return;
    }

    // Bomb animation
    for (i = 0; i < maxbombs; ++i) {

      // At frame zero we need to paint the two cells above the bomb black.
      if (bombs[i].active) {
        if (ff == 0) {
          tms_put_char(bombs[i].x, bombs[i].y - 1, ' ');
          tms_put_char(bombs[i].x + 1, bombs[i].y - 1, ' ');
        }
        plotbomb(bombs[i].x, bombs[i].y, ff);
      }
    }

    // when we have finished painting frame 3 of the bombs, we need to move them
    // all down one cell.  Here is where we check if the bombs have passed off
    // the bottom of the screen. We also create new bombs for all inactive ones.
    if (ff == 3) {
      for (i = 0; i < maxbombs; ++i) {
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

    if (sprites[0].x == last)
      sprites[0].color = LIGHT_RED;
    else
      sprites[0].color = LIGHT_GREEN;

    uitoa(shellsleft, tb);
    tms_puts_xy(1, 0, "SHELLS       ");
    tms_puts_xy(9, 0, tb);
    uitoa(score, tb);
    tms_puts_xy(15, 0, "SCORE       ");
    tms_puts_xy(22, 0, tb);
    animshells();

    // we roll the frame back to 0 at 4
    ff++;
    if (ff > 3)
      ff = 0;
    // paint right at the end.  Will wait for next vdp interrupt
    ctr++;
    paint();
  }
}

  void center(uint8_t y, char *txt) {
    uint8_t x = 15 - (strlen(txt) / 2);
    tms_puts_xy(x, y, txt);
  }

  void delay(uint8_t d) {
    while (d-- > 0)
      tms_wait();
  }

  bool menu() {
    memset(tms_buf + 32, 0x20, tms_n_tbl_len - 32);
    center(8, "Bombs Away");
    center(10, "By productiondave");
    center(11, "(c) 2025");
    center(13, "v 1.0.0");
    center(23, "Press button to play");
    sprites[0].y = 192;
    paint();

    while (1) {
      c = cpm_rawio();
      if (c > 0) {
        if (c == 0x1b) {
          return false;
        }
      }

      c = joy(0);
      if ((c & JOY_MAP_BUTTON) == 0) {
        delay(30);
        return true;
      }
    }
  }

  // Main entry function of the game.
  void main() {
    bool play = true;
    // Init the display and tiles etc.
    vidsetup();
    resetgame();

    // main game loop
    while (play) {
      play = menu();
      resetgame();
      if (play) {
        paint();
        gameloop();
      }
    }
    resetgame();
    sprites[0].y = 192;
    center(11, "Goodbye");
    paint();
  }
