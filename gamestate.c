#include "gamestate.h"
#include "entity.h"
#include "room.h"
#include "../inc/ST7735.h"
#include "graphics.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Switch.h"

extern volatile uint32_t potState;
extern uint32_t Convert(uint32_t input);

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
static uint8_t gForceHUDRedraw = 1;
static uint8_t gForceHealthRedraw = 1;

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
static uint8_t gMovementEnergyMax = 6;
static uint8_t gEnergyRemaining = 6;
static uint8_t gMoveEnergyRemaining = 6;
static uint8_t gAttackEnergyRemaining = 0;
static uint8_t gEnergySplitLocked = 0;
static uint8_t gMoveCommittedThisTurn = 0;
static uint32_t gLatestADC = 0;
static uint32_t gLatestADCCm = 0;
static uint8_t gPlayerHealthMax = 10;
static uint8_t gPlayerHealth = 10;
static uint8_t gSelectedAttackMove = 0;
static uint8_t gCursorX = 0;
static uint8_t gCursorY = 0;
static uint8_t gOldCursorX = 255;
static uint8_t gOldCursorY = 255;
static uint8_t gAwaitingEnemyTurnAck = 0;
static uint8_t gEnemyTurnSummaryVisible = 0;
static uint8_t gEnemyTurnHPBefore = 0;
static uint8_t gEnemyTurnHPAfter = 0;
static uint8_t gEnemyTurnDamageTaken = 0;
static uint8_t gEnemyTurnEnemiesMoved = 0;
static uint8_t gEnemyTurnEnemiesAttacked = 0;
static uint8_t gEnemyTurnSummaryDirty = 1;
static uint8_t gCurrentStage = 1;
static uint8_t gStageLoadedWorldX = 255;
static uint8_t gStageLoadedWorldY = 255;

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
static void GameState_DrawEnding(void);
static void GameState_EnterMainMenu(void);
static void GameState_StartINGAME(void);
static void GameState_HandlePressedLanguageSelect(GameButton button);
static void GameState_HandlePressedMainMenu(GameButton button);
static void GameState_HandlePressedGameplay(GameButton button);
static void GameState_HandleReleasedLanguageSelect(GameButton button);
static void GameState_HandleReleasedMainMenu(GameButton button);
static void GameState_HandleReleasedGameplay(GameButton button);
static void GameState_HandlePressedEnding(GameButton button);
static void GameState_HandleReleasedEnding(GameButton button);
static void Gameplay_InitTurnState(void);
static void Gameplay_SetStyle(PlayerStyle style);
static void Gameplay_ResetCursorToPlayer(void);
static uint8_t Gameplay_CursorMove(int8_t dx, int8_t dy);
static uint8_t Gameplay_PreviewEnergyAfterMove(void);
static uint8_t Gameplay_PreviewEnergyAfterAttack(void);
static uint8_t Gameplay_CurrentModeEnergyRemaining(void);
static void Gameplay_UpdateEnergySplitFromADC(void);
static uint8_t Gameplay_CursorDistanceFromPlayer(void);
static uint8_t Gameplay_CanAttackTarget(void);
static Entity* Gameplay_FindAttackTargetAtCursor(void);
static Entity* Gameplay_FindEntityAtTile(uint8_t tileX, uint8_t tileY);
static uint8_t Gameplay_IsEnemyBlockingTile(uint8_t tileX, uint8_t tileY);
static uint8_t Gameplay_IsTileTraversableForMove(uint8_t tileX, uint8_t tileY);
static uint8_t Gameplay_IsTileReachableByPlayer(uint8_t targetX, uint8_t targetY, uint8_t maxCost, uint8_t* outCost);
static void Gameplay_CommitMove(void);
static void Gameplay_CommitAttack(void);
static void Gameplay_DrawHUD(void);
static void Gameplay_DrawRangeHighlights(void);
static void Gameplay_DrawHighlightAtTile(uint8_t tileX, uint8_t tileY);
static void Gameplay_DrawEntityAtTile(uint8_t tileX, uint8_t tileY);
static void Gameplay_RedrawTile(uint8_t tileX, uint8_t tileY, uint8_t includeCursor);
static void Gameplay_DrawCursor(void);
static void Gameplay_DrawTargetMarker(void);
static void Gameplay_DrawHealthBar(void);
static void Gameplay_EndPlayerTurn(void);
static void Gameplay_RunEnemyTurn(void);
static void Gameplay_DrawEnemyTurnSummary(void);
static void Gameplay_AcknowledgeEnemyTurnSummary(void);
static void Gameplay_ClearHUDRow(uint8_t row);
static void Gameplay_UpdateTeleporterState(void);
static void Gameplay_LoadStage(uint8_t stage);
static void Gameplay_ClearEnemies(void);
static void Gameplay_SpawnEnemyAt(uint8_t tileX, uint8_t tileY, uint8_t hp, uint8_t attackPower, const uint16_t* sprite);
static uint8_t Gameplay_IsPlayerAdjacentToEnemy(const Entity* enemy);
static void Gameplay_EnemyTryMoveTowardsPlayer(Entity* enemy);
static uint8_t Gameplay_IsTileOccupiedByEnemy(uint8_t tileX, uint8_t tileY, const Entity* ignoreEnemy);
static uint8_t Gameplay_HasAliveEnemies(void);
static void Gameplay_Shutdown(void);

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
        case GAMESTATE_ENDING:
            if(gScreenDirty){
                GameState_DrawEnding();
                gScreenDirty = 0;
            }
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
        case GAMESTATE_ENDING:
            GameState_HandlePressedEnding(button);
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
        case GAMESTATE_ENDING:
            GameState_HandleReleasedEnding(button);
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
        tp->data0 = 0;
        Entity_SetBitmap(tp, teleporter_Inactive);
        Entity_Activate(tp);

        gGameplayLoaded = 1;
    }

    gCurrentStage = 1;
    Gameplay_LoadStage(gCurrentStage);
    gStageLoadedWorldX = worldX;
    gStageLoadedWorldY = worldY;
    Gameplay_InitTurnState();
    Gameplay_UpdateTeleporterState();
    gNeedsFullGameplayRedraw = 1;
    gGameState = GAMESTATE_INGAME;
}

// ------------------------------------------------------------
// Gameplay handlers
// ------------------------------------------------------------
static void GameState_HandlePressedGameplay(GameButton button){
    if(gAwaitingEnemyTurnAck){
        if(button == GAMEBUTTON_A){
            Gameplay_AcknowledgeEnemyTurnSummary();
        }
        return;
    }

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
            Gameplay_EndPlayerTurn();
            break;

        case GAMEBUTTON_ALT:
            gSelectedAttackMove = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? PLAYERSTYLE_RANGED : PLAYERSTYLE_MELEE;
            Gameplay_SetStyle((PlayerStyle)gSelectedAttackMove);
            gNeedsFullGameplayRedraw = 1;
            break;

        case GAMEBUTTON_ESC:
            Gameplay_Shutdown();
            GameState_EnterMainMenu();
            break;

        default:
            break;
    }
}

static void GameState_HandleReleasedGameplay(GameButton button){
    (void)button;
}

static void GameState_HandlePressedEnding(GameButton button){
    if(button == GAMEBUTTON_A || button == GAMEBUTTON_ESC){
        Gameplay_Shutdown();
        GameState_EnterMainMenu();
    }
}

static void GameState_HandleReleasedEnding(GameButton button){
    (void)button;
}

static void GameState_DrawGameplay(void){
    if(gStageLoadedWorldX != worldX || gStageLoadedWorldY != worldY){
        if(gCurrentStage >= 3){
            gGameState = GAMESTATE_ENDING;
            gScreenDirty = 1;
            Changed = 1;
            return;
        }
        gCurrentStage++;
        Gameplay_LoadStage(gCurrentStage);
        gStageLoadedWorldX = worldX;
        gStageLoadedWorldY = worldY;
        Gameplay_InitTurnState();
        Gameplay_UpdateTeleporterState();
        gNeedsFullGameplayRedraw = 1;
    }

    uint8_t cursorMoved = (uint8_t)(gOldCursorX != gCursorX || gOldCursorY != gCursorY);
    if(gNeedsFullGameplayRedraw || oldWorldX != worldX || oldWorldY != worldY){
      drawRoom(worldMap, worldX, worldY);
      Gameplay_DrawRangeHighlights();
      drawEntities(entList, worldMap, worldX, worldY);
      Gameplay_DrawTargetMarker();
      Gameplay_DrawCursor();
      gForceHUDRedraw = 1;
      gForceHealthRedraw = 1;
      oldWorldX = worldX;
      oldWorldY = worldY;
      gNeedsFullGameplayRedraw = 0;
    } else if(cursorMoved){
      if(gOldCursorX < 8 && gOldCursorY < 8){
        Gameplay_RedrawTile(gOldCursorX, gOldCursorY, 0);
      }
      Gameplay_RedrawTile(gCursorX, gCursorY, 1);
    } else {
      Gameplay_DrawTargetMarker();
      Gameplay_DrawCursor();
    }
    drawEntities(entList, worldMap, worldX, worldY);
    Gameplay_DrawTargetMarker();
    Gameplay_DrawCursor();
    Gameplay_DrawHUD();
    Gameplay_DrawHealthBar();
}

static void Gameplay_InitTurnState(void){
    gEnergyRemaining = gMovementEnergyMax;
    gMoveEnergyRemaining = gMovementEnergyMax;
    gAttackEnergyRemaining = 0;
    gEnergySplitLocked = 0;
    gMoveCommittedThisTurn = 0;
    Gameplay_UpdateEnergySplitFromADC();
    gTurnMode = TURNMODE_MOVE;
    gSelectedAttackMove = gPlayerStyle;
    gAwaitingEnemyTurnAck = 0;
    gEnemyTurnSummaryVisible = 0;
    gEnemyTurnSummaryDirty = 1;
    gForceHUDRedraw = 1;
    gForceHealthRedraw = 1;
    Gameplay_ResetCursorToPlayer();
}

static void Gameplay_SetStyle(PlayerStyle style){
    gPlayerStyle = style;
    if(style == PLAYERSTYLE_MELEE){
        Entity_SetBitmap(player, happyBlock);
    } else {
        Entity_SetBitmap(player, happyBlue);
    }
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
    gNeedsFullGameplayRedraw = 1;
    return 1;
}

static uint8_t Gameplay_CursorDistanceFromPlayer(void){
    return (uint8_t)(abs((int16_t)gCursorX - player->tileX) + abs((int16_t)gCursorY - player->tileY));
}

static uint8_t Gameplay_PreviewEnergyAfterMove(void){
    uint8_t moveCost = 255;
    uint8_t moveEnergy = Gameplay_CurrentModeEnergyRemaining();
    if(!Gameplay_IsTileReachableByPlayer(gCursorX, gCursorY, moveEnergy, &moveCost)){
        return 255;
    }
    return moveEnergy - moveCost;
}

static uint8_t Gameplay_PreviewEnergyAfterAttack(void){
    uint8_t attackCost = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? 2 : 1;
    uint8_t attackEnergy = Gameplay_CurrentModeEnergyRemaining();
    if(attackCost > attackEnergy){
        return 255;
    }
    return attackEnergy - attackCost;
}

static uint8_t Gameplay_CurrentModeEnergyRemaining(void){
    if(gTurnMode == TURNMODE_MOVE){
        return gMoveEnergyRemaining;
    }
    return gAttackEnergyRemaining;
}

static void Gameplay_UpdateEnergySplitFromADC(void){
    uint32_t raw;
    uint8_t moveEnergy;
    if(gEnergySplitLocked){
        return;
    }
    // Consume the latest ADC sample captured by the engine tick.
    raw = potState;
    if(raw > 4095u){
        raw = 4095u;
    }
    gLatestADC = raw;
    gLatestADCCm = Convert(raw);
    moveEnergy = (uint8_t)((raw * (gMovementEnergyMax + 1u)) / 4096u);
    if(moveEnergy > gMovementEnergyMax){
        moveEnergy = gMovementEnergyMax;
    }
    gMoveEnergyRemaining = moveEnergy;
    gAttackEnergyRemaining = (uint8_t)(gMovementEnergyMax - moveEnergy);
    gEnergyRemaining = (uint8_t)(gMoveEnergyRemaining + gAttackEnergyRemaining);
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
    uint8_t range = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? 1 : 4;
    Entity* target = Gameplay_FindAttackTargetAtCursor();
    if(!target) return 0;
    if(Gameplay_CursorDistanceFromPlayer() > range) return 0;
    return 1;
}

static Entity* Gameplay_FindEntityAtTile(uint8_t tileX, uint8_t tileY){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        if(!entList[i].active) continue;
        if(entList[i].tileX == tileX && entList[i].tileY == tileY){
            return &entList[i];
        }
    }
    return 0;
}

static uint8_t Gameplay_IsEnemyBlockingTile(uint8_t tileX, uint8_t tileY){
    Entity* e = Gameplay_FindEntityAtTile(tileX, tileY);
    if(!e) return 0;
    return (e->type == ENEMY);
}

static uint8_t Gameplay_IsTileTraversableForMove(uint8_t tileX, uint8_t tileY){
    const uint32_t* tileMap = getTileMap(worldMap, worldX, worldY);
    uint8_t idx = (uint8_t)((tileY * 8) + tileX);
    if(tileMap[idx] == 1){
        return 0;
    }
    if(Gameplay_IsEnemyBlockingTile(tileX, tileY)){
        return 0;
    }
    return 1;
}

static uint8_t Gameplay_IsTileReachableByPlayer(uint8_t targetX, uint8_t targetY, uint8_t maxCost, uint8_t* outCost){
    uint8_t dist[8][8];
    uint8_t qx[64];
    uint8_t qy[64];
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t x;
    uint8_t y;
    int8_t dx;
    int8_t dy;
    for(y = 0; y < 8; y++){
        for(x = 0; x < 8; x++){
            dist[y][x] = 255;
        }
    }

    dist[player->tileY][player->tileX] = 0;
    qx[tail] = player->tileX;
    qy[tail] = player->tileY;
    tail++;

    while(head < tail){
        uint8_t curX;
        uint8_t curY;
        uint8_t curDist;
        curX = qx[head];
        curY = qy[head];
        curDist = dist[curY][curX];
        head++;
        if(curDist >= maxCost){
            continue;
        }
        for(dy = -1; dy <= 1; dy++){
            for(dx = -1; dx <= 1; dx++){
                int8_t nextX;
                int8_t nextY;
                if((dx == 0 && dy == 0) || (dx != 0 && dy != 0)){
                    continue;
                }
                nextX = (int8_t)curX + dx;
                nextY = (int8_t)curY + dy;
                if(nextX < 0 || nextX > 7 || nextY < 0 || nextY > 7){
                    continue;
                }
                if((uint8_t)nextX == targetX && (uint8_t)nextY == targetY){
                    if(!Gameplay_IsTileTraversableForMove((uint8_t)nextX, (uint8_t)nextY) &&
                        !((uint8_t)nextX == player->tileX && (uint8_t)nextY == player->tileY)){
                        continue;
                    }
                } else if(!Gameplay_IsTileTraversableForMove((uint8_t)nextX, (uint8_t)nextY)){
                    continue;
                }
                if(dist[(uint8_t)nextY][(uint8_t)nextX] != 255){
                    continue;
                }
                dist[(uint8_t)nextY][(uint8_t)nextX] = curDist + 1;
                qx[tail] = (uint8_t)nextX;
                qy[tail] = (uint8_t)nextY;
                tail++;
            }
        }
    }
    if(outCost){
        *outCost = dist[targetY][targetX];
    }
    return (dist[targetY][targetX] != 255 && dist[targetY][targetX] <= maxCost);
}

static void Gameplay_CommitMove(void){
    uint8_t moveCost = 255;
    if(!gEnergySplitLocked){
        gEnergySplitLocked = 1;
    }
    if(gMoveCommittedThisTurn){
        return;
    }
    if(gCursorX == player->tileX && gCursorY == player->tileY){
        gTurnMode = TURNMODE_ATTACK;
        gNeedsFullGameplayRedraw = 1;
        return;
    }
    if(!Gameplay_IsTileReachableByPlayer(gCursorX, gCursorY, gMoveEnergyRemaining, &moveCost)){
        return;
    }
    Entity_SetTilePosition(player, gCursorX, gCursorY);
    gMoveEnergyRemaining = (uint8_t)(gMoveEnergyRemaining - moveCost);
    gEnergyRemaining = (uint8_t)(gMoveEnergyRemaining + gAttackEnergyRemaining);
    gMoveCommittedThisTurn = 1;
    if(gAttackEnergyRemaining == 0){
        Gameplay_EndPlayerTurn();
        return;
    }
    gTurnMode = TURNMODE_ATTACK;
    gNeedsFullGameplayRedraw = 1;
}

static void Gameplay_CommitAttack(void){
    uint8_t attackCost = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? 2 : 1;
    uint8_t attackDamage = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? 5 : 1;
    Entity* target;
    if(!gEnergySplitLocked){
        gEnergySplitLocked = 1;
    }
    if(gAttackEnergyRemaining == 0){
        Gameplay_EndPlayerTurn();
        return;
    }
    if(gAttackEnergyRemaining < attackCost) return;
    if(!Gameplay_CanAttackTarget()) return;
    target = Gameplay_FindAttackTargetAtCursor();
    if(!target) return;
    if(attackDamage >= target->data0){
        target->data0 = 0;
        Entity_Deactivate(target);
    } else {
        target->data0 = target->data0 - attackDamage;
    }
    Gameplay_UpdateTeleporterState();
    gAttackEnergyRemaining = (uint8_t)(gAttackEnergyRemaining - attackCost);
    gEnergyRemaining = (uint8_t)(gMoveEnergyRemaining + gAttackEnergyRemaining);
    if(gAttackEnergyRemaining == 0){
        Gameplay_EndPlayerTurn();
        return;
    }
    gNeedsFullGameplayRedraw = 1;
}

static void Gameplay_DrawHUD(void){
    if(gEnemyTurnSummaryVisible){
        Gameplay_DrawEnemyTurnSummary();
        return;
    }

    static uint8_t prevTurnMode = 255;
    static uint8_t prevAttackMove = 255;
    static uint8_t prevEnergy = 255;
    static uint8_t prevPreview = 255;
    static uint8_t prevEnemyPresent = 255;
    static uint8_t prevEnemyHP = 255;
    static uint8_t prevEnemyATK = 255;
    static uint32_t prevADC = 0xFFFFFFFFu;
    static uint32_t prevADCCm = 0xFFFFFFFFu;
    uint8_t previewEnergy;
    uint8_t modeEnergy;
    Entity* hoveredEnemy = Gameplay_FindAttackTargetAtCursor();
    uint8_t enemyPresent = (uint8_t)(gTurnMode == TURNMODE_ATTACK && hoveredEnemy != 0);
    uint8_t enemyHP = enemyPresent ? (uint8_t)hoveredEnemy->data0 : 255;
    uint8_t enemyATK = enemyPresent ? (uint8_t)hoveredEnemy->data1 : 255;
    Gameplay_UpdateEnergySplitFromADC();
    modeEnergy = Gameplay_CurrentModeEnergyRemaining();
    previewEnergy = (gTurnMode == TURNMODE_MOVE) ? Gameplay_PreviewEnergyAfterMove() : Gameplay_PreviewEnergyAfterAttack();

    if(!gForceHUDRedraw &&
       prevTurnMode == gTurnMode &&
       prevAttackMove == gSelectedAttackMove &&
       prevEnergy == modeEnergy &&
       prevPreview == previewEnergy &&
       prevEnemyPresent == enemyPresent &&
       prevEnemyHP == enemyHP &&
       prevEnemyATK == enemyATK &&
       prevADC == gLatestADC &&
       prevADCCm == gLatestADCCm){
        return;
    }

    Gameplay_ClearHUDRow(12);
    Gameplay_ClearHUDRow(13);
    Gameplay_ClearHUDRow(14);
    Gameplay_ClearHUDRow(15);
    Gameplay_ClearHUDRow(16);
    Gameplay_ClearHUDRow(17);
    Gameplay_ClearHUDRow(18);
    Gameplay_ClearHUDRow(19);
    Gameplay_ClearHUDRow(20);
    ST7735_SetCursor(0, 13);
    if(gTurnMode == TURNMODE_MOVE){
        ST7735_OutString("Move mode");
    } else {
        ST7735_OutString("Atk:");
        ST7735_OutString((gSelectedAttackMove == PLAYERSTYLE_MELEE) ? "Melee " : "Range ");
    }
    if(gTurnMode == TURNMODE_ATTACK && hoveredEnemy){
        ST7735_SetCursor(0, 12);
        ST7735_OutString("Enemy HP:");
        ST7735_OutUDec(hoveredEnemy->data0);
        ST7735_OutString(" ATK:");
        ST7735_OutUDec(hoveredEnemy->data1);
    }
    ST7735_SetCursor(0, 14);
    ST7735_OutString("E:");
    ST7735_OutUDec(modeEnergy);
    ST7735_OutString("->");
    previewEnergy = (gTurnMode == TURNMODE_MOVE) ? Gameplay_PreviewEnergyAfterMove() : Gameplay_PreviewEnergyAfterAttack();
    if(previewEnergy == 255){
        ST7735_OutString("TooExp");
    } else {
        ST7735_OutUDec(previewEnergy);
        ST7735_OutString("     ");
    }
    prevTurnMode = gTurnMode;
    prevAttackMove = gSelectedAttackMove;
    prevEnergy = modeEnergy;
    prevPreview = previewEnergy;
    prevEnemyPresent = enemyPresent;
    prevEnemyHP = enemyHP;
    prevEnemyATK = enemyATK;
    prevADC = gLatestADC;
    prevADCCm = gLatestADCCm;
    ST7735_SetCursor(0, 16);
    ST7735_OutString("Lv:");
    ST7735_OutUDec(gCurrentStage);
    ST7735_OutString("/3");
    ST7735_SetCursor(0, 17);
    ST7735_OutString("Move:");
    ST7735_OutUDec(gMoveEnergyRemaining);
    ST7735_OutString(" Atk:");
    ST7735_OutUDec(gAttackEnergyRemaining);
    ST7735_SetCursor(0, 18);
    ST7735_OutString("ADC:");
    ST7735_OutUDec(gLatestADC);
    ST7735_OutString(" (");
    ST7735_OutUDec(gLatestADCCm);
    ST7735_OutString("cm)");
    gForceHUDRedraw = 0;
}

static void Gameplay_DrawRangeHighlights(void){
    uint8_t x;
    uint8_t y;
    for(y = 0; y < 8; y++){
        for(x = 0; x < 8; x++){
            if(gTurnMode == TURNMODE_MOVE){
                uint8_t moveCost = 255;
                if(Gameplay_IsTileReachableByPlayer(x, y, gMoveEnergyRemaining, &moveCost) &&
                   !(x == player->tileX && y == player->tileY)){
                    //ST7735_DrawBitmap((x * 12) + 3, (y * 12) + 9, moveIcon, 6, 8);
                    ST7735_FillRect((x * 12 + 6 ), (y*12 + 5), 2, 2, ST7735_BLUE);
                }
            } else if(gTurnMode == TURNMODE_ATTACK){
                uint8_t range = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? 1 : 4;
                uint8_t distance = (uint8_t)(abs((int16_t)x - player->tileX) + abs((int16_t)y - player->tileY));
                if(distance <= range){
                    //const uint16_t* iconToDraw = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? meleeIcon : rangedIcon;
                    //ST7735_DrawBitmap((x * 12) + 3, (y * 12) + 9, iconToDraw, 6, 8);
                    ST7735_FillRect((x * 12 + 6 ), (y*12 + 5), 2, 2, ST7735_RED);
                }
            }
        }
    }
}

static void Gameplay_DrawHighlightAtTile(uint8_t tileX, uint8_t tileY){
    if(gTurnMode == TURNMODE_MOVE){
        uint8_t moveCost = 255;
        if(Gameplay_IsTileReachableByPlayer(tileX, tileY, gMoveEnergyRemaining, &moveCost) &&
           !(tileX == player->tileX && tileY == player->tileY)){
            //ST7735_DrawBitmap((tileX * 12) + 3, (tileY * 12) + 9, moveIcon, 6, 8);
            ST7735_FillRect((tileX * 12 + 6 ), (tileY * 12 + 5), 2, 2, ST7735_BLUE);
        }
    } else if(gTurnMode == TURNMODE_ATTACK){
        uint8_t range = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? 1 : 4;
        uint8_t distance = (uint8_t)(abs((int16_t)tileX - player->tileX) + abs((int16_t)tileY - player->tileY));
        if(distance <= range){
            const uint16_t* iconToDraw = (gSelectedAttackMove == PLAYERSTYLE_MELEE) ? meleeIcon : rangedIcon;
                    //ST7735_DrawBitmap((tileX * 12) + 3, (tileY * 12) + 9, iconToDraw, 6, 8);
                    ST7735_FillRect((tileX * 12 + 6 ), (tileY * 12 + 5), 2, 2, ST7735_RED);
        }
    }
}

static void Gameplay_DrawEntityAtTile(uint8_t tileX, uint8_t tileY){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        int16_t drawX;
        int16_t tileTopY;
        int16_t spriteTopY;
        int16_t drawY;
        Entity* e = &entList[i];
        if(!e->active) continue;
        if(e->tileX != tileX || e->tileY != tileY) continue;
        drawX = (int16_t)tileX * 12 + ((int16_t)12 - (int16_t)e->width) / 2;
        tileTopY = (int16_t)tileY * 12;
        spriteTopY = tileTopY + ((int16_t)12 - (int16_t)e->height) / 2;
        drawY = spriteTopY + e->height - 1;
        ST7735_DrawBitmap(drawX, drawY, e->bitmap, e->width, e->height);
    }
}

static void Gameplay_RedrawTile(uint8_t tileX, uint8_t tileY, uint8_t includeCursor){
    drawRoomTile(worldMap, worldX, worldY, tileX, tileY);
    Gameplay_DrawHighlightAtTile(tileX, tileY);
    Gameplay_DrawEntityAtTile(tileX, tileY);
    if(gTurnMode == TURNMODE_ATTACK && Gameplay_CanAttackTarget() &&
       tileX == gCursorX && tileY == gCursorY){
        ST7735_DrawBitmap(tileX * 12, (tileY + 1) * 12 - 1, redtile, 12, 12);
        Gameplay_DrawEntityAtTile(tileX, tileY);
    }
    if(includeCursor && tileX == gCursorX && tileY == gCursorY){
        ST7735_DrawBitmap((tileX * 12) + 2, (tileY * 12) + 9, yellowBlock, 8, 8);
    }
}


static void Gameplay_DrawCursor(void){
    uint8_t cursorDrawX;
    uint8_t cursorDrawY;
    cursorDrawX = (uint8_t)(gCursorX * 12 + 4);
    cursorDrawY = (uint8_t)(gCursorY * 12 + 7);
    ST7735_DrawBitmap(cursorDrawX, cursorDrawY, cursor, 4, 4);
    
    gOldCursorX = gCursorX;
    gOldCursorY = gCursorY;
}

static void Gameplay_DrawTargetMarker(void){
    if(gTurnMode == TURNMODE_ATTACK && Gameplay_CanAttackTarget()){
        ST7735_DrawBitmap(gCursorX * 12, (gCursorY + 1) * 12 - 1, redtile, 12, 12);
    }
}

static void Gameplay_DrawHealthBar(void){
    static uint8_t prevHealth = 255;
    static uint8_t prevHealthMax = 255;
    uint8_t i;
    uint8_t barX = 108;
    uint8_t barStartY = 92;
    if(!gForceHealthRedraw && prevHealth == gPlayerHealth && prevHealthMax == gPlayerHealthMax){
        return;
    }
    ST7735_SetCursor(18, 0);
    ST7735_OutString("HP");
    for(i = 0; i < gPlayerHealthMax; i++){
        uint8_t y = (uint8_t)(barStartY - (i * 8));
        if(i < gPlayerHealth){
            ST7735_DrawBitmap(barX, y, redtile, 12, 12);
        } else {
            ST7735_DrawBitmap(barX, y, bluetile, 12, 12);
        }
    }
    prevHealth = gPlayerHealth;
    prevHealthMax = gPlayerHealthMax;
    gForceHealthRedraw = 0;
}

static void Gameplay_EndPlayerTurn(void){
    Gameplay_RunEnemyTurn();
    gEnemyTurnSummaryVisible = 1;
    gEnemyTurnSummaryDirty = 1;
    gAwaitingEnemyTurnAck = 1;
    gForceHUDRedraw = 1;
    gForceHealthRedraw = 1;
    gNeedsFullGameplayRedraw = 1;
}

static void Gameplay_RunEnemyTurn(void){
    uint8_t i;
    gEnemyTurnHPBefore = gPlayerHealth;
    gEnemyTurnEnemiesMoved = 0;
    gEnemyTurnEnemiesAttacked = 0;
    for(i = 0; i < MAXENTITIES; i++){
        Entity* enemy = &entList[i];
        if(!enemy->active || enemy->type != ENEMY){
            continue;
        }
        if(Gameplay_IsPlayerAdjacentToEnemy(enemy)){
            uint8_t damage = (uint8_t)enemy->data1;
            if(damage == 0){
                damage = 1;
            }
            if(damage >= gPlayerHealth){
                gPlayerHealth = 0;
            } else {
                gPlayerHealth = (uint8_t)(gPlayerHealth - damage);
            }
            gEnemyTurnEnemiesAttacked++;
            continue;
        }
        {
            uint8_t oldX = enemy->tileX;
            uint8_t oldY = enemy->tileY;
        Gameplay_EnemyTryMoveTowardsPlayer(enemy);
            if(oldX != enemy->tileX || oldY != enemy->tileY){
                gEnemyTurnEnemiesMoved++;
            }
        }
    }
    gEnemyTurnHPAfter = gPlayerHealth;
    gEnemyTurnDamageTaken = (uint8_t)(gEnemyTurnHPBefore - gEnemyTurnHPAfter);
}

static void Gameplay_DrawEnemyTurnSummary(void){
    if(!gEnemyTurnSummaryDirty){
        return;
    }
    Gameplay_ClearHUDRow(12);
    Gameplay_ClearHUDRow(13);
    Gameplay_ClearHUDRow(14);
    Gameplay_ClearHUDRow(15);
    Gameplay_ClearHUDRow(16);
    Gameplay_ClearHUDRow(17);
    Gameplay_ClearHUDRow(18);
    Gameplay_ClearHUDRow(19);
    Gameplay_ClearHUDRow(20);
    ST7735_SetCursor(0, 12);
    ST7735_OutString("Enemy phase");
    ST7735_SetCursor(0, 13);
    ST7735_OutString("HP:");
    ST7735_OutUDec(gEnemyTurnHPBefore);
    ST7735_OutString("->");
    ST7735_OutUDec(gEnemyTurnHPAfter);
    ST7735_SetCursor(0, 14);
    ST7735_OutString("Hits:");
    ST7735_OutUDec(gEnemyTurnEnemiesAttacked);
    ST7735_OutString(" Move:");
    ST7735_OutUDec(gEnemyTurnEnemiesMoved);
    ST7735_SetCursor(0, 15);
    ST7735_OutString("Dmg:");
    ST7735_OutUDec(gEnemyTurnDamageTaken);
    ST7735_OutString(" A=Next");
    gEnemyTurnSummaryDirty = 0;
}

static void Gameplay_AcknowledgeEnemyTurnSummary(void){
    Gameplay_ClearHUDRow(12);
    Gameplay_ClearHUDRow(13);
    Gameplay_ClearHUDRow(14);
    Gameplay_ClearHUDRow(15);
    Gameplay_ClearHUDRow(16);
    Gameplay_ClearHUDRow(17);
    Gameplay_ClearHUDRow(18);
    Gameplay_ClearHUDRow(19);
    Gameplay_ClearHUDRow(20);
    gEnemyTurnSummaryVisible = 0;
    gEnemyTurnSummaryDirty = 1;
    gAwaitingEnemyTurnAck = 0;
    gEnergyRemaining = gMovementEnergyMax;
    gMoveEnergyRemaining = gMovementEnergyMax;
    gAttackEnergyRemaining = 0;
    gEnergySplitLocked = 0;
    gMoveCommittedThisTurn = 0;
    Gameplay_UpdateEnergySplitFromADC();
    gTurnMode = TURNMODE_MOVE;
    Gameplay_ResetCursorToPlayer();
    Gameplay_UpdateTeleporterState();
    gForceHUDRedraw = 1;
    gForceHealthRedraw = 1;
    gNeedsFullGameplayRedraw = 1;
}

static void Gameplay_ClearHUDRow(uint8_t row){
    ST7735_FillRect(0, (uint16_t)(row * 8), 128, 8, ST7735_BLACK);
}

static void Gameplay_LoadStage(uint8_t stage){
    uint8_t stageBucket = (uint8_t)((stage - 1) % 3);
    Gameplay_ClearEnemies();

    if(stageBucket == 0){
        Gameplay_SpawnEnemyAt(4, 2, 8, 1, enemy1);      // scout
        Gameplay_SpawnEnemyAt(2, 5, 10, 2, yellowBlock); // skirmisher
    } else if(stageBucket == 1){
        Gameplay_SpawnEnemyAt(5, 2, 10, 2, yellowBlock); // skirmisher
        Gameplay_SpawnEnemyAt(1, 1, 7, 1, enemy1);       // scout
        Gameplay_SpawnEnemyAt(4, 6, 12, 3, happyBlue);   // bruiser
    } else {
        Gameplay_SpawnEnemyAt(1, 2, 8, 1, enemy1);       // scout
        Gameplay_SpawnEnemyAt(6, 2, 9, 2, yellowBlock);  // skirmisher
        Gameplay_SpawnEnemyAt(2, 6, 12, 3, happyBlue);   // bruiser
        Gameplay_SpawnEnemyAt(5, 6, 14, 4, blueBlock);   // elite bruiser
    }
}

static void Gameplay_ClearEnemies(void){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        if(entList[i].active && entList[i].type == ENEMY){
            Entity_Deactivate(&entList[i]);
        }
    }
}

static void Gameplay_SpawnEnemyAt(uint8_t tileX, uint8_t tileY, uint8_t hp, uint8_t attackPower, const uint16_t* sprite){
    Entity* enemy = addEntity(entList);
    if(!enemy){
        return;
    }
    Entity_Init(enemy, tileX, tileY, 8, 8, ENEMY, sprite ? sprite : enemy1);
    enemy->data0 = hp;
    enemy->data1 = attackPower;
    Entity_Activate(enemy);
}

static void Gameplay_UpdateTeleporterState(void){
    uint8_t i;
    uint8_t unlocked = (uint8_t)!Gameplay_HasAliveEnemies();
    for(i = 0; i < MAXENTITIES; i++){
        if(!entList[i].active || entList[i].type != TELEPORTER){
            continue;
        }
        entList[i].data0 = unlocked;
        Entity_SetBitmap(&entList[i], unlocked ? teleporter_Active : teleporter_Inactive);
    }
}

static uint8_t Gameplay_IsPlayerAdjacentToEnemy(const Entity* enemy){
    int16_t dx;
    int16_t dy;
    if(!enemy){
        return 0;
    }
    dx = (int16_t)enemy->tileX - player->tileX;
    dy = (int16_t)enemy->tileY - player->tileY;
    if(dx < 0) dx = -dx;
    if(dy < 0) dy = -dy;
    return (uint8_t)((dx + dy) == 1);
}

static void Gameplay_EnemyTryMoveTowardsPlayer(Entity* enemy){
    int8_t stepX = 0;
    int8_t stepY = 0;
    uint8_t nextX;
    uint8_t nextY;
    if(!enemy) return;

    if(enemy->tileX < player->tileX){
        stepX = 1;
    } else if(enemy->tileX > player->tileX){
        stepX = -1;
    }

    if(stepX != 0){
        nextX = (uint8_t)((int8_t)enemy->tileX + stepX);
        nextY = enemy->tileY;
        if(!Gameplay_IsTileOccupiedByEnemy(nextX, nextY, enemy) &&
           !(nextX == player->tileX && nextY == player->tileY) &&
           Gameplay_IsTileTraversableForMove(nextX, nextY)){
            enemy->tileX = nextX;
            return;
        }
    }

    if(enemy->tileY < player->tileY){
        stepY = 1;
    } else if(enemy->tileY > player->tileY){
        stepY = -1;
    }

    if(stepY != 0){
        nextX = enemy->tileX;
        nextY = (uint8_t)((int8_t)enemy->tileY + stepY);
        if(!Gameplay_IsTileOccupiedByEnemy(nextX, nextY, enemy) &&
           !(nextX == player->tileX && nextY == player->tileY) &&
           Gameplay_IsTileTraversableForMove(nextX, nextY)){
            enemy->tileY = nextY;
        }
    }
}

static uint8_t Gameplay_IsTileOccupiedByEnemy(uint8_t tileX, uint8_t tileY, const Entity* ignoreEnemy){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        if(!entList[i].active || entList[i].type != ENEMY){
            continue;
        }
        if(&entList[i] == ignoreEnemy){
            continue;
        }
        if(entList[i].tileX == tileX && entList[i].tileY == tileY){
            return 1;
        }
    }
    return 0;
}

static uint8_t Gameplay_HasAliveEnemies(void){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        if(entList[i].active && entList[i].type == ENEMY){
            return 1;
        }
    }
    return 0;
}

static void Gameplay_Shutdown(void){
    uint8_t i;
    for(i = 0; i < MAXENTITIES; i++){
        Entity_Deactivate(&entList[i]);
    }
    gGameplayLoaded = 0;
    gNeedsFullGameplayRedraw = 1;
    gForceHUDRedraw = 1;
    gForceHealthRedraw = 1;
    gCurrentStage = 1;
    gStageLoadedWorldX = 255;
    gStageLoadedWorldY = 255;
    oldWorldX = 255;
    oldWorldY = 255;
}

static void GameState_DrawEnding(void){
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_SetCursor(3, 4);
    ST7735_OutString("You escaped!");
    ST7735_SetCursor(1, 7);
    ST7735_OutString("All 3 levels clear");
    ST7735_SetCursor(1, 10);
    ST7735_OutString("Press A for menu");
}
