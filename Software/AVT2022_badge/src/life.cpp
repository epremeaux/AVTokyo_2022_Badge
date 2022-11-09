

//The Game of Life, also known simply as Life, is a cellular automaton
//devised by the British mathematician John Horton Conway in 1970.
// https://en.wikipedia.org/wiki/Conway's_Game_of_Life

// See license at end of file.

// Adapted by Bodmer
// Modified for tcMenu By DIYSciBorg

#include "life.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"

/* LIFE game does NEED high frame performance, so we use the frame counter
 * (TODcounter). This counter also warns the MQTT loop not to post.
 * Since we are useing it, MQTT will NOT post while playing LIFE.
 * However we should reset TODcounter to 0 on exit, so MQTT will resume.
 */



// Maximum number of generations until the screen is refreshed
#define MAX_GEN_COUNT 500

// Frames per second
#define FPS 20

// grid size is related to screen size.
// screen is 240x280 (portrait)
// or 280x240 (landscape)

// The ESP8266 has plenty of memory so we can create a large array
// 2 x 2 pixel cells, array size = 5120 bytes per array, runs fast
#define GRIDX 140    // 80
#define GRIDY 120    // 64  
#define CELLXY 2

// 1 x 1 pixel cells, array size = 20480 bytes per array
//#define GRIDX 280   // 160
//#define GRIDY 240   // 128
//#define CELLXY 1

//Current grid and newgrid arrays are needed
uint8_t grid[GRIDX][GRIDY];

//The new grid for the next generation
uint8_t newgrid[GRIDX][GRIDY];


void create_life(unsigned int encoderValue, RenderPressMode clicked) {
  // we initialise the display on the first call.
  if(Global::TODcounter == 0) {
    // Update the refresh speed of the screen
    renderer.setUpdatesPerSecond(FPS);
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_WHITE);
    gfx.setCursor(0, 0);
  
    //Display a simple splash screen
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_WHITE);
    gfx.setCursor(40, 5);
    gfx.println(F("Arduino"));
    gfx.setCursor(35, 25);
    gfx.println(F("Cellular"));
    gfx.setCursor(35, 45);
    gfx.println(F("Automata"));
    //gfx.setTextSize(1);
    gfx.setCursor(35, 75);
    gfx.println(F("Game of Life is"));
    gfx.setCursor(50, 95);
    gfx.println(F("NOT a game!"));

    delay(2000);

    gfx.fillScreen(TFT_BLACK);

    initGrid();

    drawGrid();
  }

  Serial.printf("loop : Select button status : %d\n",  clicked);
  // Give back display if user clicks the select button
  if (clicked) {
    renderer.giveBackDisplay();
    Global::TODcounter = 0;
  }
  // Otherwise compute generations
  else {
    computeCA();
    drawGrid();
    Global::TODcounter++;
    for (int16_t x = 1; x < GRIDX-1; x++) {
      for (int16_t y = 1; y < GRIDY-1; y++) {
        grid[x][y] = newgrid[x][y];
      }
    }
    // If we reach maximum generation, reset to the beginning
    if(Global::TODcounter == MAX_GEN_COUNT) {
      delay(2000);
      Global::TODcounter = 0;
    }
  }
}

// Check the Moore neighbourhood
int getNumberOfNeighbors(int x, int y) {
  return grid[x - 1][y] + grid[x - 1][y - 1] + grid[x][y - 1] + grid[x + 1][y - 1] + grid[x + 1][y] + grid[x + 1][y + 1] + grid[x][y + 1] + grid[x - 1][y + 1];
}

//Draws the grid on the display
void drawGrid(void) {
  uint16_t color = TFT_WHITE;
  for (int16_t x = 1; x < GRIDX - 1; x++) {
    for (int16_t y = 1; y < GRIDY - 1; y++) {
      if ((grid[x][y]) != (newgrid[x][y])) {
        if (newgrid[x][y] == 1) color = 0xFFFF; //random(0xFFFF);
        else color = 0;
        gfx.fillRect(CELLXY * x, CELLXY * y, CELLXY, CELLXY, color);
      }
    }
  }
}

//Initialise Grid
void initGrid(void) {
  for (int16_t x = 0; x < GRIDX; x++) {
    for (int16_t y = 0; y < GRIDY; y++) {
      newgrid[x][y] = 0;

      if (x == 0 || x == GRIDX - 1 || y == 0 || y == GRIDY - 1) {
        grid[x][y] = 0;
      }
      else {
        if (random(3) == 1)
          grid[x][y] = 1;
        else
          grid[x][y] = 0;
      }

    }
  }
}

//Compute the CA. Basically everything related to CA starts here
void computeCA() {
  for (int16_t x = 1; x < GRIDX; x++) {
    for (int16_t y = 1; y < GRIDY; y++) {
      int neighbors = getNumberOfNeighbors(x, y);
      if (grid[x][y] == 1 && (neighbors == 2 || neighbors == 3 ))
      {
        newgrid[x][y] = 1;
      }
      else if (grid[x][y] == 1)  newgrid[x][y] = 0;
      if (grid[x][y] == 0 && (neighbors == 3))
      {
        newgrid[x][y] = 1;
      }
      else if (grid[x][y] == 0) newgrid[x][y] = 0;
    }
  }
}

/*
   The MIT License (MIT)

   Copyright (c) 2016 RuntimeProjects.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
