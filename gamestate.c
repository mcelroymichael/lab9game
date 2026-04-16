#include "gamestate.h"
#include "entity.h"
#include "room.h"
#include "../inc/ST7735.h"
#include "graphics.h"
#include <stdio.h>
#include <stdbool.h>

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
                    Changed = 1;
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
        Room testRoom1, testRoom2, testRoom3, NULLROOM;
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
        

////WE NEED TO IMPLEMENT THESE FUNCTIONS. THEY ARE PLACEHOLDERS
        //World_InitINGAME(world);
        //INGAME_SpawnEntities(entities, world);
        //INGAME_ResetPlayerPosition();
        gGameplayLoaded = 1;
    }

    gGameState = GAMESTATE_INGAME;
}

// ------------------------------------------------------------
// Gameplay handlers
// ------------------------------------------------------------
static void GameState_HandlePressedGameplay(GameButton button){
    switch(button){
        case GAMEBUTTON_B:
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
    if(oldWorldX != worldX || oldWorldY != worldY){
      drawRoom(worldMap, worldX, worldY);
      oldWorldX = worldX;
      oldWorldY = worldY;
    }
    drawEntities(entList, worldMap, worldX, worldY);
}
