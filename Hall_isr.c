/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : Hall_isr.c
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
#include <ioavr.h>
#include "System.h"

extern unsigned char CurHall,LastHall,LastLastHall;
extern unsigned char HallPoint;	 //CurHall change to a No, Value=0~5

#ifdef REGVAR_H_H	
	extern __regvar __no_init unsigned char ADCState @ 15;
	extern __regvar __no_init unsigned char RunState @ 13;

#else
	extern unsigned char volatile ADCState;	  //ADC Select 0:invalid 1:Current 2:Handle 3:Voltage 4:COMPENSATE 
	extern unsigned char volatile RunState;
#endif


#ifdef CURRENT_COMPEN_H_H
	extern unsigned char PWMWidth,AverageCrrtValue;
	extern unsigned char CompenValue; 
	extern unsigned char CrrtValue; 
#endif
	extern Union16 wPWMCnt;

#ifdef UART_H_H
	unsigned char LastCompenV,ADC1,ADC2; //debug???Compen
#endif


unsigned char HallTbl[6]; //Change 120/60 and Interval/Not-interval
#if 0
__flash unsigned char OC0A_TABLE[6]={OCnA|Mode,Mode,Mode,Mode,Mode,OCnA|Mode};
__flash unsigned char OC1A_TABLE[6]={Mode,OCnA|Mode,OCnA|Mode,Mode,Mode,Mode};
__flash unsigned char OC2A_TABLE[6]={Mode,Mode,Mode,OCnA|Mode,OCnA|Mode,Mode};
#endif
//OCnB_TABLE[] Just for EABS
__flash unsigned char OC0B_TABLE[6]={OCnB|Mode,OCnB|Mode,Mode,Mode,OCnB|Mode,OCnB|Mode};
__flash unsigned char OC1B_TABLE[6]={OCnB|Mode,OCnB|Mode,OCnB|Mode,OCnB|Mode,Mode,Mode};
__flash unsigned char OC2B_TABLE[6]={Mode,Mode,OCnB|Mode,OCnB|Mode,OCnB|Mode,OCnB|Mode};

__flash unsigned char OC0AB_TABLE[6]={OCnA|OCnB|Mode,Mode,Mode,Mode,Mode,OCnA|OCnB|Mode};
__flash unsigned char OC1AB_TABLE[6]={Mode,OCnA|OCnB|Mode,OCnA|OCnB|Mode,Mode,Mode,Mode};
__flash unsigned char OC2AB_TABLE[6]={Mode,Mode,Mode,OCnA|OCnB|Mode,OCnA|OCnB|Mode,Mode};

__flash unsigned char PORTB_TABLE[6]={0xff,0xff,0xff,0xff,0xfb,0xfb};
__flash unsigned char PORTD_TABLE[6]={0xf7,0xf7,0xdf,0xdf,0xff,0xff};

__flash unsigned char HALL_TBL120[6]={0x28,0x20,0x30,0x10,0x18,0x08};
__flash unsigned char HALL_TBL60[6] ={0x00,0x08,0x18,0x38,0x30,0x20};


#ifdef UART_H_H
extern void putchar(char c);
#endif


/****************************************************************************
 * HallTblReSet() 
 * 1. Initial the Offset Current Value before the mainloop;
 * 2. When the angle 120/60 or Forward/Backward change, MUST call it
 *
 * Iput: the Flag of angle 120/60 or Forward/Backward
 *
 * Output: HallTbl (Renew the table for Hall_isr())
 *
 ***************************************************************************/
void HallTblReSet()
{
	unsigned char i;
	for(i=0;i<6;i++)
		HallTbl[i]=HALL_TBL120[i]; 
}


/****************************************************************************
 * ReadHall() 
 *
 ***************************************************************************/
void ReadHall()
{
	unsigned char i;

	HallPoint=6;	//6 is illegality Value 
	
	i=PINC&MASK_HALL;      			//Read two times
	CurHall=PINC&MASK_HALL;
	if(CurHall!=i)
		return;	
	
if(ON_RUN==RunState)  //only@ ON_RUN	no Other EABS,Start ect.
	{
	if(CurHall==LastHall)  			//different from last Hall
		return;	
	if((wPWMCnt.b[0]<PHASIC_0640ms)&&(wPWMCnt.b[1]==0)) 	//Hall interrupt space >0.64ms
		return;
	}

	for(i=0;i<6;i++)
		{
		if(CurHall==HallTbl[i])
			{
			wPWMCnt.w=0;
			HallPoint=i;
			break;
			}
		}
}

/****************************************************************************
 * PhaseOutput() Can Unite with ReadHall()??? debug
 * Initial the Offset Current Value, when the Current is ZERO
 *
 * Iput:CrrtBuf
 * Output: OffsetCrrtValue *
 *
 ***************************************************************************/
#define SYN_VALUE 0 
void PhaseOutput()
{
#ifdef CURRENT_COMPEN_H_H
		unsigned char CompenPWMWidth;
#endif

if(HallPoint!=6)
	{	
	if((RunState&(START_RUN|ON_RUN|EABS_RUN))==(START_RUN|ON_RUN))	//@START_RUN or ON_RUN, but not EABS_RUN
		{
		// PWM Synchronization
		TCNT2=0x00+SYN_VALUE; 
		TCNT1L=0x03+SYN_VALUE;   
		TCNT0=0x05+SYN_VALUE; 
		
        //TCCR0A=OC0A_TABLE[HallPoint];//if delete, will short ciruit decelerate
        //TCCR1A=OC1A_TABLE[HallPoint];//maybe it can delete, but MUST test & test
        //TCCR2A=OC2A_TABLE[HallPoint]; 
        PORTB=PORTB_TABLE[HallPoint];
        PORTD=PORTD_TABLE[HallPoint];
		
        TCCR0A=OC0AB_TABLE[HallPoint];
        TCCR1A=OC1AB_TABLE[HallPoint];
        TCCR2A=OC2AB_TABLE[HallPoint];
		
		
#ifdef CURRENT_COMPEN_H_H // Add Enterence Condition
		if((AverageCrrtValue>CRRT_3A)&&(PWMWidth>40)) //Current Value >1A,debug Conpen Start PWM???
			{			
			if(PWMWidth<0x80)
				CompenPWMWidth=PWMWidth+0x80;
			else
				CompenPWMWidth=0xff;			

			OCR2B=OCR1BL=OCR0B= 0xFF;			  //Close Corrent Continuse								
			OCR2A=OCR1AL=OCR0A= CompenPWMWidth;   //dead time 7=888ns  6=768ns	  
			//Set ADC autorun			

			//LEDON();  //debug???compen
					
			if(!(ADCState&CURRENT_CHNNL))
				{
				ChnnlCrrt();								 //Go to Current
				//ADCState&=MASK_CLEAR_ALLCHNNL;
				//ADCState|=SELECT_CURRENT_CHNNL;
				}
			//ADCState|=CURRENT_COMPEN;
			
			ADCState&=MASK_CLEAR_ALLCHNNL;
			ADCState|=(SELECT_CURRENT_CHNNL|CURRENT_COMPEN);
			
			CompenValue=CrrtValue-AHEAN_CRRT;
			} 
#endif
		}
#ifdef EABS_F1_H_H
	else if(RunState&EABS_RUN)	//Just include EABS_RUN?? no @START_RUN or ON_RUN
		{
        PORTB=PORTB_TABLE[HallPoint]; //bottom bridge LOW unchange
        PORTD=PORTD_TABLE[HallPoint];
		
        TCCR0A=OC0B_TABLE[HallPoint];//up bridge close
        TCCR1A=OC1B_TABLE[HallPoint];//bottom bridge other TOW PWM
        TCCR2A=OC2B_TABLE[HallPoint];
		}
#endif
	}
else
	{
#ifdef UART_H_H 
		putchar(0xE1);  //
		putchar(0x01);	//Hall Error 01
#endif
	}
	
}

/****************************************************************************
 * Hall_isr() Happen when Hall Change Interrupt PC3\4\5
 * 1. Read the New Hall
 * 2. Driver OutPut
 * 3. Record the Run State
 *
 * Iput:
 * Output: Driver Ouput
 *
 ***************************************************************************/
#pragma vector=PCINT1_vect   /* Hall Change Interrupt Request PC3\4\5 */ 
__interrupt void Hall_isr(void)
{  
	
	ReadHall();
	PhaseOutput();

	LastLastHall=LastHall;
	LastHall=CurHall;
}


