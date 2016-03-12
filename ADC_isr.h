/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : ADC_isr.h
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
#pragma vector=ADC_vect      /* ADC Conversion Complete PC0*/
__interrupt void adc_isr(void);

