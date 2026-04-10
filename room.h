#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>

typedef struct{
    uint32_t* tilemap;
} Room;

void roomInit(Room* r, uint32_t* tilemap);
void drawRoom(Room* r);

#endif