/**
 * This is a simple demo application for Stm32Duino based boards. It just showcases many of the types of editor that
 * are available. By default it is setup for an OLED screen and a rotary encoder, although it could be moved to use
 * many other different display and input technologies.
 */

#include "stm32DuinoDemo_menu.h"
#include <PlatformDetermination.h>
#include <SPI.h>

#include <TaskManagerIO.h>

//                        0123456789 0123456789 0123456789 0123456789 0123456789
const char* ramDataSet = "Item 1\0   Item 2\0   Item 3\0   Item 4\0   Item 5\0   ";

class MyCustomDrawing : public CustomDrawing {
private:
    GraphicsDeviceRenderer& dev;
    int ticks;
public:
    MyCustomDrawing(GraphicsDeviceRenderer& r) : dev(r), ticks(0) {}

    void registerWithRenderer() {
        dev.setCustomDrawingHandler(this);
    }

    void started(BaseMenuRenderer *currentRenderer) override {
        // called once when the take-over display  is started before calling renderLoop so you can set things up.
        switches.getEncoder()->changePrecision(100, 50);
    }

    void reset() override {
        // called whenever the display is reset, IE times out on editing etc.
    }

    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {
        // called in a game loop between takeOverDisplay and giveBackDisplay, at this point you renderer the display.
        if(userClick == RPRESS_PRESSED) {
            dev.giveBackDisplay();
        }
        else if(++ticks % 10 == 1) {
            // Why write your own code using device drawable? The main reason is, that it works exactly the same over
            // adafruit, u8g2 and TFTeSPI with a moderately complete API.
            DeviceDrawable *dd = dev.getDeviceDrawable();
            dd->startDraw();
            const Coord &dims = dd->getDisplayDimensions();
            dd->setDrawColor(BLACK);
            dd->drawBox(Coord(0, 0), dims, true);
            dd->setColors(WHITE, BLACK);
            auto height = int(dims.y) - 16;
            int width = int(dims.x) - 20;
            dd->drawText(Coord(rand() % width, (rand() % height) + 10), nullptr, 1, "hello");
            dd->drawText(Coord(rand() % width, (rand() % height) + 10), nullptr, 1, "world");
            char sz[10];
            ltoaClrBuff(sz, currentValue, 4, NOT_PADDED, sizeof sz);
            dd->drawText(Coord(0, 0), nullptr, 1, sz);
            dd->endDraw();
        }
    }
} myCustomDrawing(renderer);

void setup() {
    // Start up serial and prepare the correct SPI
    Serial.begin(115200);
    SPI.setMISO(PB4);
    SPI.setMOSI(PB5);
    SPI.setSCLK(PB3);

    // Now start up the ethernet library.
    Ethernet.begin();
    Serial.print("My IP address is ");
    Ethernet.localIP().printTo(Serial);
    Serial.println();

    // and then run the menu setup
    setupMenu();

    menuMgr.load();

    myCustomDrawing.registerWithRenderer();
    setTitlePressedCallback([](int) {
        renderer.takeOverDisplay();
    });
}

void loop() {
    taskManager.runLoop();
}


// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnRuntimesCustomListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        // TODO - your code to invoke goes here - row is the index of the item
        return true;
    case RENDERFN_NAME:
        // TODO - each row has it's own name - 0xff is the parent item
        ltoaClrBuff(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_VALUE:
        // TODO - each row can has its own value - 0xff is the parent item
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}


void CALLBACK_FUNCTION decimalDidChange(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION saveWasPressed(int id) {
     auto bspBackupRam = reinterpret_cast<HalStm32EepromAbstraction*>(menuMgr.getEepromAbstraction());
     menuMgr.save();
     bspBackupRam->commit();
}


void CALLBACK_FUNCTION largeNumDidChange(int id) {
    // TODO - your menu change code
}
