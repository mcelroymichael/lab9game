#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>

#define MAXWORLD_SIZE 2

typedef struct{
    uint32_t* tilemap;
} Room;

void roomInit(Room* r, uint32_t* tilemap);
void drawRoom(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y);
void setWorld(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], Room* room,uint8_t x, uint8_t y);

#endif