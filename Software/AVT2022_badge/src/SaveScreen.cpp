#include "SaveScreen.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"


void SavedScreen(unsigned int encoderValue, RenderPressMode clicked) {
    // At this point clicked is the status of the select button
    // it can be RPRESS_NONE, RPRESS_PRESSED or RPRESS_HELD
    // encoderValue is the current value of the rotary encoder

   if (Global::TODcounter == 0) {
        Global::TODcounter++;
        gfx.fillScreen(TFT_BLACK);
        gfx.setTextSize(1);
        gfx.setTextColor(TFT_WHITE);
        gfx.setCursor(50, 10);
        gfx.println("Settings saved");
    }
    delay(1000);
    Global::TODcounter = 0;
    renderer.giveBackDisplay();
}

