/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : Variable.h
 * Dicrition : Variable of System, you can use them
 *  Version : V0.0
 *  Author  : Jonathan
 *      Date : Mar-3-2008
 *
 * Amender:
 *   Content:
 *       Date:
 *
 ***********************************************************************/
#ifndef VARIABLE_H_H
#define VARIABLE_H_H

//Publise
	unsigned char CurHall,LastHall,LastLastHall;
	unsigned char HallPoint;   //CurHall change to a No, Value=0~5

	unsigned char CrrtValue,HandleValue,VoltValue;//ADC original Value
	unsigned char OffsetCrrtValue,PPCrrtValue,AverageCrrtValue;	
	unsigned char PWMWidth;
	unsigned char ISRPWMReflesh;

#ifdef REGVAR_H_H	
	__regvar __no_init unsigned char ADCState @ 15;
	__regvar __no_init unsigned char PWMState @ 14;	
	__regvar __no_init unsigned char RunState @ 13;
#else
	unsigned char volatile ADCState;	  //ADC Select 0:invalid 1:Current 2:Handle 3:Voltage 4:COMPENSATE 
	unsigned char volatile PWMState;
	unsigned char volatile RunState;	
#endif
	
	Union16 wPWMCnt;


	
  //Eeprom

  //__flash

//Protect

//Privite


#endif

