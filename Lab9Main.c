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

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
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
uint32_t potState, oldPotState; 

bool DRAWREADY;

Room* worldMap[MAXWORLD_SIZE][MAXWORLD_SIZE];
Entity* player;

void handleButtons(void){
  if(buttonState == oldButtonState) return;
  else{
    uint8_t status;
    bool inMenu = false;

    if(GameState_Get() == GAMESTATE_MAIN_MENU || GameState_Get() == GAMESTATE_LANGUAGE_SELECT) {
      inMenu = true;
    }

    // DEBOUNCING (WAIT FUNCTION CAN GO HERE)
    if(((buttonState >> 0) & 0b1) != oldButtonState){                  // A_BUTTON
        if(((buttonState >> 0) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_A);
          } else{

          }
        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_A);
          } else{

          }
        }
    }
    if(((buttonState >> 1) & 0b1) != (oldButtonState >> 1)){                  // RIGHT
        if(((buttonState >> 1) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_RIGHT);
          } else{
            status = Entity_TryMove(player, 1, 0, getTileMap(worldMap, worldX, worldY));
            if(status == 2) {
              if((worldX + 1) < MAXWORLD_SIZE){
                worldX = worldX + 1;
                Entity_SetTilePosition(player, 0, player->tileY);
              }
            }
          }
        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_RIGHT);
          } else{

          }
        }
    }
    if(((buttonState >> 2) & 0b1) != (oldButtonState >> 2)){    // DOWN
        if(((buttonState >> 2) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_DOWN);
          } else{
            status = Entity_TryMove(player, 0, 1, getTileMap(worldMap, worldX, worldY));
            if(status == 2) {
              if((worldY + 1) < MAXWORLD_SIZE){
                worldY = worldY + 1;
                Entity_SetTilePosition(player, player->tileX, 0);
              }
            }
          }

        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_DOWN);
          } else{

          }
      }
    }
    if(((buttonState >> 3) & 0b1) != (oldButtonState >> 3)){    // LEFT
        if(((buttonState >> 3) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_LEFT);
          } else{
            status = Entity_TryMove(player, -1, 0, getTileMap(worldMap, worldX, worldY));
            if(status == 2) {
              if((worldX - 1) >= 0){
                worldX = worldX - 1;
                Entity_SetTilePosition(player, 7, player->tileY);
              }
            }
          }
        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_LEFT);
          } else{

          }
      }
    }
    if(((buttonState >> 4) & 0b1) != (oldButtonState >> 4)){    // UP
        if(((buttonState >> 4) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_UP);
          } else{
            status = Entity_TryMove(player, 0, -1, getTileMap(worldMap, worldX, worldY));
            if(status == 2) {
              if((worldY - 1) >= 0){
                worldY = worldY - 1;
                Entity_SetTilePosition(player, player->tileX, 7);
              }
            }
          }
        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_UP);
          } else{

          }
      }
    }
      if(((buttonState >> 5) & 0b1) != (oldButtonState >> 5)){    //B
        if(((buttonState >> 5) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_UP);
          } else{
            status = Entity_TryMove(player, 0, -1, getTileMap(worldMap, worldX, worldY));
            if(status == 2) {
              if((worldY - 1) >= 0){
                worldY = worldY - 1;
                Entity_SetTilePosition(player, player->tileX, 7);
              }
            }
          }
        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_UP);
          } else{

          }
      }
    }
      if(((buttonState >> 6) & 0b1) != (oldButtonState >> 6)){    // ALT
        if(((buttonState >> 6) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            GameState_OnButtonPressed(GAMEBUTTON_UP);
          } else{
            status = Entity_TryMove(player, 0, -1, getTileMap(worldMap, worldX, worldY));
            if(status == 2) {
              if((worldY - 1) >= 0){
                worldY = worldY - 1;
                Entity_SetTilePosition(player, player->tileX, 7);
              }
            }
          }
        } else {                      //FALLING EDGE
          if(inMenu){
            GameState_OnButtonReleased(GAMEBUTTON_UP);
          } else{

          }
      }
    }
      if(((buttonState >> 7) & 0b1) != (oldButtonState >> 7)){    // ESC
        if(((buttonState >> 7) & 0b1) == 1){ //RISING EDGE DETECTION
          if(inMenu){
            
          } else{
            
          }
        } else {                      //FALLING EDGE
          
          
          }
      }
  
  oldButtonState = buttonState;
  }
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


// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs
    
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

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main5(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}
