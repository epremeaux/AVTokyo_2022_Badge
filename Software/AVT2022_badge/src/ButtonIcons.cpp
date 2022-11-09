#include "ButtonIcons.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"

#define RIGHT_PIN   32      //36
#define DOWN_PIN    33
#define UP_PIN      34
#define LEFT_PIN    35
#define SELECT_PIN  39
#define A_PIN       25      //A button
#define B_PIN       26
#define USR_LED     14


TFT_eSprite joypad =TFT_eSprite(&gfx);
TFT_eSprite A_btn =TFT_eSprite(&gfx);
TFT_eSprite B_btn =TFT_eSprite(&gfx);

void draw_A(int32_t posx, int32_t posy, bool pressed) {
    A_btn.setColorDepth(8);
    A_btn.createSprite(40, 40);
    if (pressed == true) {
        A_btn.pushImage(0, 0, 40, 40, A_button_pressed_4040);
    } else {
        A_btn.pushImage(0, 0, 40, 40, A_button_4040);
    }
    A_btn.pushSprite(posx, posy);
}

void draw_B(int32_t posx, int32_t posy, bool pressed) {
    B_btn.setColorDepth(8);
    B_btn.createSprite(40, 40);
    if (pressed == true) {
        B_btn.pushImage(0, 0, 40, 40, B_button_pressed_4040);
    } else {
        B_btn.pushImage(0, 0, 40, 40, B_button_4040);
    }
    B_btn.pushSprite(posx, posy);
}

void draw_Joy(int32_t posx, int32_t posy, int dir) {
    //0: nothing, 1: up, 2: down, 3: left, 4: right, 5: clicked, 6: Up/Down, 7: Left/Right

    joypad.setColorDepth(8);
    joypad.createSprite(76, 76);

/*
    joypad.setColorDepth(4);
    joypad.createSprite(81, 81);
  
    uint16_t cmap[16];
    cmap[0] = TFT_BLACK;
    cmap[1] = TFT_NAVY;
    cmap[2] = TFT_DARKGREEN;
    cmap[3] = TFT_DARKCYAN;
    cmap[4] = TFT_MAROON;
    cmap[5] = TFT_PURPLE;
    cmap[6] = TFT_OLIVE;
    cmap[7] = TFT_LIGHTGREY;
    cmap[8] = TFT_DARKGREY;
    cmap[9] = TFT_BLUE;             // TFT_BLUE;
    cmap[10] = TFT_GREEN;
    cmap[11] = TFT_CYAN;
    cmap[12] = TFT_RED;
    cmap[13] = TFT_MAGENTA;
    cmap[14] = TFT_YELLOW;
    cmap[15] = TFT_WHITE;

    joypad.createPalette(cmap, 16);
   */
    joypad.pushImage(0, 0, 76, 76, Joy_7676);
   // joypad.pushImage(0, 0, 81, 81, Joypad_8181);

    // select case draw magenta triangles over the top of the image, or center circle
    switch(dir) {
        case 1:     // up
            //joypad.drawTriangle(37, 4, 32, 11, 42, 11, TFT_MAGENTA);
            joypad.fillTriangle(37, 0, 27, 14, 48, 14, TFT_MAGENTA);
            break;
        case 2:     // down
            joypad.fillTriangle(37, 75, 48, 61, 27, 61, TFT_MAGENTA);
            break;
        case 3:     // left
            joypad.fillTriangle(0, 37, 14, 48, 14, 28, TFT_MAGENTA);
            break;
        case 4:     // right
            joypad.fillTriangle(75, 37, 61, 27, 61, 48, TFT_MAGENTA);
            break;
        case 5:     // center click
            joypad.fillCircle(38, 38, 10, TFT_MAGENTA);
            break;
        case 6:     // up and down
            joypad.fillTriangle(37, 0, 27, 14, 48, 14, TFT_MAGENTA);
            joypad.fillTriangle(37, 75, 48, 61, 27, 61, TFT_MAGENTA);
            break;
        case 7:     // left and right
            joypad.fillTriangle(0, 37, 14, 48, 14, 28, TFT_MAGENTA);
            joypad.fillTriangle(75, 37, 61, 27, 61, 48, TFT_MAGENTA);
            break;
        default:
            // clean joypad image
            break;
    }
    joypad.pushSprite(posx, posy);
}


/*******************  BUTTON TESTER  ********************************/

void ButtonTester(unsigned int encoderValue , RenderPressMode clicked) {
    String message;
    
    if(Global::TODcounter == 0){
        Global::TODcounter++;
        gfx.fillScreen(TFT_BLACK);
        gfx.setTextSize(1);
        gfx.setTextColor(TFT_WHITE);
        gfx.setCursor(0, 5);
        gfx.println(F("Button Tester"));
        gfx.println();

       draw_Joy(10, 80, 0);
       draw_B(120, 120, false);
       draw_A(200, 80, false);
    }
    
    if (clicked) {
    Global::TODcounter = 0;
    draw_Joy(10, 80, 5);
    delay(2000);
    renderer.giveBackDisplay();
    }
    else {
        message = "";
        draw_Joy(10, 80, 0);
        if (switches.isSwitchPressed(UP_PIN)) {message = "UP"; draw_Joy(10, 80, 1);}
        else if (switches.isSwitchPressed(DOWN_PIN)) {message = "DOWN"; draw_Joy(10, 80, 2);}
        
        if (switches.isSwitchPressed(LEFT_PIN)) {message = message.isEmpty() ? message + "LEFT" : message + "+LEFT"; draw_Joy(10, 80, 3);}
        else if (switches.isSwitchPressed(RIGHT_PIN)) {message = message.isEmpty() ? message + "RIGHT" : message + "+RIGHT"; draw_Joy(10, 80, 4);}
        
        if (switches.isSwitchPressed(A_PIN)) {
            message = message.isEmpty() ? message + "A" : message + "+A";
            draw_A(200, 80, true);
            } else {
                draw_A(200, 80, false);
            }
        if (switches.isSwitchPressed(B_PIN)) {
            message = message.isEmpty() ? message + "B" : message + "+B";
            draw_B(120, 120, true);
            } else {
                draw_B(120, 120, false);
            }
        Serial.println(message);
        // Clear the line by painting a black filled rectangle on it
        gfx.fillRect(0,35,280,30,TFT_BLACK);
        gfx.setCursor(0,35);
        gfx.println(message);
    }
}