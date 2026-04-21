// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// Alp Shamsapour & Michael McElroy
// Last Modified: April 19, 2026 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

const uint8_t *Sound_pt;
uint32_t Sound_count = 0;

void SysTick_IntArm(uint32_t period, uint32_t priority){
  SysTick->CTRL = 0x00;                                     // disable SysTick during setup
  SysTick->LOAD = period-1;                                 // reload value
  SCB->SHP[1] = (SCB->SHP[1]&(~0xC0000000))|(priority<<30); // priority 2
  SysTick->VAL = 0;                                         // any write to VAL clears COUNT and sets VAL equal to LOAD
  SysTick->CTRL = 0x07;                                     // enable SysTick with 80 MHz bus clock and interrupts
}

// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
  SysTick->CTRL  = 0x00;                        // disable SysTick during setup
  SysTick->LOAD = 7271;                         // reload value for 11kHz (assuming 80MHz)
  SysTick->VAL = 0;                             // any write to current clears it
  SCB->SHP[1] = SCB->SHP[1]&(~0xC0000000);      // set priority = 0 (bits 31,30)
  SysTick->CTRL = 0x00000007;                   // enable with core clock and interrupts
  __enable_irq();                               // normally we set I=0 in the main
}

void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active

  if (Sound_count > 0) {
      // Read the 8-bit sample and bit-shift it to 12-bit (multiply by 16)
      uint16_t sample_12bit = *Sound_pt; 
      sample_12bit = (sample_12bit<<4);
      
      // Output to the DAC. Use the DriverLib function or register access:
      // DriverLib approach:
      //DL_DAC12_output12(DAC0, sample_12bit);
      
      // Bare-metal approach (if configured correctly):
      // DAC0->DAT[0].DAT_12B = sample_12bit;

      Sound_pt++;    // Move to the next sample
      Sound_count--; // Decrement the samples left
  
  }

}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){

  Sound_pt = pt;
  Sound_count = count;
  
}

void Sound_Shoot(void){
  Sound_Start(shoot, 4080);
}

void Sound_Killed(void){
  //Sound_Start(killed, 4080);
}
void Sound_Explosion(void){
  Sound_Start(explosion, 4080);
}

void Sound_Fastinvader1(void){
  Sound_Start(fastinvader1, 4080);
}
void Sound_Fastinvader2(void){

}
void Sound_Fastinvader3(void){

}
void Sound_Fastinvader4(void){

}
void Sound_Highpitch(void){

}

