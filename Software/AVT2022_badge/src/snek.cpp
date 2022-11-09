
#include "snek.h"
#include "Preferences.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"

// Frames per second
#define FPS 4

/* SNEK game does NEED high frame performance, so we use the frame counter
 * (TODcounter). This counter also warns the MQTT loop not to post.
 * Since we are useing it, MQTT will NOT post while playing Snek.
 * However we should reset TODcounter to 0 on exit, so MQTT will resume.
 */

// Screen is 280x240 in landscape
// We need 20 pixels from each side to account for rounded corners
// Here we calculate all play area related constants
#define OFFSET 24
#define WIDTH (280 - (2 * OFFSET))
#define HEIGHT (240 - (2 * OFFSET))
#define BLOCKXY 8
#define BLOCK_X (WIDTH / BLOCKXY)
#define BLOCK_Y (HEIGHT / BLOCKXY)
#define MAX_SNEK_LENGTH (BLOCK_X * BLOCK_Y)

// Snek variables
uint snek[MAX_SNEK_LENGTH][2];
enum direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} dir;
uint16_t snek_length;
uint16_t max_score;
uint food[2];
bool ate_food;
uint free_locations[MAX_SNEK_LENGTH - 1][2]; // Had to add this here, otherwise I had memory issues

//Global::preferences;

void snek_game(unsigned int encoderValue, RenderPressMode clicked)
{
    // We display the splash screen first
    if (Global::TODcounter == 0)
    {
        // Update the refresh speed of the screen
        renderer.setUpdatesPerSecond(FPS);

        // Display a simple splash screen
        gfx.fillScreen(TFT_BLACK);
        gfx.setTextSize(3);
        gfx.setTextColor(TFT_GREEN);
        gfx.setTextDatum(MC_DATUM);
        gfx.drawString("SNEK", 140, 120);

        delay(2000);
        Global::TODcounter++;
    }
    // Then we initialize the game screen
    else if (Global::TODcounter == 1)
    {
        gfx.fillScreen(TFT_BLACK);

        // Get maximum score
        Global::preferences.begin("Snek", false);
        max_score = Global::preferences.getInt("Score", 0);
        Global::preferences.end();

        // Draw scoreboard
        set_score_text();
        draw_score_text();
        update_score(0);
        draw_max_score_text();
        update_max_score(max_score);

        // Draw play area
        gfx.drawRect(OFFSET - 1, OFFSET - 1, WIDTH + 2, HEIGHT + 2, TFT_WHITE);

        // Draw the snek
        reset_game(BLOCK_X / 2, BLOCK_Y / 2);
        update_display();
        create_food();

        Global::TODcounter++;
    }
    // Play the game
    else if (Global::TODcounter == 2)
    {
        // Give back display if user clicks the select button
        if (clicked)
        {
            Global::TODcounter++;
        }
        // Otherwise continue the game
        else
        {
            change_direction();
            move();
            // If we collide with a wall or ourselves, it's game over
            if (wall_collision() || ate_itself())
            {
                Global::TODcounter++;
            }
            // If we reach the maximum length, we won the game
            else if (snek_length >= MAX_SNEK_LENGTH)
            {
                Global::TODcounter++;
            }
            else
            {
                food_eaten();
                update_display();
            }
        }
    }
    // To display the game over screen
    else if (Global::TODcounter == 3)
    {
        // Save max score
        if ((snek_length - 1) > max_score)
        {
            max_score = snek_length - 1;
            Global::preferences.begin("Snek", false);
            Global::preferences.putInt("Score", max_score);
            Global::preferences.putBool("PostQueue", true);
            Global::preferences.end();
        }

        gfx.fillScreen(TFT_BLACK);

        // Draw scoreboard
        set_score_text();
        draw_score_text();
        update_score(snek_length - 1);
        draw_max_score_text();
        update_max_score(max_score);

        // Display game over screen
        gfx.setTextSize(3);
        gfx.setTextColor(TFT_RED);
        gfx.setTextDatum(BC_DATUM);
        gfx.drawString("GAME", 140, 120);
        gfx.setTextDatum(TC_DATUM);
        gfx.drawString("OVER", 140, 120);

        delay(2000);
        exit_game();
    }
    // Something went wrong, exit the game
    else
    {
        exit_game();
    }
}

void exit_game()
{
    Global::TODcounter = 0;
    gfx.setTextSize(1);
    gfx.setTextDatum(TL_DATUM);
 
    renderer.giveBackDisplay();
}

void reset_game(int x, int y)
{
    // Initialize head
    snek[0][0] = x;
    snek[0][1] = y;

    // Clean up the body of the snek
    for (uint16_t i = MAX_SNEK_LENGTH; i > 0; i--)
    {
        snek[i][0] = 0;
        snek[i][1] = 0;
    }

    dir = RIGHT;
    snek_length = 1;
    ate_food = false;
}

void change_direction()
{
    if ((dir == LEFT || dir == RIGHT) && switches.isSwitchPressed(UP_PIN))
    {
        dir = UP;
    }
    else if ((dir == LEFT || dir == RIGHT) && switches.isSwitchPressed(DOWN_PIN))
    {
        dir = DOWN;
    }
    else if ((dir == UP || dir == DOWN) && switches.isSwitchPressed(LEFT_PIN))
    {
        dir = LEFT;
    }
    else if ((dir == UP || dir == DOWN) && switches.isSwitchPressed(RIGHT_PIN))
    {
        dir = RIGHT;
    }
}

void move()
{
    uint x = snek[0][0];
    uint y = snek[0][1];

    switch (dir)
    {
    case UP:
        y--;
        break;
    case DOWN:
        y++;
        break;
    case LEFT:
        x--;
        break;
    case RIGHT:
        x++;
        break;
    }

    // Shift individual segments, starting from the tail
    for (uint16_t i = snek_length; i > 0; i--)
    {
        snek[i][0] = snek[i - 1][0];
        snek[i][1] = snek[i - 1][1];
    }

    // Update the head position
    snek[0][0] = x;
    snek[0][1] = y;
}

bool wall_collision()
{
    if ((snek[0][0] >= BLOCK_X) || (snek[0][1] >= BLOCK_Y))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ate_itself()
{
    // Snek cannot eat itself if it's less than 5 segments long
    if (snek_length > 4)
    {
        for (uint16_t i = 4; i < snek_length; i++)
        {
            // If snek's head is on top of another segment, snek ate itself
            if ((snek[i][0] == snek[0][0]) && (snek[i][1] == snek[0][1]))
            {
                return true;
            }
        }
    }
    return false;
}

void update_display()
{
    // Draw the new head segment
    int x = snek[0][0];
    int y = snek[0][1];
    gfx.fillRect(BLOCKXY * x + OFFSET, BLOCKXY * y + OFFSET, BLOCKXY, BLOCKXY, TFT_GREEN);

    // Erase the old tail segment if we didn't eat the food
    if (!ate_food)
    {
        x = snek[snek_length][0];
        y = snek[snek_length][1];
        gfx.fillRect(BLOCKXY * x + OFFSET, BLOCKXY * y + OFFSET, BLOCKXY, BLOCKXY, TFT_BLACK);
    }
    else
    {
        // If we ate the food, we need to reset the tracker after updating the display
        ate_food = false;
    }
}

void create_food()
{
    // Counter to build our free location array
    uint16_t i = 0;
    bool on_snek;

    // Go through the whole grid
    for (uint x = 0; x < BLOCK_X; x++)
    {
        for (uint y = 0; y < BLOCK_Y; y++)
        {
            on_snek = false;

            // Check if a location is on top of our snek
            for (uint s = 0; s < snek_length; s++)
            {
                if ((snek[i][0] == x) && (snek[i][1] == y))
                {
                    on_snek = true;
                    break;
                }
            }

            // If a picked location is not on our snek, add it to possible locations
            if (!on_snek)
            {
                free_locations[i][0] = x;
                free_locations[i][1] = y;
                i++;
            }
        }
    }

    // Pick a random location, save it and display the food
    // To get the number of free locations we substract the length from the number of cells in the grid
    i = random(MAX_SNEK_LENGTH - snek_length);
    food[0] = free_locations[i][0];
    food[1] = free_locations[i][1];
    gfx.fillCircle(BLOCKXY * food[0] + OFFSET + BLOCKXY / 2, BLOCKXY * food[1] + OFFSET + BLOCKXY / 2, BLOCKXY / 2 - 1, TFT_RED);
}

void food_eaten()
{
    ate_food = ((snek[0][0] == food[0]) && (snek[0][1] == food[1]));

    // If we ate the food, we need to increase the length, update the score and create a new food
    if (ate_food)
    {
        snek_length++;
        set_score_text();
        update_score(snek_length - 1);
        create_food();
    }
}

void set_score_text()
{
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_GREEN);
}

void draw_score_text()
{
    gfx.fillRect(0, 0, 140, 20, TFT_BLACK);
    gfx.setTextDatum(TR_DATUM);
    gfx.drawString("Score: ", 139, 0);
}

void update_score(uint16_t score)
{
    gfx.fillRect(140, 0, 140, 20, TFT_BLACK);
    gfx.setTextDatum(TL_DATUM);
    gfx.drawNumber(score, 140, 0);
}

void draw_max_score_text()
{
    gfx.fillRect(0, 220, 140, 20, TFT_BLACK);
    gfx.setTextDatum(TR_DATUM);
    gfx.drawString("Max: ", 139, 220);
}

void update_max_score(uint16_t max_score)
{
    gfx.fillRect(140, 220, 140, 20, TFT_BLACK);
    gfx.setCursor(140, 220);
    gfx.setTextDatum(TL_DATUM);
    gfx.drawNumber(max_score, 140, 220);
}
