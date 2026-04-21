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
#include "../inc/Clock.h"

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

  /*DAC0->CTL0.RES = 1;
  DAC0->CTL0.ENABLE = 1;
  DAC0->CTL1.OPS = 1;*/

  // Reset DAC and VREF
    // RSTCLR
    //   bits 31-24 unlock key 0xB1
    //   bit 1 is Clear reset sticky bit
    //   bit 0 is reset DAC
  VREF->GPRCM.RSTCTL = 0xB1000003;
  DAC0->GPRCM.RSTCTL = 0xB1000003;

    // Enable power DAC and VREF
    // PWREN
    //   bits 31-24 unlock key 0x26
    //   bit 0 is Enable Power
  VREF->GPRCM.PWREN = 0x26000001;
  DAC0->GPRCM.PWREN = 0x26000001;

  Clock_Delay(24); // time for ADC and VREF to power up

  VREF->CLKSEL = 0x00000008; // bus clock
  VREF->CLKDIV = 0; // divide by 1
  VREF->CTL0 = 0x0001;
  // bit 8 SHMODE = off
  // bit 7 BUFCONFIG=0 for 2.5 (=1 for 1.4)
  // bit 0 is enable
  VREF->CTL2 = 0;
  // bits 31-16 HCYCLE=0
    // bits 15-0 SHCYCLE=0
  while((VREF->CTL1&0x01)==0){}; // wait for VREF to be ready
  // CTL0
  // bit 16 DFM 0 straight binary
  // bit 8 RES 1 12-bit
  // bit 0 ENABLE 1 for on, 0 for disable
  DAC0->CTL0 = 0x0100; // 12-bit, straight, disable
  // CTL1
  // bit 24 OPS 1 OUT0 is selected
  // bit 9 REFSN 0 VR- is VrefN, 1 for analog ground
  // bit 8 REFSP 1 VR+ is VrefP, 0 for analog supply
  // bit 1 AMPHIZ 0 HiZ when disabled
  // bit 0 AMPEN 1 enabled
  //DAC0->CTL1 = (1<<24)|(1<<9)|(1<<8)|1; // 0 to 2.5V
  DAC0->CTL1 = (1<<24)|(1<<9)|1; // 0 to 3.3V
  DAC0->CTL2 = 0; // no DMA, no FIFO trigger
  // CTL3
  // bits 11-8 STIMCONFIG n=0 for 500 sps
  //     for n = 0 to 5 500*2^n sps (500 to  16k)
  //     n=6 100 ksps
  //     n=7 200 ksps
  //     n=8 500 ksps
  //     n=9 1 Msps
  // bit 0 STIMEN =0 disable sample time generator
  DAC0->CTL3 = 0x0700; // no sample time generator
  DAC0->CTL0 = 0x0101; // 12-bit, straight, enable

  SysTick->CTRL  = 0x00;                        // disable SysTick during setup
  SysTick->LOAD = 7271;                         // reload value for 11kHz (assuming 80MHz)
  SysTick->VAL = 0;                             // any write to current clears it
  SCB->SHP[1] = SCB->SHP[1]&(~0xC0000000);      // set priority = 0 (bits 31,30)
  SysTick->CTRL = 0x00000007;                   // enable with core clock and interrupts
  __enable_irq();                               // normally we set I=0 in the main
  Sound_count = 0;

}

void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active

  if (Sound_count > 0) {
      // Read the 8-bit sample and bit-shift it to 12-bit (multiply by 16)
      uint16_t sample_12bit = *Sound_pt; 
      sample_12bit = (sample_12bit<<5);
      
      // Output to the DAC. Use the DriverLib function or register access:
      // DriverLib approach:
      //DL_DAC12_output12(DAC0, sample_12bit);
      
      // Bare-metal approach (if configured correctly):
      // DAC0->DAT[0].DAT_12B = sample_12bit;

      DAC0->DATA0 = sample_12bit;
      

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

