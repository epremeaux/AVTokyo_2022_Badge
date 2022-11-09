/**********************************************************************
 * Quiz game
 * Inspired by:
 * https://www.geeksforgeeks.org/quiz-game-in-c/
 * and https://www.youtube.com/watch?v=yD3MGmGtlWY
 * 
 * Modified for AVT-Badge by DIYSCIBORG (Emery)
 **********************************************************************
 *
 * Game is driven by state machine using the state variable
 * 0 : first time / instructions
 * 1 : asking a question
 * 2 : wait for answer  : user options here are up, down, select answer, or quit.
 * 3 : ask are you sure you want to quit
 * 4 : wait for user input : quit or return to game
 * 5 : game over screen
 * 6 : wait for user input : exit or replay
***********************************************************************/


#include "quiz.h"
#include "ButtonIcons.h"
#include "Preferences.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"

/* Quiz game does not need high frame performance, so we dont use the frame counter
 * (TODcounter). This counter also warns the MQTT loop not to post.
 * Since we dont use it, MQTT will continue to post while playing quiz.
 */

// we need these so we can capture all the button inputs
#define RIGHT_PIN   32      //36
#define DOWN_PIN    33
#define UP_PIN      34
#define LEFT_PIN    35
#define SELECT_PIN  39
#define A_PIN       25      //A button
#define B_PIN       26


// make these strings as its better performance especially in that we dont need to define the length first, and it can be of "any" length..
static const String questions[] =  {  "Are you sure you\nwant to quit?\n(Best score saved)",
                         "1st year of AVTOKYO?",
                         "Port number of SSH?",
                         "Famous phrase: \n\"____ the Planet!\"",
                         "No ___, no ___ !", 
                         "When was the first \nelectronic badge of \nAVTokyo?",
                         "Mt Fuji's altitude?",
                         "Tokyo Tower's hight?",
                         "Default port of MQTT?",
                         "A famous port mapping\n tool?",
                         "C-3PO is what kind of\n droid?",
                         "R2D2 is what kind of\n droid?",
                         "...---... is what code?",
                         "The answer to life,\n the universe\n and everything?",
                         "ASCII char '0' to hex",
                         "Who said \"I'm sorry\n Dave. I can't do that.\"",
                         "Who said \"I'll be back!\"",
                         "\"I feel the need!\n The need for ___!\"",
                         "Allows current to flow\n only one way.",
                         "Many electric parts in a\n small package.",
                         "Has a gate, a source,\n and a drain.",
                         "Converts current flow\n into heat.",
                         "Famous frequency for\n hacking telephone\n networks.",
                         "\"Manhatten style\" PC\n with Motorolla CPU",
                         "Japanese PC made in\n cooporation with\n Microsoft",
                         "This Japanese PC had\n a built in CD player",
                         "NEC's 16 & 32-bit\n computer line",
                         "NOT a Japanese\n Cyberpunk story",
                         "Social hack : searching\n company trash",
                         "Social hack : romantic\n talk to get information",
                         "Social hack : sneaking in behind someone else",
                         "Social hack : telling a\n backstory to gain trust",
                         "Early HackerSpace in\n Berlin",
                         "Early HackerSpace in\n Boston, Testifed in US\n Congress.",
                         "First HackerSpace in\n Japan",
                         "30th hacker con in L.V.\nin this year?",
                         "What is #3 in the \nOWASP Top 10:2021?",
                         "What is a start command for Metasploit?",
                         "Nmap's OS detection\n option?",
                         "Wi-Fi 6 is IEEE802.11 __",
                         "Which is the ROT-13's\n'hack'?",
                         "First character color\nof Google",
                         "Hacker tool disguised\nas USB memory",
                         "Breakfast cereal toy used\nto hack phone network",
                         "A Software Defined\nRadio",
                         "A Multi-tool for\npentesters",
                         "Which was NOT shown\nat the Mother Of All\nDemos?",
                         "The Hacker Quarterly",
                         "What is a Blue Box?",
                         "NOT a hard disk\ninterface",
                         "Graphic element of a\ngame",

                         };


static const String options[] =   {  "No!", "", "YES", "",
                        "2011", "2000", "2009", "2008",
                        "20", "21", "22", "25",
                        "Duck", "Where's", "Hack", "Lock",
                        "drink, hack", "alcohol, make", "make, hack", "food, service",
                        "2010", "2015", "2018", "2020",
                        "8848 meters", "3776 meters", "1192 meters", "634 meters",
                        "634 meters", "333 meters", "1010 meters", "321 meters",
                        "1024", "833", "1880", "1883",
                        "SMAP", "PortList", "Nmap", "FooTool",
                        "Protocol", "Astromech", "Battle", "Pilot",
                        "Protocol", "Astromech", "Battle", "Pilot",
                        "Baud code", "Gray code", "Morse code", "Bro code",
                        "Love", "0x2A", "0b11110000", "Peace",
                        "0x2A", "0xAA", "0x3B", "0x30",
                        "HAL 9000", "C-3PO", "Johnny 5", "T-800",
                        "HAL 9000", "C-3PO", "Johnny 5", "T-800",
                        "drinks", "banannas", "speed", "weed",
                        "diode", "MOSFET", "Integrated Circuit", "resistor",
                        "diode", "transistor", "Integrated Circuit", "resistor",
                        "diode",  "MOSFET", "Integrated Circuit", "resistor",
                        "diode", "transistor", "Integrated Circuit", "resistor",
                        "920 mhz", "1400 khz", "2600 hz", "1.21 ghz",
                        "X68000", "FM Towns", "MSX", "PC-9800",
                        "X68000", "FM Towns", "MSX", "PC-9800",
                        "X68000", "FM Towns", "MSX", "PC-9800",
                        "X68000", "FM Towns", "MSX", "PC-9800",
                        "Tetsuo: Iron Man", "Burst City", "964 Pinocchio", "Metropolis",
                        "Tailgating", "Dumpster Dive", "Honey Trap", "Pretexting",
                        "Tailgating", "Dumpster Dive", "Honey Trap", "Pretexting",
                        "Tailgating", "Dumpster Dive", "Honey Trap", "Pretexting",
                        "Tailgating", "Dumpster Dive", "Honey Trap", "Pretexting",
                        "NoiseBridge", "C-Base", "Root Access", "Chaos Com. Camp",
                        "Artisan's Asylum", "HeatSync Lab", "Pumping Station One", "L0pht",
                        "TokyoHackerSpace", "OkayamaMakerSpace", "Akihabara HackerSpace", "Fab Cafe",
                        "DEF CON", "BlackHat",	"AVTOKYO", 	"CODE BLUE",
                        "CSRF",	"Injection", "SSRF", "XSS",
                        "metasploit",	"meta",	"meta-start",	"msfconsole",
                        "- D",	"- B",	"- O",	"-sO",
                        "ax",	"ac",	"n",	"ad",
                        "kcah",	"untr",	"unpx",	"grpu",
                        "red", "blue", "yellow", "green",
                        "Cpt. Crunch Whistle", "Flipper Zero", "HackRF", "Rubber Ducky",
                        "Cpt. Crunch Whistle", "Flipper Zero", "HackRF", "Rubber Ducky",
                        "Cpt. Crunch Whistle", "Flipper Zero", "HackRF", "Rubber Ducky",
                        "Cpt. Crunch Whistle", "Flipper Zero", "HackRF", "Rubber Ducky",
                        "Mouse",	"Hypertext",	"Smalltalk",	"Video Conferencing",
                        "MAKE Magazine",	"2600",	"WIRED",	"Interface",
                        "Telephone booth",	"TARDIS",	"Tone Generator",	"Hard Disk",
                        "EISA",	"IDE",	"ESDI",	"SASI",
                        "Pixie",	"Sprite",	"Nibble",	"Char",
        
                        };


            
// make this 8 bit int
// answers are zero indexed (answer #4 => 3)
// game answers start here....|
const uint8_t answers[] = {2, 3, 2, 2, 0, 1, 1, 1, 3, 2, 0, 1, 2, 1, 3, 0, 3, 2, 0, 2, 1, 3, 2, 0, 2, 1, 3, 3, 1, 2, 0, 3, 1, 3, 0, 0, 1, 3, 2, 0, 2, 1, 3, 0, 2, 1, 2, 1, 2, 0, 1};
const int numberOfQuestions = 50;            //  question 0 is not a game question. Game questions are 1-x
bool wasAsked[numberOfQuestions];           // when a question has been asked, the Q numer is marked true in this array


uint8_t quizstate = 0;              // the state machine counter
long randNumber;                // a.. random number
int remaining;                  // how many questions remain (used to trigger end of game when all questions asked.)

int Score = 0;                  // uhh.. the score ;)
uint8_t selected = 0;           // the answer the user currently has selected
uint8_t currentquestion = 1;    // game question being asked
uint8_t currentCorrectAnswer;   // the correct answer for the current question
int optionIndex = 0;            // the REAL location of the answer choices in the answer array. ex: in array {q1-1, q1-2, q1-3, q1-4, q2-1, q2-2, q2-3, q2-4}  Question 2 answers begin at 4 (zero index array)
#define answerY  120            // screen Y position for drawing the answer list
int TopScore;                   // the player's best score, recalled from the preferences storage





/******************************************************************************************
 *  Helper functions and screen drawing
 ******************************************************************************************/

void debugger() {
    for (int i = 0; i < numberOfQuestions +1; i++) {
        Serial.print(wasAsked[i]); Serial.print(" ");
    }
    Serial.print(" :    remaining : ");
    Serial.println(remaining);
}


void initAskedArray(){
    for (int i = 0; i < numberOfQuestions +1; i++) {
        wasAsked[i] = false;
    }
    randomSeed(millis());  // shuffle random function.
}

void renderQuestion(int QNum) {
    gfx.setTextColor(TFT_CYAN);
    gfx.println(questions[QNum]);
}

void renderAnswerList(int QNum, int selected){
    int optionIndex = QNum * 4;
    gfx.setTextColor(TFT_WHITE);
    gfx.setCursor(0, answerY);
    if (selected == 0) {gfx.textbgcolor = TFT_WHITE;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_WHITE;}
        gfx.println(options[optionIndex]);
    if (selected == 1) {gfx.textbgcolor = TFT_WHITE;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_WHITE;}
        gfx.println(options[optionIndex +1]);
    if (selected == 2) {gfx.textbgcolor = TFT_WHITE;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_WHITE;}
        gfx.println(options[optionIndex +2]);
    if (selected == 3) {gfx.textbgcolor = TFT_WHITE;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_WHITE;}
        gfx.println(options[optionIndex +3]);  
}

void eraseAnswerList(int QNum, int selected){
    int optionIndex = QNum * 4;
    gfx.setTextColor(TFT_BLACK);
    gfx.setCursor(0, answerY);
    if (selected == 0) {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;}
        gfx.println(options[optionIndex]);
    if (selected == 1) {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;}
        gfx.println(options[optionIndex +1]);
    if (selected == 2) {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;}
        gfx.println(options[optionIndex +2]);
    if (selected == 3) {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;} else {gfx.textbgcolor = TFT_BLACK;  gfx.textcolor = TFT_BLACK;}
        gfx.println(options[optionIndex +3]);  
}


void renderGameOver() {
      // screen work
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_CYAN);
    gfx.setCursor(20, 5);
    gfx.println(F("Quiz game"));
    gfx.println();
    gfx.setTextColor(TFT_WHITE);
    gfx.println("Game Over");
    gfx.println();
    gfx.print("Score: ");
    gfx.setTextColor(TFT_GREEN);
    gfx.println(Score);
    gfx.println();
     // save top score
    if (Score > TopScore){
        TopScore = Score;
        Global::preferences.begin("Quiz", false);
        Global::preferences.putInt("Score", TopScore);
        Global::preferences.putBool("PostQueue", true);
        Serial.println("set score");
        Global::preferences.end();
        gfx.setTextColor(TFT_GREEN);
        gfx.println("Congratulations! Your\nnew top score saved.");
    }
    delay(3000);
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_CYAN);
    gfx.setCursor(20, 5);
    gfx.println(F("Quiz game"));
    gfx.setTextColor(TFT_WHITE);

    int32_t joy1y = 40;
    draw_Joy(5, joy1y, 5);
    gfx.setCursor(110, joy1y + 30);
    gfx.print("Exit Game");

    int32_t btny = joy1y + 80 + 10;
    draw_A(20, btny, false);
    gfx.setCursor(110, btny + 10);
    gfx.print("Replay");

    // reset variables
    Score = 0;
    currentCorrectAnswer = 0;
    currentquestion = 1;
    selected = 0;
    // set outside game flow and wait for A button (replay) or Select (quit)
  //  Global::TODcounter = 100;
}


void renderInstructions() {
    // draw the display
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextSize(1);
    gfx.setTextColor(TFT_CYAN);
    gfx.setCursor(20, 5);
    gfx.println(F("Quiz game"));
    gfx.setTextColor(TFT_WHITE);

    int32_t joy1y = 40;
    draw_Joy(5, joy1y, 6);
    gfx.setCursor(110, joy1y + 30);
    gfx.print("List up/down");

    int32_t btny = joy1y + 80 + 10;
    draw_B(20, btny, false);
    gfx.setCursor(110, btny + 10);
    gfx.print("Final Answer");

    delay(3000);
    gfx.setTextColor(TFT_BLACK);
    gfx.setCursor(110, joy1y + 30);
    gfx.print("List up/down");
    // print the new text
    gfx.setTextColor(TFT_WHITE);
    draw_Joy(5, joy1y, 5);
    gfx.setCursor(110, joy1y + 30);
    gfx.print("Quit");
}


/******************************************************************************************
 *  Game State Machien
 ******************************************************************************************
 *
 * This is where the menu actually jumps into the game when calling renderer.takeoverdisplay
 * Note that the menu task loop keeps reentering this same new_quiz function. Therefore,
 * it is important to design each game logic case so thatthat it runs through, then advances
 * to the next case, where we can wait in a loop, constantly rentering that case, until the 
 * user takes action.
 * 
 * If not, we would keep redrawing the screen, or otherwise racing through the game, since
 * the menu loop keeps reentering the game logic case.
 * 

 State Machine :
 * 0 : first time / instructions
 * 1 : asking a question
 * 2 : WAIT for answer  : user options here are up, down, select answer, or quit.
 * 3 : ask are you sure you want to quit
 * 4 : WAIT for user input : quit or return to game
 * 5 : game over screen
 * 6 : WAIT for user input : exit or replay
 */ 
void new_quiz(unsigned int encoderValue , RenderPressMode clicked) {
    switch (quizstate) {
/*************** first time / instructions ***************/
        case 0: 
            initAskedArray();
            remaining = numberOfQuestions;
            Global::preferences.begin("Quiz", false);
            TopScore = Global::preferences.getInt("Score", 0);
            Global::preferences.end();
            Score = 0;
            currentCorrectAnswer = 0;
            currentquestion = 1;
            selected = 0;
            renderInstructions();
            delay(1000);
            quizstate++;        // move on to ask the first question
        break;
/*************** Asking a game question ***************/
        case 1: 
            if (remaining == 0) {
                quizstate = 5;      // we have asked all questions. End the game.
            } else {
                // random select a quesiton..
                randNumber = random(1, numberOfQuestions);
                currentquestion = randNumber;
                Serial.print("rand: "); Serial.print(randNumber); Serial.print(" : "); Serial.println(currentquestion);
                // if it has already been asked, advance forward through the asked array until we find one
                while(wasAsked[currentquestion] == true) {
                    currentquestion++;
                    Serial.print(">");
                    // if we reach the end of the asked array, loop to position 1 (not zero, thats the quit quesiton)
                    if(currentquestion > numberOfQuestions) {
                        currentquestion = 1;
                        Serial.print(currentquestion);
                    }
                }
                Serial.println();
                // found a question, set the asked array to true
                wasAsked[currentquestion] = true;
                remaining--;
                debugger();
                // draw the screen
                gfx.fillScreen(TFT_BLACK);
                gfx.setCursor(20, 5);
                gfx.setTextColor(TFT_GREEN);
                gfx.print("Score: ");
                gfx.print(Score);
                gfx.print("      top: ");
                gfx.println(TopScore);
                renderQuestion(currentquestion);
                selected = 0;
                renderAnswerList(currentquestion, selected);
                delay(1000);
                quizstate++;        // now wait for the user input
            }
        break;
/*************** Wait for answer to the question ***************/
        case 2:         // user options here are up, down, select answer, or quit.
            if (clicked) {
                // other code?
                quizstate = 3;
            }

            if (switches.isSwitchPressed(UP_PIN)) {
                if (selected > 0) {
                    delay(100);         // a bit of trouble with skipping over selections
                    eraseAnswerList(currentquestion, selected);
                    selected = selected - 1;
                    renderAnswerList(currentquestion, selected);
                 }
            }

            if (switches.isSwitchPressed(DOWN_PIN)) {
                if (selected < 3) {
                    delay(100);         // a bit of trouble with skipping over selections
                    eraseAnswerList(currentquestion, selected);
                    selected = selected +1;
                    renderAnswerList(currentquestion, selected);
                  }
            }

            if (switches.isSwitchPressed(B_PIN)) {
                if(selected == answers[currentquestion]){
                    //correct!
                    Score = Score + 10;
                    gfx.fillScreen(TFT_BLACK);
                    gfx.setTextColor(TFT_GREEN);
                    gfx.setCursor(20, 5);
                    gfx.println("CORRECT!");
                } else {
                    gfx.fillScreen(TFT_BLACK);
                    gfx.setTextColor(TFT_RED);
                    gfx.setCursor(20, 5);
                    gfx.println("Sorry. Wrong.");
                }
                delay(1000);
                currentquestion++;          // next game question
                selected = 0;
                quizstate = 1;          // set state back to ask a question
            }
        break;
/*************** Ask the user if they want to quit the game ***************/
        case 3: 
            gfx.fillScreen(TFT_BLACK);
            gfx.setCursor(30, 5);
            gfx.setTextColor(TFT_GREEN);
            gfx.print("Score: ");
            gfx.print(Score);
            gfx.print("      top: ");
            gfx.println(TopScore);
            renderQuestion(0);
            selected = 0;
            renderAnswerList(0, selected);
            quizstate++;        // now wait for the user input
        break;
/*************** wait for them to answer about qutting the game ***************/
        case 4:
         if (switches.isSwitchPressed(UP_PIN)) {
                if (selected > 0) {
                    delay(100);         // avoid double clicks
                    eraseAnswerList(0, selected);
                    selected = selected - 2;
                    renderAnswerList(0, selected);
                }
            }

            if (switches.isSwitchPressed(DOWN_PIN)) {
                if (selected < 2) {
                    delay(100);         // avoid double clicks
                    eraseAnswerList(0, selected);
                    selected = selected + 2;
                    renderAnswerList(0, selected);
                }
            }

            if (switches.isSwitchPressed(B_PIN)) {
                if(selected == 2){
                    //  yes quit
                    delay(1000);
                    quizstate = 5; 
                } else {
                    // no! go back to the game
                    quizstate = 1;
                }
            }
        break;
/*************** Show the game over screen ***************/
        case 5:
            renderGameOver();
            // save high score
            quizstate++;
        break;
/*************** wait for user to choose exit, or replay game ***************/
        case 6: 
             if (clicked) {         // quit
                quizstate = 0;          // reset the state or you'll just loop back out when restarting from the menu
                renderer.giveBackDisplay();     // quit back to menu
            }

            if (switches.isSwitchPressed(A_PIN)) {
                quizstate = 0;                      // replay
            }
        break;

        default:
        // if you got here something went seriously wrong
        break;
    }
}
