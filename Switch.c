/*
 * Switch.c
 *
 *  Created on: January 12, 2026
 *      Author: Michael McElroy & Alp Shamsapour
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
     IOMUX->SECCFG.PINCM[PB0INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB1INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB2INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB3INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB4INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PA26INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081;
}
// return current state of switches
uint32_t Switch_In(void){
    uint32_t keys;
    keys = ((GPIOA->DIN31_0 >> 26) & 0b111);        //get buttons on A bus
    keys = ((keys<<5) | ((GPIOB->DIN31_0) & 0b11111));   //get buttons on B bus


    return keys; // replace this line
}
