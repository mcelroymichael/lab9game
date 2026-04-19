// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Alp Shamsapour & Michael McElroy
// Last Modified: April 13, 2026

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "entity.h"
#include "graphics.h"
#include "room.h"
#include "gamestate.h"
//#include <SPI.h>
//#include <SD.h>

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

Entity entList[MAXENTITIES];

uint8_t worldX, worldY, oldWorldX, oldWorldY;
uint32_t buttonState, oldButtonState; 
volatile uint32_t potState;
uint32_t oldPotState; 

bool DRAWREADY;

Room* worldMap[MAXWORLD_SIZE][MAXWORLD_SIZE];
Entity* player;

void handleButtons(void){
  uint32_t changed = buttonState ^ oldButtonState;
  if(!changed) return;

  if(changed & (1u << 0)){ // A
    if(buttonState & (1u << 0)) GameState_OnButtonPressed(GAMEBUTTON_A);
    else GameState_OnButtonReleased(GAMEBUTTON_A);
  }
  if(changed & (1u << 1)){ // RIGHT
    if(buttonState & (1u << 1)) GameState_OnButtonPressed(GAMEBUTTON_RIGHT);
    else GameState_OnButtonReleased(GAMEBUTTON_RIGHT);
  }
  if(changed & (1u << 2)){ // DOWN
    if(buttonState & (1u << 2)) GameState_OnButtonPressed(GAMEBUTTON_DOWN);
    else GameState_OnButtonReleased(GAMEBUTTON_DOWN);
  }
  if(changed & (1u << 3)){ // LEFT
    if(buttonState & (1u << 3)) GameState_OnButtonPressed(GAMEBUTTON_LEFT);
    else GameState_OnButtonReleased(GAMEBUTTON_LEFT);
  }
  if(changed & (1u << 4)){ // UP
    if(buttonState & (1u << 4)) GameState_OnButtonPressed(GAMEBUTTON_UP);
    else GameState_OnButtonReleased(GAMEBUTTON_UP);
  }
  if(changed & (1u << 5)){ // B
    if(buttonState & (1u << 5)) GameState_OnButtonPressed(GAMEBUTTON_B);
    else GameState_OnButtonReleased(GAMEBUTTON_B);
  }
  if(changed & (1u << 6)){ // ALT
    if(buttonState & (1u << 6)) GameState_OnButtonPressed(GAMEBUTTON_ALT);
    else GameState_OnButtonReleased(GAMEBUTTON_ALT);
  }
  if(changed & (1u << 7)){ // ESC
    if(buttonState & (1u << 7)) GameState_OnButtonPressed(GAMEBUTTON_ESC);
    else GameState_OnButtonReleased(GAMEBUTTON_ESC);
  }

  oldButtonState = buttonState;
  
}

// games  engine runs at 30Hz 
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
  if(DRAWREADY) return; //Don't run interrupt if the frame hasnt been drawn yet.
    
    potState = ADCin();         // 1) sample slide pot
    
    buttonState = Switch_In();  // 2) read input switches
    handleButtons();
    
    updateEntities(entList);    // 3) move sprites
    
  //manageSounds();             // 4) start sounds
    
    DRAWREADY = true;           // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES

    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}

uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}


int main(void){ // main testing
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init();
  DAC_Init();
  ST7735_InitPrintf(INITR_BLACKTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  
  GameState_Init();
  buttonState = oldButtonState = 0;

  DRAWREADY = false;              //initialize draw flag
  TimerG12_IntArm(80000000/30,2);
  __enable_irq();

  while(1){
    
    if(!DRAWREADY) continue; //pass until draw flag is set
    
    GameState_Draw();
    
    DRAWREADY = false;
  }
}


// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}
