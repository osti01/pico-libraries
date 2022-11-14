// ILI9341 Driver by Piotr Ostafin
// 2021

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <string>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "Class_ILI9341.cpp"
#include "Class_Graphics.cpp"
#include "Class_L76B.cpp"

//#include "ATASCII.h"
//#include "ILI9341.h"
//#include "colors.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 4
#define UART_RX_PIN 5




cIli9341 *Display, *Display2;
cGraphics *Gph, *Gph2;
cL76B *Gps;

char text[100];
char textDriver[100];
int frameCount = 0;
int fps = 0;
int instrCount = 0;
int score = 0;
int baudRate = 0;
int sysClock = 0;

#define LED_PIN     PICO_DEFAULT_LED_PIN


void blink(int delay){
    gpio_put(LED_PIN, 1);
    sleep_ms(delay);
    gpio_put(LED_PIN, 0);
    sleep_ms(delay);
    return;
}


bool timerCallback(struct repeating_timer *t){
    Display->statTick();
    printf("tick\n");
    return true;
}

void gpsRxInterupt(){
    Gps->onUartRx();
}

bool timerRefreshScreen(struct repeating_timer *t){
    //Display->writeScreenBuffer();
    //Display2->enableRefreshingScreen();
    //Display2->refresh();
    //Display2->enableRefreshingScreen();
    //Display2->clearDisplay();
    Gph->clearAll();
    Gph->refresh();
    Display->refresh();
    

    return true;
}


void core1(){
    char text[100];
    struct repeating_timer timer1;
    struct repeating_timer timer2;
    int substeps = 200;
    //int tailLength = 100;
    int artSize = 110;

    add_repeating_timer_ms(1000, timerCallback, NULL, &timer1);
    add_repeating_timer_ms(5, timerRefreshScreen, NULL, &timer2);


    while(1){
        

        for(long i=0; i<10000000; i+=10){
            Display->disableRefreshingScreen();
            Gph->clearAll();
            Gph2->clearAll();
            int x1, x2, y1, y2;
            //Gph->fadeScreen();
            for(long substep=0; substep<substeps; substep+=8){


                x1 = 110*sin(float((float)(i+substep)/153))+120;
                y1 = 35*cos(float((float)(i+substep)/28))+40;

                Gph2->sRectangle(x1-2,y1-2, 5, 5, ORANGE);

                x1 = artSize*sin(float((float)(i+substep)/57))+120;
                y1 = artSize*cos(float((float)(i+substep)/129))+120;
                x2 = artSize*sin(float((float)(i+substep)/189) + 33.14)+120;
                y2 = artSize*cos(float((float)(i+substep)/69) + 3.14)+120;

                Gph->sLine(x1, y1, x2, y2, RED);
                Gph->sRectangle(x1, y1, 5, 5, YELLOW);
                Gph->sRectangle(x2, y2, 5, 5, BLUE);
            }
            //Gph->print("       ");
            //Gph->setOrientation((i/500)%4);
            sprintf(text, "F:%i %i %i %i %i\n\n", Display->fps, x1, y1, x2, y2);
            Gph2->print(text);
            Gph2->refresh();
            Display->refresh();
            Display2->refresh();
            sleep_ms(10);
        }

    }
}

int main() {
    printf("start\n");
    Display = new cIli9341(spi1, 240, 320);
    Display->setupClocks();
    Display->initialise();
    Display->clearDisplay();
    Display->createWorkArea(0, 0, 240, 240);

    Display2 = new cIli9341(spi1, 240, 320);
    Display2->createWorkArea(0, 240, 240, 80);
 
    Gph = new cGraphics(Display->buffer16, GPH_ORI_UP, Display->workWidth, Display->workHeight);
    Gph2 = new cGraphics(Display2->buffer16, GPH_ORI_UP, Display2->workWidth, Display2->workHeight);
    Gph2->cursorShow();

    Gph->setBackTransp(true);
    Gph2->setBackTransp(true);
    Gph2->setFontColor(RED);
    
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    blink(100);

    //Gps = new cL76B(gpsRxInterupt);
    //Gps->setGph(Gph);
    //Gps->enable();

    struct repeating_timer timer1;
    struct repeating_timer timer2;
    add_repeating_timer_ms(5, timerCallback, NULL, &timer1);
    add_repeating_timer_ms(5, timerRefreshScreen, NULL, &timer2);

 
    multicore_launch_core1(core1);

    while(1){
        tight_loop_contents();
        //gpsRxInterupt();
        //sleep_ms(100);
        //sleep_ms(100);
        //gpsRxInterupt();
        //blink(100);
    }
}



