#include "Plotter.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"
#include "MQTT_Sensors.h"
#include <TFT_eWidget.h>               // Widget library

#define ScreenWidth 280
#define ScreenHeight 240



// generic plot size values
const float gxLow  = 0.0;
const float gxHigh = 100.0;
const float gyLow  = -512.0;
const float gyHigh = 512.0;

int plotState = 0;


GraphWidget gr = GraphWidget(&gfx);    // Graph widget gr instance with pointer to tft (gfx)
TraceWidget tr = TraceWidget(&gr);     // Graph trace tr with pointer to gr


void drawTemperaturePlot(unsigned int encoderValue, RenderPressMode clicked) {
    const float T_rangeHigh = 50.0;
    const float T_rangeLow =  0;   // -23.0;

    switch(plotState) {
        case 0: {
            gfx.fillScreen(TFT_BLACK);
            // Graph area is 200 pixels wide, 150 pixels high, dark grey background
            gr.createGraph(200, 150, gfx.color565(5, 5, 5));

            // x scale units is from 0 to 100, y scale units is -512 to 512
            gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);

            // X grid starts at 0 with lines every 20 x-scale units
            // Y grid starts at -512 with lines every 64 y-scale units
            // blue grid
            gr.setGraphGrid(gxLow, 20.0, gyLow, 64.0, TFT_BLUE);

            // Draw empty graph, top left corner at pixel coordinate 40,10 on TFT
            gr.drawGraph(40, 10);

            // Start a trace with using red, trace points are in x and y scale units
            // In this example a horizontal line is drawn
            tr.startTrace(TFT_RED);
            // Add a trace point at 0.0,0.0 on graph
            tr.addPoint(0.0, 0.0);
            // Add another point at 100.0, 0.0 this will be joined via line to the last point added
            tr.addPoint(100.0, 0.0);

            // Start a new trace with using white
            tr.startTrace(TFT_WHITE);

            plotState++;
        }

        case 1: {
            if(clicked) {
                plotState = 0;
                renderer.giveBackDisplay();
            }
            envSensor_Update();
            static float gx = 0.0, gy = 0.0;
           // static float delta = 10.0;

            // Create a new plot point every 100ms
            // nope.. this will be handled by the frame rate...
            //if (millis() - plotTime >= 100) {
            //  plotTime = millis();
            gy = map(Global::Temperature, T_rangeLow, T_rangeHigh, gyLow, gyHigh);
            // Add a plot, first point in a trace will be a single pixel (if within graph area)
           tr.addPoint(gx, gy);
           // tr.addPoint(gx, gy);
            gx += 1.0;
           // if (gy >  500.0) delta = -10.0;
           // if (gy < -500.0) delta =  10.0;
           // gy += delta;

            // If the end of the graph x ais is reached start a new trace at 0.0,0.0
            if (gx > gxHigh) {
            gx = 0.0;
            gy = 0.0;

            // Draw empty graph at 40,10 on display to clear old one
            gr.drawGraph(40, 10);
            // Start new trace
            tr.startTrace(TFT_GREEN);
            }
        }
    }
}


void drawHumidityPlot(unsigned int encoderValue, RenderPressMode clicked) {
    const float H_rangeHigh = 100.0;
    const float H_rangeLow =  0;   // -23.0;

    switch(plotState) {
        case 0: {
            gfx.fillScreen(TFT_BLACK);
            // Graph area is 200 pixels wide, 150 pixels high, dark grey background
            gr.createGraph(200, 150, gfx.color565(5, 5, 5));

            // x scale units is from 0 to 100, y scale units is -512 to 512
            gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);

            // X grid starts at 0 with lines every 20 x-scale units
            // Y grid starts at -512 with lines every 64 y-scale units
            // blue grid
            gr.setGraphGrid(gxLow, 20.0, gyLow, 64.0, TFT_BLUE);

            // Draw empty graph, top left corner at pixel coordinate 40,10 on TFT
            gr.drawGraph(40, 10);

            // Start a trace with using red, trace points are in x and y scale units
            // In this example a horizontal line is drawn
            tr.startTrace(TFT_RED);
            // Add a trace point at 0.0,0.0 on graph
            tr.addPoint(0.0, 0.0);
            // Add another point at 100.0, 0.0 this will be joined via line to the last point added
            tr.addPoint(100.0, 0.0);

            // Start a new trace with using white
            tr.startTrace(TFT_WHITE);

            plotState++;
        }

        case 1: {
            if(clicked) {
                plotState = 0;
                renderer.giveBackDisplay();
            }
            envSensor_Update();
            static float gx = 0.0, gy = 0.0;
           // static float delta = 10.0;

            // Create a new plot point every 100ms
            // nope.. this will be handled by the frame rate...
            //if (millis() - plotTime >= 100) {
            //  plotTime = millis();
            gy = map(Global::Humidity, H_rangeLow, H_rangeHigh, gyLow, gyHigh);
            // Add a plot, first point in a trace will be a single pixel (if within graph area)
           tr.addPoint(gx, gy);
           // tr.addPoint(gx, gy);
            gx += 1.0;
           // if (gy >  500.0) delta = -10.0;
           // if (gy < -500.0) delta =  10.0;
           // gy += delta;

            // If the end of the graph x ais is reached start a new trace at 0.0,0.0
            if (gx > gxHigh) {
            gx = 0.0;
            gy = 0.0;

            // Draw empty graph at 40,10 on display to clear old one
            gr.drawGraph(40, 10);
            // Start new trace
            tr.startTrace(TFT_GREEN);
            }
        }
    }
}



TraceWidget trX = TraceWidget(&gr);     // Graph trace tr with pointer to gr
TraceWidget trY = TraceWidget(&gr);     // Graph trace tr with pointer to gr
TraceWidget trZ = TraceWidget(&gr);     // Graph trace tr with pointer to gr

void drawIMUPlot(unsigned int encoderValue, RenderPressMode clicked) {
    const float A_rangeHigh = 1.5;
    const float A_rangeLow =  -1.5;   // -23.0;

    switch(plotState) {
        case 0: {
            init_IMU();
            gfx.fillScreen(TFT_BLACK);
            // Graph area is 200 pixels wide, 150 pixels high, dark grey background
            gr.createGraph(200, 150, gfx.color565(5, 5, 5));

            // x scale units is from 0 to 100, y scale units is -512 to 512
            gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);

            // X grid starts at 0 with lines every 20 x-scale units
            // Y grid starts at -512 with lines every 64 y-scale units
            // blue grid
            gr.setGraphGrid(gxLow, 20.0, gyLow, 64.0, TFT_BLUE);

            // Draw empty graph, top left corner at pixel coordinate 40,10 on TFT
            gr.drawGraph(40, 10);

            // Start a trace with using red, trace points are in x and y scale units
            // In this example a horizontal line is drawn
            trX.startTrace(TFT_RED);
            // Add a trace point at 0.0,0.0 on graph
            trX.addPoint(0.0, 0.0);
            // Add another point at 100.0, 0.0 this will be joined via line to the last point added
            trX.addPoint(100.0, 0.0);

            // Start a new trace with using white
            trX.startTrace(TFT_WHITE);

            plotState++;
        }

        case 1: {
            if(clicked) {
                plotState = 0;
                renderer.giveBackDisplay();
            }
            IMU_Update();
            static float gx = 0.0;
            static float IMUXy = 0.0,  IMUYy = 0.0, IMUZy = 0.0;

           // static float delta = 10.0;

            // Create a new plot point every 100ms
            // nope.. this will be handled by the frame rate...
            //if (millis() - plotTime >= 100) {
            //  plotTime = millis();
            IMUXy = map(Global::IMU_x, A_rangeLow, A_rangeHigh, gyLow, gyHigh);
            IMUYy = map(Global::IMU_y, A_rangeLow, A_rangeHigh, gyLow, gyHigh);
            IMUZy = map(Global::IMU_z, A_rangeLow, A_rangeHigh, gyLow, gyHigh);
            // Add a plot, first point in a trace will be a single pixel (if within graph area)
           trX.addPoint(gx, IMUXy);
           trY.addPoint(gx, IMUYy);
           trZ.addPoint(gx, IMUZy);

           // tr.addPoint(gx, gy);
            gx += 1.0;
           // if (gy >  500.0) delta = -10.0;
           // if (gy < -500.0) delta =  10.0;
           // gy += delta;

            // If the end of the graph x ais is reached start a new trace at 0.0,0.0
            if (gx > gxHigh) {
            gx = 0.0;
            IMUXy = IMUXy = IMUXy = 0.0;

            // Draw empty graph at 40,10 on display to clear old one
            gr.drawGraph(40, 10);
            // Start new trace
            trX.startTrace(TFT_GREEN);
            trY.startTrace(TFT_YELLOW);
            trZ.startTrace(TFT_CYAN);
            }
        }
    }
}
