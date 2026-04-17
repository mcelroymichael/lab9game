#include "entity.h"
#include "../inc/ST7735.h"
#include "graphics.h"
#include "room.h"

#define MAP_WIDTH      8
#define MAP_HEIGHT     8
#define TILE_SIZE      12
#define BLOCKED_TILE   1u

extern Entity* player;
extern uint8_t worldX, worldY;

static uint8_t Map_InBounds(int8_t col, int8_t row);
static uint16_t Map_Index(uint8_t col, uint8_t row);
static uint32_t Map_GetTile(const uint32_t* tilemap, uint8_t col, uint8_t row);
static uint8_t Map_IsPassable(const uint32_t* tilemap, int8_t col, int8_t row);
static int16_t Entity_DrawX(const Entity* e);
static int16_t Entity_DrawY(const Entity* e);
static void Entity_PrintSelf(Entity* e, Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t roomX, uint8_t roomY);

void Entity_Init(Entity* e, uint8_t tileX, uint8_t tileY, uint8_t width, uint8_t height, uint8_t type, const uint16_t* bitmap){
    if(!e) return;

    e->active = 0;

    // Tile/grid position is the gameplay position.
    e->tileX = tileX;
    e->tileY = tileY;
    e->oldTileX = 255;
    e->oldTileY = 255;

    // Kept only for compatibility/future animation use.
    e->vx = 0;
    e->vy = 0;

    e->width = width;
    e->height = height;
    e->type = type;
    e->bitmap = bitmap;
}

// **********Entity_Activate*******************
void Entity_Activate(Entity* e){
    if(!e) return;
    e->active = 1;
}

// **********Entity_Deactivate*******************
void Entity_Deactivate(Entity* e){
    if(!e) return;
    e->active = 0;
}

// **********Entity_Update*******************
// Grid movement should happen only through Entity_TryMove.
void Entity_Update(Entity* e){
    if(!e || !e->active) return;
    switch (e->type) {
        
        case PLAYER:
            break;

        case ENEMY:
            break;
        
        case TELEPORTER: {
            const TeleporterData* tpdata = &teleportTable[worldX][worldY];
            if((e->tileX != tpdata->currentX) || (e->tileY != tpdata->currentY)){
                e->tileX = tpdata->currentX;
                e->tileY = tpdata->currentY;
            }

            if((player->tileX == e->tileX) && (player->tileY == e->tileY)){
                const TeleporterData* nexttpdata = &teleportTable[tpdata->destinationWorldX][tpdata->destinationWorldY];
                worldX = tpdata->destinationWorldX;
                worldY = tpdata->destinationWorldY;
                player->tileX = tpdata->destinationPlayerX;
                player->tileY = tpdata->destinationPlayerY;
                e->tileX = nexttpdata->currentX;
                e->tileY = nexttpdata->currentY;
            }
            break;
        }

        default:
            break;
        }
}

// **********Entity_SetTilePosition*******************
void Entity_SetTilePosition(Entity* e, uint8_t tileX, uint8_t tileY){
    if(!e) return;
    e->tileX = tileX;
    e->tileY = tileY;
}

// **********Entity_TryMove*******************
// Attempts to move one tile. Returns 1 on success, 0 if blocked/out of bounds.
uint8_t Entity_TryMove(Entity* e, int8_t dx, int8_t dy, const uint32_t* roomTilemap){
    int8_t newCol;
    int8_t newRow;

    if(!e || !e->active || !roomTilemap) return 0;

    newCol = (int8_t)e->tileX + dx;
    newRow = (int8_t)e->tileY + dy;

    if(!Map_InBounds(newCol, newRow)){
        return 2;
    }

    if(!Map_IsPassable(roomTilemap, newCol, newRow)){
        return 0;
    }

    e->tileX = (uint8_t)newCol;
    e->tileY = (uint8_t)newRow;
    return 1;
}

// **********Entity_SetBitmap*******************
void Entity_SetBitmap(Entity* e, const uint16_t* bitmap){
    if(!e) return;
    e->bitmap = bitmap;
}

// **********Entity_PrintSelf*******************
// Displays the entity on the screen.
static void Entity_PrintSelf(Entity* e, Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t roomX, uint8_t roomY){
    int16_t drawX;
    int16_t drawY;

    if(!e || !e->active || !e->bitmap) return;

    // redraw OLD tile if moved
    if(e->oldTileX != e->tileX || e->oldTileY != e->tileY){
        if(e->oldTileX < MAP_WIDTH && e->oldTileY < MAP_HEIGHT){
            drawRoomTile(world, roomX, roomY, e->oldTileX, e->oldTileY);
        }
    }

    // draw entity at new position
    drawX = Entity_DrawX(e);
    drawY = Entity_DrawY(e);
    ST7735_DrawBitmap(drawX, drawY, e->bitmap, e->width, e->height);

    // sync old position AFTER redraw
    e->oldTileX = e->tileX;
    e->oldTileY = e->tileY;
}

// **********Entity_DrawX*******************
// Centers a smaller sprite inside a 12x12 tile.
static int16_t Entity_DrawX(const Entity* e){
    int16_t tilePixelX = (int16_t)e->tileX * TILE_SIZE;
    return tilePixelX + ((int16_t)TILE_SIZE - (int16_t)e->width) / 2;
}

// **********Entity_DrawY*******************
// ST7735_DrawBitmap uses bottom-left as its Y anchor.
// This centers a smaller sprite inside a 12x12 tile.
static int16_t Entity_DrawY(const Entity* e){
    int16_t tileTopY = (int16_t)e->tileY * TILE_SIZE;
    int16_t spriteTopY = tileTopY + ((int16_t)TILE_SIZE - (int16_t)e->height) / 2;
    return spriteTopY + e->height - 1;
}

// **********entityArrInit*******************
void entityArrInit(Entity* entities){
    uint8_t i;

    if(!entities) return;

    for(i = 0; i < MAXENTITIES; i++){
        Entity_Init(&entities[i], 0, 0, 8, 8, 255, NULLARR16);
    }
}

// **********addEntity*******************
// Returns an initialized inactive slot.
Entity* addEntity(Entity* entities){
    uint8_t i;

    if(!entities) return 0;

    for(i = 0; i < MAXENTITIES; i++){
        if(!entities[i].active){
            Entity_Init(&entities[i], 0, 0, 8, 8, 255, NULLARR16);
            return &entities[i];
        }
    }

    // Fallback: reuse last slot.
    Entity_Init(&entities[MAXENTITIES - 1], 0, 0, 8, 8, 255, NULLARR16);
    return &entities[MAXENTITIES - 1];
}

// **********updateEntities*******************
void updateEntities(Entity* entities){
    uint8_t i;

    if(!entities) return;

    for(i = 0; i < MAXENTITIES; i++){
        Entity_Update(&entities[i]);
    }
}

// **********drawEntities*******************
void drawEntities(Entity* entities, Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t roomX, uint8_t roomY){
    uint8_t i;

    if(!entities) return;

    for(i = 0; i < MAXENTITIES; i++){
        Entity_PrintSelf(&entities[i], world, roomX, roomY);
    }
}


// ============================================================
// Map helpers
// ============================================================

// **********Map_InBounds*******************
static uint8_t Map_InBounds(int8_t col, int8_t row){
    return (col >= 0 && col < MAP_WIDTH && row >= 0 && row < MAP_HEIGHT);
}

// **********Map_Index*******************
static uint16_t Map_Index(uint8_t col, uint8_t row){
    return (uint16_t)row * MAP_WIDTH + col;
}

// **********Map_GetTile*******************
static uint32_t Map_GetTile(const uint32_t* tilemap, uint8_t col, uint8_t row){
    return tilemap[Map_Index(col, row)];
}

// **********Map_IsPassable*******************
static uint8_t Map_IsPassable(const uint32_t* tilemap, int8_t col, int8_t row){
    if(!tilemap) return 0;
    if(!Map_InBounds(col, row)) return 0;
    return (Map_GetTile(tilemap, (uint8_t)col, (uint8_t)row) != BLOCKED_TILE);
}

