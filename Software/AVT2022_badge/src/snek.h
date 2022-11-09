#include <Arduino.h>
#include "AVT2022_badge_menu.h"

#define RIGHT_PIN 32 // 36
#define DOWN_PIN 33
#define UP_PIN 34
#define LEFT_PIN 35

void snek_game(unsigned int encoderValue, RenderPressMode clicked);
void exit_game();
void reset_game(int x, int y);
void change_direction();
void move();
bool wall_collision();
bool ate_itself();
void update_display();
void create_food();
void food_eaten();
void set_score_text();
void draw_score_text();
void update_score(uint16_t score);
void draw_max_score_text();
void update_max_score(uint16_t max_score);
