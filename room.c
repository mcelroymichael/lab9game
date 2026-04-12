#include "room.h"
#include "../inc/ST7735.h"
#include "graphics.h"

void roomInit(Room* r, uint32_t* tilemap){
    r->tilemap = tilemap;
}

void drawRoom(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y){
    Room* r = world[x][y];
    
    for(uint32_t i = 0; i < 64; i++){

        if(r->tilemap[i]==0){
          ST7735_DrawBitmap((i%8)*12, (((i/8)+1)*12)-1, greentile, 12, 12);
          } else if(r->tilemap[i]==1) {
          ST7735_DrawBitmap((i%8)*12, (((i/8)+1)*12)-1, bluetile, 12, 12);
          } else if(r->tilemap[i]==2) {
          ST7735_DrawBitmap((i%8)*12, (((i/8)+1)*12)-1, redtile, 12, 12);
          }
  }
}

void setWorld(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], Room* room,uint8_t x, uint8_t y){
  world[x][y] = room;
}