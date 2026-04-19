#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>

#define MAXWORLD_SIZE 2
#define ROOM_TILE_WIDTH 8
#define ROOM_TILE_HEIGHT 10

typedef struct{
    const uint32_t* tilemap;
} Room;

void roomInit(Room* r, const uint32_t* tilemap);
void drawRoom(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y);
void drawRoomTile(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t roomX, uint8_t roomY,uint8_t tileX, uint8_t tileY);
void worldInit(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], Room* null);
void setWorld(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], Room* room,uint8_t x, uint8_t y);
const uint32_t* getTileMap(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y);

#endif
