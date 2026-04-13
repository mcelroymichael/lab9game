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
     IOMUX->SECCFG.PINCM[PB1INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB2INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB3INDEX] = 0x00050081;
     IOMUX->SECCFG.PINCM[PB4INDEX] = 0x00050081;
}
// return current state of switches
uint32_t Switch_In(void){
    uint32_t keys;
    keys = ((GPIOB->DIN31_0 >> 1) & 0b1111);
    return keys; // replace this line
}
