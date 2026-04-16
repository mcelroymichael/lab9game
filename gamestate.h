#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    GAMESTATE_LANGUAGE_SELECT = 0,
    GAMESTATE_MAIN_MENU,
    GAMESTATE_PAUSE_MENU,
    GAMESTATE_INGAME
} GameState;

typedef enum {
    LEVELSTATE_1 = 1,
    LEVELSTATE_2,
    LEVELSTATE_3,
    LEVELSTATE_B
} LevelState;

typedef enum {
    LANGUAGE_ENGLISH = 0,
    LANGUAGE_SPANISH
} Language;

typedef enum {
    GAMEBUTTON_UP = 0,
    GAMEBUTTON_DOWN,
    GAMEBUTTON_LEFT,
    GAMEBUTTON_RIGHT,
    GAMEBUTTON_A,
    GAMEBUTTON_B,
    GAMEBUTTON_ESC,
    GAMEBUTTON_ALT
} GameButton;

void GameState_Init(void);
bool inGame(void);
GameState GameState_Get(void);
Language GameState_GetLanguage(void);
void GameState_Set(GameState newState);
void GameState_Update(void);
void GameState_Draw(void);
void GameState_OnButtonPressed(GameButton button);
void GameState_OnButtonReleased(GameButton button);

#endif