/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : PWM_isr.h
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

#pragma vector=TIMER0_OVF_vect   /* PWM_isr = ADC_isr */ 
 __interrupt void PWM_isr(void);


