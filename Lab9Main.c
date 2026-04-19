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

#define BUTTON_A_MASK      0x1U
#define BUTTON_RIGHT_MASK  0x1U
#define BUTTON_DOWN_MASK   0x2U
#define BUTTON_LEFT_MASK   0x4U
#define BUTTON_UP_MASK     0x8U
#define BUTTON_ESC_MASK    0xFU  // Press all 4 direction buttons as ESC

typedef enum {
  MODE_MAIN_MENU = 0,
  MODE_TUTORIAL,
  MODE_PLAYING,
  MODE_CONFIRM_RETURN
} GameMode_t;

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
volatile GameMode_t gMode = MODE_MAIN_MENU;
volatile uint8_t menuSelect = 0;
volatile uint8_t confirmYesSelected = 0;
volatile uint8_t requestStartGame = 0;
volatile uint8_t requestContinueFromTutorial = 0;
volatile uint8_t requestOpenEscapePrompt = 0;
volatile uint8_t requestConfirmReturn = 0;
volatile uint8_t requestCancelReturn = 0;

Room* worldMap[MAXWORLD_SIZE][MAXWORLD_SIZE];
Entity* player;

static void DrawTopBanner(void){
  ST7735_FillRect(0, 0, 128, 12, ST7735_BLUE);
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_SetCursor(4, 0);
  ST7735_OutString("PLAYER TURN");
}

static void DrawMainMenu(void){
  ST7735_FillScreen(ST7735_BLACK);
  DrawTopBanner();
  ST7735_SetTextColor(ST7735_YELLOW);
  ST7735_SetCursor(1, 2); // keep menu near/flush to banner
  ST7735_OutString("MAIN MENU");
  ST7735_SetCursor(1, 4);
  if(menuSelect == 0){
    ST7735_OutString("> (Start Game)");
  }else{
    ST7735_OutString("  (Start Game)");
  }
  ST7735_SetTextColor(ST7735_WHITE);
}

static void DrawTutorial(void){
  ST7735_FillScreen(ST7735_BLACK);
  DrawTopBanner();
  ST7735_SetTextColor(ST7735_GREEN);
  ST7735_SetCursor(1, 3);
  ST7735_OutString("TUTORIAL");
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_SetCursor(0, 5);
  ST7735_OutString("Use D-pad to move.");
  ST7735_SetCursor(0, 7);
  ST7735_OutString("Avoid blocked tiles.");
  ST7735_SetCursor(0, 9);
  ST7735_OutString("Press all 4 buttons");
  ST7735_SetCursor(0,10);
  ST7735_OutString("to open ESC menu.");
  ST7735_SetTextColor(ST7735_YELLOW);
  ST7735_SetCursor(0, 13);
  ST7735_OutString("(press A to continue)");
  ST7735_SetTextColor(ST7735_WHITE);
}

static void DrawConfirmReturnDialog(void){
  uint16_t boxX = 8;
  uint16_t boxY = 32;
  uint16_t boxW = 112;
  uint16_t boxH = 66;
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_FillRect(boxX, boxY, boxW, boxH, ST7735_WHITE);
  ST7735_FillRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, ST7735_BLACK);
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_SetCursor(2, 5);
  ST7735_OutString("Return to Main Menu?");
  ST7735_SetCursor(2, 7);
  ST7735_OutString("Unsaved progress");
  ST7735_SetCursor(2, 8);
  ST7735_OutString("will be lost.");
  ST7735_SetCursor(3, 10);
  if(confirmYesSelected){
    ST7735_OutString("> Y      N");
  }else{
    ST7735_OutString("  Y    > N");
  }
  ST7735_SetCursor(0, 13);
  ST7735_OutString("L/R select, A confirm");
}

static void ResetGameProgress(void){
  worldX = 0;
  worldY = 0;
  oldWorldX = 255;
  oldWorldY = 255;
  if(player){
    player->x = 0;
    player->y = 0;
  }
}

void handleButtons(void){
  if(buttonState == oldButtonState) return;
  else{
    uint32_t changed = buttonState ^ oldButtonState;
    uint8_t aPressed = ((changed & BUTTON_A_MASK) != 0) && ((buttonState & BUTTON_A_MASK) != 0);
    if(gMode == MODE_MAIN_MENU){
      if(aPressed){
        requestStartGame = 1;
      }
      oldButtonState = buttonState;
      return;
    }
    if(gMode == MODE_TUTORIAL){
      if(aPressed){
        requestContinueFromTutorial = 1;
      }
      oldButtonState = buttonState;
      return;
    }
    if(gMode == MODE_CONFIRM_RETURN){
      if(((changed & BUTTON_LEFT_MASK) != 0) && ((buttonState & BUTTON_LEFT_MASK) != 0)){
        confirmYesSelected = 1;
      }
      if(((changed & BUTTON_RIGHT_MASK) != 0) && ((buttonState & BUTTON_RIGHT_MASK) != 0)){
        confirmYesSelected = 0;
      }
      if(aPressed){
        if(confirmYesSelected){
          requestConfirmReturn = 1;
        }else{
          requestCancelReturn = 1;
        }
      }
      oldButtonState = buttonState;
      return;
    }
    // DEBOUNCING (WAIT FUNCTION CAN GO HERE)
    if((buttonState & BUTTON_ESC_MASK) == BUTTON_ESC_MASK && (oldButtonState & BUTTON_ESC_MASK) != BUTTON_ESC_MASK){
      requestOpenEscapePrompt = 1;
      oldButtonState = buttonState;
      return;
    }
    if((buttonState & 0b1) != (oldButtonState & 0b1)){                  // RIGHT
        if((buttonState & 0b1) == 1){ //RISING EDGE DETECTION
          //worldX = (worldX+1) %2;
          Entity_TryMove(player, 1, 0, getTileMap(worldMap, worldX, worldY));
        } else {                      //FALLING EDGE

        }
    }
    if(((buttonState >> 1) & 0b1) != (oldButtonState >> 1)){    // DOWN
        if(((buttonState >> 1) & 0b1) == 1){ //RISING EDGE DETECTION
          //worldY = (worldY+1) %2;
          Entity_TryMove(player, 0, 1, getTileMap(worldMap, worldX, worldY));

        } else {                      //FALLING EDGE

      }
    }
    if(((buttonState >> 2) & 0b1) != (oldButtonState >> 2)){    // LEFT
        if(((buttonState >> 2) & 0b1) == 1){ //RISING EDGE DETECTION
          //worldY = (worldY+1) %2;
          Entity_TryMove(player, -1, 0, getTileMap(worldMap, worldX, worldY));

        } else {                      //FALLING EDGE

      }
    }
    if(((buttonState >> 3) & 0b1) != (oldButtonState >> 3)){    // UP
        if(((buttonState >> 3) & 0b1) == 1){ //RISING EDGE DETECTION
          //worldY = (worldY+1) %2;
          Entity_TryMove(player, 0, -1, getTileMap(worldMap, worldX, worldY));

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
    // 1) sample slide pot
    potState = ADCin();
    // 2) read input switches
    buttonState = Switch_In();
    handleButtons();
    // 3) move sprites
    updateEntities(entList);
    // 4) start sounds
    // 5) set semaphore
    DRAWREADY = true;
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
  entityArrInit(entList);

  Entity *block = addEntity(entList);
  player = block;
  Entity_Init(block, 0, 0, 8, 8, PLAYER, happyBlock);
  Entity_Activate(block);

  //Entity *block2 = addEntity(entList);
  //Entity_Init(block2, 80, 30, 8, 8, ENEMY, happyBlock, testMap);
  //Entity_Activate(block2);

  uint32_t tilemap1[64] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    1,1,1,0,0,1,1,1,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
  };
  uint32_t tilemap2[64] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    1,0,0,0,1,1,1,1,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
  };
  uint32_t tilemap3[64] = {
    0,0,0,1,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,1,1,0,0,0,0,
    0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,1,1,0,0,0,0,
    0,1,1,0,0,0,0,0,
    0,1,1,0,0,0,0,0
  };

  buttonState = oldButtonState = 0;
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


  DRAWREADY = false;              //initialize draw flag
  TimerG12_IntArm(80000000/30,2);
  __enable_irq();
  DrawMainMenu();

  while(1){
    //Entity_Update(block);
    //Entity_PrintSelf(block);
    //Entity_Update(Entity *e);
    if(!DRAWREADY) continue; //pass until draw flag is set
    if(requestStartGame){
      requestStartGame = 0;
      gMode = MODE_TUTORIAL;
      DrawTutorial();
    }
    if(requestContinueFromTutorial){
      requestContinueFromTutorial = 0;
      ResetGameProgress();
      gMode = MODE_PLAYING;
      ST7735_FillScreen(ST7735_BLACK);
      oldWorldX = 255;
      oldWorldY = 255;
    }
    if(requestOpenEscapePrompt){
      requestOpenEscapePrompt = 0;
      gMode = MODE_CONFIRM_RETURN;
      confirmYesSelected = 0;
      DrawConfirmReturnDialog();
    }
    if(requestConfirmReturn){
      requestConfirmReturn = 0;
      gMode = MODE_MAIN_MENU;
      DrawMainMenu();
    }
    if(requestCancelReturn){
      requestCancelReturn = 0;
      gMode = MODE_PLAYING;
      ST7735_FillScreen(ST7735_BLACK);
      oldWorldX = 255;
      oldWorldY = 255;
    }
    if(gMode == MODE_PLAYING){
      if(oldWorldX != worldX || oldWorldY != worldY){
        drawRoom(worldMap, worldX, worldY);
        oldWorldX = worldX;
        oldWorldY = worldY;
      }
      drawEntities(entList, worldMap, worldX, worldY);
      DrawTopBanner();
      ST7735_SetTextColor(ST7735_WHITE);
      ST7735_SetCursor(0, 2);
      ST7735_OutString("Room:");
      ST7735_OutUDec(worldX);
      ST7735_OutChar(',');
      ST7735_OutUDec(worldY);
      ST7735_SetCursor(0, 3);
      ST7735_OutString("Btns:");
      ST7735_OutUDec(Switch_In());
    }

    
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
