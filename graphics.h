#ifndef GRAPHICS_H

#include "../inc/ST7735.h"
#include "room.h"

#define GRAPHICS_H

#define COLOR_BLACK  ST7735_BLACK
#define COLOR_YELLOW ST7735_YELLOW
#define COLOR_BLUE   ST7735_BLUE

//Null array of type uint8_t
extern const uint8_t NULLARR8[0];
//Null array of type uint16_t
extern const uint16_t NULLARR16[0];

extern const uint16_t blueBlock[64];
extern const uint16_t yellowBlock[64];
extern const uint16_t happyBlock[64];
extern const uint8_t testMap[320];
extern const uint16_t bighappy[144];
extern const uint16_t cloudBottom[64];
extern const uint16_t cloudTop[64];
extern const uint16_t cat[256];
extern const uint16_t greentile[144];
extern const uint16_t redtile[144];
extern const uint16_t bluetile[144];
extern const uint16_t teleporter_Inactive[144];
extern const uint16_t teleporter_Active[144];
extern const uint16_t happyBlue[64];
extern const uint16_t meleeIcon[48];
extern const uint16_t moveIcon[48];
extern const uint16_t rangedIcon[48];
extern const uint16_t cursor[16];
extern const uint16_t logo[10240];
extern const uint16_t enemy1[64];
extern const uint32_t tilemap1[80];
extern const uint32_t tilemap2[80];
extern const uint32_t tilemap3[80];

typedef struct{
    uint8_t destinationWorldX;
    uint8_t destinationWorldY;
    uint8_t destinationPlayerX;
    uint8_t destinationPlayerY;
    uint8_t currentX;
    uint8_t currentY;
} TeleporterData;

extern const TeleporterData teleportTable[MAXWORLD_SIZE][MAXWORLD_SIZE];

#endif
