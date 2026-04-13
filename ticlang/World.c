// World.c
// Runs on MSPM0G3507
// Stores, manages, and modifies world data
// Alp Shamsapour & Michael McElroy
// Last Modified: April 13, 2026

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "entity.h"
#include "graphics.h"
#include "room.h"
#include "Lab9Main.c"
#include "World.h"

#define language    0;
#define mainmenu    1;
#define tutorial    2;
#define level1      3;
#define level2      4;
#define level3      5;

struct world{
    int cur;
    int states[6] = {0};
    bool allowinput;
}

typedef struct world world_t;

void WorldInit(void){
    world.cur = 0;
    allowinput = 0;
}