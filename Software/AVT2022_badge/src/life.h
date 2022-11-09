#include <Arduino.h>
#include "AVT2022_badge_menu.h"


void create_life(unsigned int encoderValue, RenderPressMode clicked) ;
int getNumberOfNeighbors(int x, int y);
void drawGrid(void);
void initGrid(void) ;
void computeCA();
