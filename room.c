#include "room.h"
#include "../inc/ST7735.h"
#include "graphics.h"

void roomInit(Room* r, const uint32_t* tilemap){
    r->tilemap = tilemap;
}

void drawRoom(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y){
    uint8_t tileX;
    uint8_t tileY;

    for(tileY = 0; tileY < ROOM_TILE_HEIGHT; tileY++){
        for(tileX = 0; tileX < ROOM_TILE_WIDTH; tileX++){
            drawRoomTile(world, x, y, tileX, tileY);
        }
    }
}

void drawRoomTile(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t roomX, uint8_t roomY,uint8_t tileX, uint8_t tileY){
    Room* r = world[roomX][roomY];
    uint32_t i = tileX + tileY * ROOM_TILE_WIDTH;

    if(r->tilemap[i] == 0){
        ST7735_DrawBitmap(tileX * 12, ((tileY + 1) * 12) - 1, greentile, 12, 12);
    } else if(r->tilemap[i] == 1){
        ST7735_DrawBitmap(tileX * 12, ((tileY + 1) * 12) - 1, bluetile, 12, 12);
    } else if(r->tilemap[i] == 2){
        ST7735_DrawBitmap(tileX * 12, ((tileY + 1) * 12) - 1, redtile, 12, 12);
    }
}

void worldInit(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], Room* null){
  for(uint32_t x = 0; x < MAXWORLD_SIZE; x++){
    for(uint32_t y = 0; y < MAXWORLD_SIZE; y++){
      world[x][y] = null;
    }
  }
}

void setWorld(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], Room* room,uint8_t x, uint8_t y){
  world[x][y] = room;
}

const uint32_t* getTileMap(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y){
  return world[x][y]->tilemap;
}
