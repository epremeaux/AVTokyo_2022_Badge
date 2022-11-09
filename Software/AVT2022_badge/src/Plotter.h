#include <Arduino.h>
#include "AVT2022_badge_menu.h"

void drawTemperaturePlot(unsigned int encoderValue, RenderPressMode clicked);

void drawHumidityPlot(unsigned int encoderValue, RenderPressMode clicked);

void drawIMUPlot(unsigned int encoderValue, RenderPressMode clicked) ;