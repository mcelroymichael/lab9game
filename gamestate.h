#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdint.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define A_BUTTON 4
#define B_BUTTON 5

typedef enum {
    GAMESTATE_LANGUAGE_SELECT = 0,
    GAMESTATE_MAIN_MENU,
    GAMESTATE_LEVEL1
} GameState;

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
    GAMEBUTTON_B
} GameButton;

void GameState_Init(void);
GameState GameState_Get(void);
Language GameState_GetLanguage(void);
void GameState_Set(GameState newState);
void GameState_Update(void);
void GameState_Draw(void);
void GameState_OnButtonPressed(GameButton button);
void GameState_OnButtonReleased(GameButton button);

#endif