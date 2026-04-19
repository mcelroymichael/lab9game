#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "room.h"

#define MAXENTITIES 10
#define PLAYER 0
#define ENEMY 1
#define TELEPORTER 2


extern const uint16_t NULLARR16[];

typedef struct Entity{
    uint8_t active;

    // Grid position
    uint8_t tileX;
    uint8_t tileY;
    uint8_t oldTileX;
    uint8_t oldTileY;

    // Optional fields kept for compatibility / future use
    int8_t vx;
    int8_t vy;

    uint8_t width;
    uint8_t height;
    uint8_t type;
    const uint16_t* bitmap;
    uint32_t data0;
    uint32_t data1;
} Entity;

void Entity_Init(Entity* e, uint8_t tileX, uint8_t tileY, uint8_t width, uint8_t height, uint8_t type, const uint16_t* bitmap);

void Entity_Activate(Entity* e);
void Entity_Deactivate(Entity* e);
void Entity_Update(Entity* e);
void Entity_SetTilePosition(Entity* e, uint8_t tileX, uint8_t tileY);
uint8_t Entity_TryMove(Entity* e, int8_t dx, int8_t dy, const uint32_t* roomTilemap);
void Entity_SetBitmap(Entity* e, const uint16_t* bitmap);
void entityArrInit(Entity* entities);
Entity* addEntity(Entity* entities);
void updateEntities(Entity* entities);
void drawEntities(Entity* entities, Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t roomX, uint8_t roomY);

#endif