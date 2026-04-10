#include "entity.h"
#include "../inc/ST7735.h"
#include "graphics.h"



// **********Entity_Init*******************
// initializes entity
// Input:   e is the entity's pointer
//          x         horizontal position
//          y         vertical position
//          width     width of bitmap
//          height    height of bitmap
//          bitmap    points to bitmap data
// Output: none
void Entity_Init(Entity* e, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t type, const uint16_t* bitmap, const uint8_t* tilemap){
    e->active = 0;
    e->x = x;
    e->y = y;
    e->oldx = 255;
    e->oldy = 255;
    e->vx = 0;
    e->vy = 0;
    e->width = width;
    e->height = height;
    e->type = type;
    e->bitmap = bitmap;
    e->tilemap = tilemap;
}

// **********Entity_Activate*******************
// set entity active
// Input: e is the entity's pointer
// Output: none
void Entity_Activate(Entity* e){
    e->active = 1;
}

// **********Entity_Deactivate*******************
// set entity inactive
// Input: e is the entity's pointer
// Output: none
void Entity_Deactivate(Entity* e){
    e->active = 0;
}

// **********Entity_Update*******************
// updates entity position using velocity
// Input: e is the entity's pointer
// Output: none
void Entity_Update(Entity* e){
    if(e->active){
        Entity_Friction(e);
        Entity_AddVelocity(e, 0, 1);
        Entity_SetVelocity(e, 1, e->vy);
        e->y = (e->y + e->vy)%160;
        checkVertCollisions(e, e->tilemap);
        e->x =(e->x + e->vx)%128;
        checkHoriCollisions(e, e->tilemap);

    }
}

void Entity_Friction(Entity* e){
    
    if(e->vx == 0) return;
    e->vx += (e->vx < 0) ? 1 : -1;
}

void Entity_SetPosition(Entity* e, int8_t x, int8_t y){
    e->x = x;
    e->y = y;
}

// **********Entity_SetVelocity*******************
// sets entity velocity
// Input: e is the entity's pointer
//        vx is horizontal velocity
//        vy is vertical velocity
// Output: none
void Entity_SetVelocity(Entity* e, int8_t vx, int8_t vy){
    e->vx = vx;
    e->vy = vy;
}

// **********Entity_AddVelocity*******************
// adds vector to entity's velocity
// Input: e is the entity's pointer
//        vx is horizontal velocity
//        vy is vertical velocity
// Output: none
void Entity_AddVelocity(Entity* e, int8_t vx, int8_t vy){
    e->vx = (e->vx + vx > 7||e->vx + vx < -7) ? 7 : e->vx + vx;
    e->vy = (e->vy + vy > 7||e->vx + vy < -7) ? 7 : e->vy + vy;
}

// **********Entity_PrintSelf*******************
// displays the entity on the screen
// Input: e is the entity's pointer
// Output: none
static void Entity_PrintSelf(Entity* e){
    if(e->active&&(e->oldx!=e->x||e->oldy!=e->y)){
        //ST7735_FillRect(e->oldx, e->oldy - 7, e->width, e->height, ST7735_BLACK);
        redrawback(e);
        ST7735_DrawBitmap(e->x, e->y, e->bitmap, e->width, e->height);
        e->oldx = e->x;
        e->oldy = e->y;
        }
}

void Entity_SetBitmap(Entity* e, const uint16_t* bitmap){
    e->bitmap = bitmap;
}

void Entity_SetTilemap(Entity* e, const uint8_t* tilemap){
    e->tilemap = tilemap;
}

static void redrawback(Entity* e){
    uint8_t leftCol = e->oldx / 8;
    uint8_t rightCol = (e->oldx + e->width - 1) / 8;
    uint8_t topRow = (e->oldy - e->height + 1) / 8;
    uint8_t bottomRow = e->oldy / 8;

    for(uint8_t col = leftCol; col <=rightCol; col++){
        for(uint8_t row = topRow; row <= bottomRow; row++){
            uint16_t i = col + (row << 4);
            if(e->tilemap[i]==1){
          ST7735_DrawBitmap((i%16)*8, (((i/16)+1)*8)-1, yellowBlock, 8, 8);
          } else if(e->tilemap[i]==0) {
          ST7735_DrawBitmap((i%16)*8, (((i/16)+1)*8)-1, blueBlock, 8, 8);
          } else if(e->tilemap[i]==2) {
          ST7735_DrawBitmap((i%16)*8, (((i/16)+1)*8)-1, cloudBottom, 8, 8);
          } else if(e->tilemap[i]==3) {
          ST7735_DrawBitmap((i%16)*8, (((i/16)+1)*8)-1, cloudTop, 8, 8);
          }

        }
    }
    
}

void entityArrInit(Entity* entities){
    for(uint8_t i = 0; i < MAXENTITIES; i++){
        Entity newEntity;
        Entity_Init(&newEntity, 0, 0, 0, 0, 255, NULLARR16, NULLARR8);
        entities[i] = newEntity;
    }
}

Entity* addEntity(Entity* entities){
    for(uint8_t i = 0; i < MAXENTITIES; i++){
        if(!entities[i].active){
            Entity newEntity;
            entities[i] = newEntity;
            return &entities[i];
        }       
    }
        /*****IF THERE ARE NO AVAILABLE SLOTS, THE ENTITY WILL BE PLACES IN LAST SLOT*****/
        Entity newEntity;
        entities[MAXENTITIES-1] = newEntity;
        return &entities[MAXENTITIES-1];
}

void updateEntities(Entity* entities){
    for(uint8_t i = 0; i < MAXENTITIES; i++){
        Entity_Update(&entities[i]);
        //Entity_PrintSelf(&entities[i]);
    }
}

void drawEntities(Entity* entities){
    for(uint8_t i = 0; i < MAXENTITIES; i++){
        //Entity_Update(&entities[i]);
        Entity_PrintSelf(&entities[i]);
    }
}

void checkHoriCollisions(Entity* e, const uint8_t* tilemap){
    if(e->vx == 0) return;

    uint8_t leftCol = e->x / 8;
    uint8_t rightCol = (e->x + e->width - 1) / 8;
    uint8_t topRow = (e->y - e->height + 1) / 8;
    uint8_t bottomRow = e->y / 8;

    uint8_t row;
    uint16_t i;

    if(e->vx < 0){
        for(row = topRow; row <= bottomRow; row++){
            i = leftCol + (row << 4);
            if(tilemap[i] == 1){
                fixHoriCollision(e);
                return;
            }
        }
    }
    else if(e->vx > 0){
        for(row = topRow; row <= bottomRow; row++){
            i = rightCol + (row << 4);
            if(tilemap[i] == 1){
                fixHoriCollision(e);
                return;
            }
        }
    }
}

void checkVertCollisions(Entity* e, const uint8_t* tilemap){
    uint8_t leftCol = e->x / 8;
    uint8_t rightCol = (e->x + e->width - 1) / 8;
    uint8_t topRow = (e->y - e->height + 1) / 8;
    uint8_t bottomRow = e->y / 8;

    uint8_t col;
    uint16_t i;

    if(e->vy < 0){ // moving up
        for(col = leftCol; col <= rightCol; col++){
            i = col + (topRow << 4);
            if(tilemap[i] == 1){
                fixVertCollision(e);
                return;
            }
        }
    }
    else if(e->vy > 0){ // moving down
        for(col = leftCol; col <= rightCol; col++){
            i = col + (bottomRow << 4);
            if(tilemap[i] == 1){
                fixVertCollision(e);
                return;
            }
        }
    }
}

static void fixHoriCollision(Entity* e){

    if(e->vx > 0){ // moving right
        int tileX = (e->x + e->width - 1) / 8;
        e->x = tileX * 8 - e->width;
    }
    else if(e->vx < 0){ // moving left
        int tileX = e->x / 8;
        e->x = (tileX + 1) * 8;
    }

    e->vx = 0;
    e->vy = -5;
}

static void fixVertCollision(Entity* e){

    if(e->vy > 0){ // moving down (falling)
        int tileY = e->y / 8;
        e->y = tileY * 8 - 1;
        // grounded = 1;
    }
    else if(e->vy < 0){ // moving up (ceiling)
        int tileY = (e->y - e->height + 1) / 8;
        e->y = (tileY + 1) * 8 + e->height - 1;
    }

    e->vy = 0;
}
