#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#define MAXENTITIES 10
#define PLAYER 0
#define ENEMY 1

// **********Entity Struct Supertype definition*******************
// draw score on screen horizontally as 4-digit number
// font is 5 wide-6 high Courier bold yellow on black
// Input: n is between 0 and 9999
//        x     horizontal position of the bottom left corner of the image, columns from the left edge
//        y     vertical position of the bottom left corner of the image, rows from the top edge
// Output: none
typedef struct {
    uint8_t active;     
    uint8_t x;
    uint8_t y;
    uint8_t oldx;
    uint8_t oldy;
    int8_t vx;
    int8_t vy;
    uint8_t width;
    uint8_t height;
    uint8_t type;
    const uint16_t* bitmap;
    const uint8_t* tilemap;
} Entity;

// **********Entity_Init*******************
// initializes entity
// Input:   e is the entity's pointer
//          x         horizontal position
//          y         vertical position
//          width     width of bitmap
//          height    height of bitmap
//          bitmap    points to bitmap data
// Output: none
void Entity_Init(Entity* e, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t type, const uint16_t* bitmap, const uint8_t* tilemap);

// **********Entity_Activate*******************
// set entity active
// Input: e is the entity's pointer
// Output: none
void Entity_Activate(Entity* e);

// **********Entity_Deactivate*******************
// set entity inactive
// Input: e is the entity's pointer
// Output: none
void Entity_Deactivate(Entity* e);

// **********Entity_Update*******************
// updates entity position using velocity
// Input: e is the entity's pointer
// Output: none
void Entity_Update(Entity* e);

void Entity_Friction(Entity* e);

void Entity_SetPosition(Entity* e, int8_t x, int8_t y);

// **********Entity_SetVelocity*******************
// sets entity velocity
// Input: e is the entity's pointer
//        vx is horizontal velocity
//        vy is vertical velocity
// Output: none
void Entity_SetVelocity(Entity* e, int8_t vx, int8_t vy);

// **********Entity_AddVelocity*******************
// adds vector to entity's velocity
// Input: e is the entity's pointer
//        vx is horizontal velocity
//        vy is vertical velocity
// Output: none
void Entity_AddVelocity(Entity* e, int8_t vx, int8_t vy);

// **********Entity_PrintSelf*******************
// displays the entity on the screen
// Input: e is the entity's pointer
// Output: none
static void Entity_PrintSelf(Entity* e);

// **********Entity_SetBitmap*******************
// displays the entity on the screen
// Input: e is the entity's pointer, bitmap is a pointer to an array of pixels
// Output: none
// Output: none
void Entity_SetBitmap(Entity* e, const uint16_t* bitmap);

// **********Entity_SetTiles*******************
// displays the entity on the screen
// Input: e is the entity's pointer, tilemap is a pointer to an array of tiles
// Output: none
void Entity_SetTiles(Entity* e, const uint8_t* tilemap);

static void redrawback(Entity* e);

void entityArrInit(Entity* entities);

//  PLEASE ALWAYS INITIALIZE YOUR ENTITY IMMEDIATELY!!
//  THE RETURNED POINTER IS ALWAYS GOING TO POINT TO GARBAGE
//  UNLESS INITIALIZED
Entity* addEntity(Entity* entities);

void updateEntities(Entity* entities);

void drawEntities(Entity* entities);

void checkHoriCollisions(Entity* e, const uint8_t* tilemap);
void checkVertCollisions(Entity* e, const uint8_t* tilemap);

static void fixHoriCollision(Entity* e);
static void fixVertCollision(Entity* e);

#endif // ENTITY_H