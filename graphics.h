#ifndef GRAPHICS_H

#include "../inc/ST7735.h"

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
extern const uint16_t logo[10240];
extern const uint32_t tilemap1[64];
extern const uint32_t tilemap2[64];
extern const uint32_t tilemap3[64];

#endif