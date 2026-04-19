#include "gamestate.h"
#include "entity.h"
#include "room.h"
#include "../inc/ST7735.h"
#include "graphics.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Switch.h"

#define MENU_OPTION_COUNT 2
#define LANGUAGE_OPTION_COUNT 2

// ------------------------------------------------------------
// Internal state
// ------------------------------------------------------------
static GameState gGameState = GAMESTATE_LANGUAGE_SELECT;
static Language gLanguage = LANGUAGE_ENGLISH;
static uint8_t gMainMenuSelection = 0;
static uint8_t gLanguageSelection = 0;
static uint8_t gGameplayLoaded = 0;
static uint8_t gScreenDirty = 1;
static bool Changed = 0;
static uint8_t gNeedsFullGameplayRedraw = 1;

static Room testRoom1, testRoom2, testRoom3, NULLROOM;

typedef enum {
    PLAYERSTYLE_MELEE = 0,
    PLAYERSTYLE_RANGED
} PlayerStyle;

typedef enum {
    TURNMODE_MOVE = 0,
    TURNMODE_ATTACK
} TurnMode;

static PlayerStyle gPlayerStyle = PLAYERSTYLE_MELEE;
static TurnMode gTurnMode = TURNMODE_MOVE;
static uint8_t gActionEnergyMax = 4;
static uint8_t gEnergyRemaining = 4;
static uint8_t gCursorX = 0;
static uint8_t gCursorY = 0;
static uint8_t gOldCursorX = 255;
static uint8_t gOldCursorY = 255;

// ------------------------------------------------------------
// External gameplay objects/functions you already have or will add
// ------------------------------------------------------------
extern Entity entList[MAXENTITIES];
extern Room* worldMap[MAXWORLD_SIZE][MAXWORLD_SIZE];
extern uint8_t worldX, worldY, oldWorldX, oldWorldY;
extern Entity* player;

extern void drawRoom(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE], uint8_t x, uint8_t y);
extern void drawEntities(Entity* entities,
                         Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE],
                         uint8_t roomX,
                         uint8_t roomY);

// Gameplay hooks you should implement elsewhere.
extern void World_InitINGAME(Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE]);
extern void INGAME_SpawnEntities(Entity* entities, Room* world[MAXWORLD_SIZE][MAXWORLD_SIZE]);
extern void INGAME_ResetPlayerPosition(void);

// ------------------------------------------------------------
// Internal helpers
// ------------------------------------------------------------
static void GameState_DrawLanguageSelect(void);
static void GameState_DrawMainMenu(void);
static void GameState_DrawGameplay(void);
static void GameState_EnterMainMenu(void);
static void GameState_StartINGAME(void);
static void GameState_HandlePressedLanguageSelect(GameButton button);
static void GameState_HandlePressedMainMenu(GameButton button);
static void GameState_HandlePressedGameplay(GameButton button);
static void GameState_HandleReleasedLanguageSelect(GameButton button);
static void GameState_HandleReleasedMainMenu(GameButton button);
static void GameState_HandleReleasedGameplay(GameButton button);
static void Gameplay_InitTurnState(void);
static void Gameplay_SetStyle(PlayerStyle style);
static void Gameplay_ResetCursorToPlayer(void);
static uint8_t Gameplay_CursorMove(int8_t dx, int8_t dy);
static uint8_t Gameplay_PreviewEnergyAfterMove(void);
static uint8_t Gameplay_PreviewEnergyAfterAttack(void);
static uint8_t Gameplay_CursorDistanceFromPlayer(void);
static uint8_t Gameplay_CanAttackTarget(void);
static Entity* Gameplay_FindAttackTargetAtCursor(void);
static void Gameplay_CommitMove(void);
static void Gameplay_CommitAttack(void);
static void Gameplay_DrawHUD(void);
static void Gameplay_DrawCursor(void);
static void Gameplay_DrawTargetMarker(void);

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------
void GameState_Init(void){
    gGameState = GAMESTATE_LANGUAGE_SELECT;
    gLanguage = LANGUAGE_ENGLISH;
    gMainMenuSelection = 0;
    gLanguageSelection = 0;
    gGameplayLoaded = 0;
}

bool inGame(void){
    return gGameplayLoaded;
}

GameState GameState_Get(void){
    return gGameState;
}

Language GameState_GetLanguage(void){
    return gLanguage;
}

void GameState_Set(GameState newState){
    gGameState = newState;
    gScreenDirty = 1;
}

void GameState_Update(void){
    // Event-driven version: no button polling here.
    // Keep this function in case you later want animations,
    // timers, or gameplay updates that run every tick.
    switch(gGameState){
        case GAMESTATE_LANGUAGE_SELECT:
            break;

        case GAMESTATE_MAIN_MENU:
            break;

        case GAMESTATE_INGAME:
            // Put gameplay tick updates here later.
            // Example:
            // updateEntities(entities);
            break;

        default:
            gGameState = GAMESTATE_LANGUAGE_SELECT;
            break;
    }
}

void GameState_Draw(void){
    if(Changed == 1){
        ST7735_FillScreen(ST7735_BLACK);
        Changed = 0;
    }
    switch(gGameState){
        case GAMESTATE_LANGUAGE_SELECT:
            if(gScreenDirty){
                GameState_DrawLanguageSelect();
                gScreenDirty = 0;
            }
            break;

        case GAMESTATE_MAIN_MENU:
            if(gScreenDirty){
                GameState_DrawMainMenu();
                gScreenDirty = 0;
            }
            break;

        case GAMESTATE_INGAME:
            GameState_DrawGameplay();
            break;

        default:
            break;
    }
}

void GameState_OnButtonPressed(GameButton button){
    switch(gGameState){
        case GAMESTATE_LANGUAGE_SELECT:
            GameState_HandlePressedLanguageSelect(button);
            break;

        case GAMESTATE_MAIN_MENU:
            GameState_HandlePressedMainMenu(button);
            break;

        case GAMESTATE_INGAME:
            GameState_HandlePressedGameplay(button);
            break;

        default:
            break;
    }
}

void GameState_OnButtonReleased(GameButton button){
    switch(gGameState){
        case GAMESTATE_LANGUAGE_SELECT:
            GameState_HandleReleasedLanguageSelect(button);
            break;

        case GAMESTATE_MAIN_MENU:
            GameState_HandleReleasedMainMenu(button);
            break;

        case GAMESTATE_INGAME:
            GameState_HandleReleasedGameplay(button);
            break;

        default:
            break;
    }
}

// ------------------------------------------------------------
// Language select handlers
// ------------------------------------------------------------
static void GameState_HandlePressedLanguageSelect(GameButton button){
    switch(button){
        case GAMEBUTTON_UP:
            if(gLanguageSelection > 0){
                gScreenDirty = 1;
                gLanguageSelection--;
            }
            break;

        case GAMEBUTTON_DOWN:
            if(gLanguageSelection + 1 < LANGUAGE_OPTION_COUNT){
                gScreenDirty = 1;
                gLanguageSelection++;
            }
            break;

        case GAMEBUTTON_A:
            if(gLanguageSelection == 0){
                gScreenDirty = 1;
                gLanguage = LANGUAGE_ENGLISH;
            } else {
                gScreenDirty = 1;
                gLanguage = LANGUAGE_SPANISH;
            }
            GameState_EnterMainMenu();
            break;

        default:
            break;
    }
}

static void GameState_HandleReleasedLanguageSelect(GameButton button){
    (void)button;
}

static void GameState_DrawLanguageSelect(void){
    if(gLanguage == LANGUAGE_ENGLISH){
        ST7735_SetCursor(2, 1);
        ST7735_OutString("Select Language   ");

        ST7735_SetCursor(2, 4);
        ST7735_OutString(gLanguageSelection == 0 ? "> English" : "  English");

        ST7735_SetCursor(2, 6);
        ST7735_OutString(gLanguageSelection == 1 ? "> Spanish" : "  Spanish");
    } else {
        /*ST7735_SetCursor(2, 1);
        ST7735_OutString("               ");*/
        ST7735_SetCursor(2, 1);
        ST7735_OutString("Seleccionar idioma");

        ST7735_SetCursor(2, 4);
        ST7735_OutString(gLanguageSelection == 0 ? "> Ingles " : "  Ingles ");

        ST7735_SetCursor(2, 6);
        ST7735_OutString(gLanguageSelection == 1 ? "> Espanol" : "  Espanol");
    }
}

// ------------------------------------------------------------
// Main menu handlers
// ------------------------------------------------------------
static void GameState_EnterMainMenu(void){
    gMainMenuSelection = 0;
    gGameState = GAMESTATE_MAIN_MENU;
    gScreenDirty = 1;
    Changed = 1;
}

static void GameState_HandlePressedMainMenu(GameButton button){
    switch(button){
        case GAMEBUTTON_UP:
            if(gMainMenuSelection > 0){
                gMainMenuSelection--;
                gScreenDirty = 1;
            }
            break;

        case GAMEBUTTON_DOWN:
            if(gMainMenuSelection + 1 < MENU_OPTION_COUNT){
                gMainMenuSelection++;
                gScreenDirty = 1;
            }
            break;

        case GAMEBUTTON_A:
            switch(gMainMenuSelection){
                case 0:
                    gScreenDirty = 1;
                    GameState_StartINGAME();
                    ST7735_FillScreen(ST7735_BLACK);
                    break;

                case 1:
                    gScreenDirty = 1;
                    gGameState = GAMESTATE_LANGUAGE_SELECT;
                    Changed = 1;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

static void GameState_HandleReleasedMainMenu(GameButton button){
    (void)button;
}

static void GameState_DrawMainMenu(void){
    
    ST7735_DrawBitmap(0, 159, logo, 128, 80);
    if(gLanguage == LANGUAGE_ENGLISH){
        ST7735_SetCursor(3, 1);
        ST7735_OutString("Main Menu");

        ST7735_SetCursor(2, 4);
        ST7735_OutString(gMainMenuSelection == 0 ? "> Level 1" : "  Level 1");

        ST7735_SetCursor(2, 6);
        ST7735_OutString(gMainMenuSelection == 1 ? "> Language" : "  Language");
    } else {
        ST7735_SetCursor(3, 1);
        char str[20];
        sprintf(str, "Men%c principal", 0xA3);
        ST7735_OutString(str);

        ST7735_SetCursor(2, 4);
        ST7735_OutString(gMainMenuSelection == 0 ? "> Nivel 1" : "  Nivel 1");

        ST7735_SetCursor(2, 6);
        ST7735_OutString(gMainMenuSelection == 1 ? "> Idioma" : "  Idioma");
    }
}

static void GameState_StartINGAME(void){
    if(!gGameplayLoaded){
        entityArrInit(entList);
        worldX = worldY = 0;
        oldWorldX = oldWorldY = 255;
        roomInit(&testRoom1, tilemap1);
        roomInit(&testRoom2, tilemap2);
        roomInit(&testRoom3, tilemap3);
        roomInit(&NULLROOM, tilemap1);
        worldInit(worldMap, &NULLROOM);
        setWorld(worldMap, &testRoom1, 0, 0);
        setWorld(worldMap, &testRoom2, 1, 0);
        setWorld(worldMap, &testRoom3, 1, 1);
        Entity *block = addEntity(entList);
        player = block;
        Entity_Init(block, 0, 0, 8, 8, PLAYER, happyBlock);
        Entity_Activate(block);
        Gameplay_SetStyle(PLAYERSTYLE_MELEE);
        Gameplay_InitTurnState();

        Entity *tp = addEntity(entList);
        Entity_Init(tp, 6, 4, 12, 12, TELEPORTER, teleporter_Active);
        Entity_Activate(tp);

        Entity *enemy = addEntity(entList);
        Entity_Init(enemy, 4, 2, 8, 8, ENEMY, blueBlock);
        Entity_Activate(enemy);

        gGameplayLoaded = 1;
    }

    Gameplay_InitTurnState();
    gNeedsFullGameplayRedraw = 1;
    gGameState = GAMESTATE_INGAME;
}

// ------------------------------------------------------------
// Gameplay handlers
// ------------------------------------------------------------
static void GameState_HandlePressedGameplay(GameButton button){
    switch(button){
        case GAMEBUTTON_UP:
            Gameplay_CursorMove(0, -1);
            break;

        case GAMEBUTTON_DOWN:
            Gameplay_CursorMove(0, 1);
            break;

        case GAMEBUTTON_LEFT:
            Gameplay_CursorMove(-1, 0);
            break;

        case GAMEBUTTON_RIGHT:
            Gameplay_CursorMove(1, 0);
            break;

        case GAMEBUTTON_A:
            if(gTurnMode == TURNMODE_MOVE){
                Gameplay_CommitMove();
            } else {
                Gameplay_CommitAttack();
            }
            break;

        case GAMEBUTTON_B:
            gTurnMode = (gTurnMode == TURNMODE_MOVE) ? TURNMODE_ATTACK : TURNMODE_MOVE;
            gNeedsFullGameplayRedraw = 1;
            break;

        case GAMEBUTTON_ALT:
            Gameplay_SetStyle((gPlayerStyle == PLAYERSTYLE_MELEE) ? PLAYERSTYLE_RANGED : PLAYERSTYLE_MELEE);
            gNeedsFullGameplayRedraw = 1;
            break;

        case GAMEBUTTON_ESC:
            gGameState = GAMESTATE_MAIN_MENU;
            break;

        default:
            break;
    }
}

static void GameState_HandleReleasedGameplay(GameButton button){
    (void)button;
}

static void GameState_DrawGameplay(void){
    if(gNeedsFullGameplayRedraw || oldWorldX != worldX || oldWorldY != worldY){
      drawRoom(worldMap, worldX, worldY);
      oldWorldX = worldX;
      oldWorldY = worldY;
      gNeedsFullGameplayRedraw = 0;
    }
    drawEntities(entList, worldMap, worldX, worldY);
    Gameplay_DrawTargetMarker();
    Gameplay_DrawCursor();
    Gameplay_DrawHUD();
}

static void Gameplay_InitTurnState(void){
    gEnergyRemaining = gActionEnergyMax;
    gTurnMode = TURNMODE_MOVE;
    Gameplay_ResetCursorToPlayer();
}

static void Gameplay_SetStyle(PlayerStyle style){
    gPlayerStyle = style;
    if(style == PLAYERSTYLE_MELEE){
        gActionEnergyMax = 4;
        Entity_SetBitmap(player, happyBlock);
    } else {
        gActionEnergyMax = 5;
        Entity_SetBitmap(player, yellowBlock);
    }
    gEnergyRemaining = gActionEnergyMax;
}

static void Gameplay_ResetCursorToPlayer(void){
    gCursorX = player->tileX;
    gCursorY = player->tileY;
    gOldCursorX = 255;
    gOldCursorY = 255;
}

static uint8_t Gameplay_CursorMove(int8_t dx, int8_t dy){
    int8_t nextX = (int8_t)gCursorX + dx;
    int8_t nextY = (int8_t)gCursorY + dy;
    if(nextX < 0 || nextX > 7 || nextY < 0 || nextY > 7){
        return 0;
    }
    gCursorX = (uint8_t)nextX;
    gCursorY = (uint8_t)nextY;
    return 1;
}

static uint8_t Gameplay_CursorDistanceFromPlayer(void){
    return (uint8_t)(abs((int16_t)gCursorX - player->tileX) + abs((int16_t)gCursorY - player->tileY));
}

static uint8_t Gameplay_PreviewEnergyAfterMove(void){
    uint8_t moveCost = Gameplay_CursorDistanceFromPlayer();
    if(moveCost > gEnergyRemaining){
        return gEnergyRemaining;
    }
    return gEnergyRemaining - moveCost;
}

static uint8_t Gameplay_PreviewEnergyAfterAttack(void){
    uint8_t attackCost = (gPlayerStyle == PLAYERSTYLE_MELEE) ? 2 : 1;
    if(attackCost > gEnergyRemaining){
        return gEnergyRemaining;
    }
    return gEnergyRemaining - attackCost;
}

static Entity* Gameplay_FindAttackTargetAtCursor(void){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        if(!entList[i].active) continue;
        if(entList[i].type != ENEMY) continue;
        if(entList[i].tileX == gCursorX && entList[i].tileY == gCursorY){
            return &entList[i];
        }
    }
    return 0;
}

static uint8_t Gameplay_CanAttackTarget(void){
    uint8_t range = (gPlayerStyle == PLAYERSTYLE_MELEE) ? 1 : 4;
    Entity* target = Gameplay_FindAttackTargetAtCursor();
    if(!target) return 0;
    if(Gameplay_CursorDistanceFromPlayer() > range) return 0;
    return 1;
}

static void Gameplay_CommitMove(void){
    const uint32_t* tileMap;
    uint8_t moveCost;
    if(gCursorX == player->tileX && gCursorY == player->tileY){
        gTurnMode = TURNMODE_ATTACK;
        return;
    }
    moveCost = Gameplay_CursorDistanceFromPlayer();
    if(moveCost > gEnergyRemaining){
        return;
    }
    tileMap = getTileMap(worldMap, worldX, worldY);
    if(tileMap[(gCursorY * 8) + gCursorX] == 1){
        return;
    }
    Entity_SetTilePosition(player, gCursorX, gCursorY);
    gEnergyRemaining = gEnergyRemaining - moveCost;
    gTurnMode = TURNMODE_ATTACK;
}

static void Gameplay_CommitAttack(void){
    uint8_t attackCost = (gPlayerStyle == PLAYERSTYLE_MELEE) ? 2 : 1;
    Entity* target;
    if(gEnergyRemaining < attackCost) return;
    if(!Gameplay_CanAttackTarget()) return;
    target = Gameplay_FindAttackTargetAtCursor();
    Entity_Deactivate(target);
    gEnergyRemaining = gEnergyRemaining - attackCost;
    gEnergyRemaining = gActionEnergyMax;
    gTurnMode = TURNMODE_MOVE;
    Gameplay_ResetCursorToPlayer();
}

static void Gameplay_DrawHUD(void){
    uint8_t previewEnergy;
    ST7735_SetCursor(0, 10);
    ST7735_OutString((gPlayerStyle == PLAYERSTYLE_MELEE) ? "Melee " : "Range ");
    ST7735_SetCursor(7, 10);
    ST7735_OutString((gTurnMode == TURNMODE_MOVE) ? "Move " : "Atk  ");
    ST7735_SetCursor(0, 11);
    ST7735_OutString("E:");
    ST7735_OutUDec(gEnergyRemaining);
    ST7735_OutString("->");
    previewEnergy = (gTurnMode == TURNMODE_MOVE) ? Gameplay_PreviewEnergyAfterMove() : Gameplay_PreviewEnergyAfterAttack();
    ST7735_OutUDec(previewEnergy);
    ST7735_OutString("  ");
}

static void Gameplay_DrawCursor(void){
    uint8_t cursorDrawX;
    uint8_t cursorDrawY;
    if(gOldCursorX != 255 && (gOldCursorX != gCursorX || gOldCursorY != gCursorY)){
        drawRoomTile(worldMap, worldX, worldY, gOldCursorX, gOldCursorY);
    }
    cursorDrawX = (uint8_t)(gCursorX * 12 + 2);
    cursorDrawY = (uint8_t)(gCursorY * 12 + 9);
    ST7735_DrawBitmap(cursorDrawX, cursorDrawY, yellowBlock, 8, 8);
    gOldCursorX = gCursorX;
    gOldCursorY = gCursorY;
}

static void Gameplay_DrawTargetMarker(void){
    if(gTurnMode == TURNMODE_ATTACK && Gameplay_CanAttackTarget()){
        ST7735_DrawBitmap(gCursorX * 12, (gCursorY + 1) * 12 - 1, redtile, 12, 12);
    }
}
