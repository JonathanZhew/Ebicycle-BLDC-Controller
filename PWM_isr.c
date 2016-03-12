/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : PWM_isr.c
 * Dicrition : 
 *  Version : V0.0
 *  Author  : Jonathan
 *      Date : Mar-3-2008
 *
 * Amender:
 *   Content:
 *       Date:
 *
 ***********************************************************************/
#include <ioavr.h>
#include "System.h"

extern Union16 wPWMCnt;

/****************************************************************************
 * Hall_isr()           PWM_isr = ADC_isr 
 * 1. Caqulater the time
 * 2. Caqulater the speed
 * 3. 
 *
 * Iput:
 * Output: CountValue
 *
 ***************************************************************************/
#pragma vector=TIMER0_OVF_vect   /* PWM_isr = ADC_isr */ 
__interrupt void PWM_isr(void)
{  
	if(wPWMCnt.b[1]<0xFF)
		wPWMCnt.w++;
}


