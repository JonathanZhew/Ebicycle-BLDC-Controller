/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : Hall_isr.h
 * Dicrition : Hall interrupt, the driver Output, you must change carefullly
 *  Version : V0.0
 *  Author  : Jonathan
 *      Date : Mar-3-2008
 *
 * Amender:
 *   Content:
 *       Date:
 *
 ***********************************************************************/
void ReadHall();
void PhaseOutput();

#pragma vector=PCINT1_vect   /* Hall Change Interrupt Request PC3\4\5 */ 
__interrupt void Hall_isr(void);

void HallTblReSet();


