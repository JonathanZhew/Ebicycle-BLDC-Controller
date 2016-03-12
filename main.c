/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : main.c
 * Dicrition : This program was produced by the
 *                IAR C/C++ Compiler for AVR
 *                4.30A/W32 [Evaluation] (4.30.1.3)
 *                Chip type           : ATmega88
 *                Program type     : Application
 *                Clock frequency : 8.000000 MHz(inside)
 *                Memory model   : Small
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
#include "Variable.h"
#include "SysInitial.h"
#include "ADCProcess.h"
#include "Hall_isr.h"

unsigned char AssistFlg;
unsigned char EABS_PWMWidth;  

#ifdef UART_H_H
extern unsigned char ADC1,ADC2;
void putchar(char c)
{
  UDR0=c;
  while(!(UCSR0A&0x40));
}
#endif

void StopOutput()
{
	PORTB=0xff;
	PORTD=0xff;

	TCCR0A=Mode;
	TCCR1A=Mode;
	TCCR2A=Mode;
}

#pragma vector=INT0_vect   /* Over Current Interrupt Request PD3*/ 
__interrupt void OverCrrt_isr(void)
{  
#ifdef UART_H_H 
	putchar(0xEE);
	putchar(0x0C);
#endif
}


/****************************************************************************
 * PWMProcess() revoling in mainloop
 *    Process anything about PWM width
 *   1. Handle Control PWM
 *   2. Limit Current reduce PWM width
 *   3. Open/Close Bottom Bridge, Continue
 *   3. Cruise lock PWM
 *   4. Brake Cut PWM
 *
 *     Iput:HandleValue
 *        
 * Output: nothing
 *
 ***************************************************************************/
void PWMProcess() 
{
  static unsigned char OldPWMWidth;

  if((PWMState&PWM_REFLESH)||(ISRPWMReflesh))
  	{
    //no handle overvolt, no limit current, no in cruise, no brake..any else???
	if((PWMState&MASK_HANDLE_VALID)==0)
		{
		if(PWMWidth<HandleValue)
			PWMWidth++;
		if(PWMWidth>HandleValue)
			PWMWidth--;
		}
#ifdef LIMIT_CURRENT_H_H
	else if(PWMState&(LIMIT_CURRENT|PWM_BLOCKUP|PWM_BRAKE))
		{
		if(PWMWidth>0) //debug?? delet it
		    PWMWidth--;
		}
#endif

	if(PWMWidth==OldPWMWidth)
		return;

  if(!(ADCState&CURRENT_COMPEN)) //the same as (!(ADCState&CURRENT_COMPEN)) not @ Compen State
	{
	dis_Interrupt();
	OCR2A=OCR1AL=OCR0A= PWMWidth;	//dead time 7=888ns  6=768ns    
#ifdef CONTINUE_CURRENT_H_H
    //CONTINUE_CURRENT Flag Set AND PWM lower the 0xFF-DEAD_TIME_PWM
	if((PWMState&CONTINUE_CURRENT)&&(PWMWidth<(0xFF-DEAD_TIME_PWM)))
		{
        OCR2B=OCR1BL=OCR0B= (PWMWidth+DEAD_TIME_PWM); //Open
		}
	else
		{
		OCR2B=OCR1BL=OCR0B= 0xFF; //Close Corrent Continuse 
		}
#else
    OCR2B=OCR1BL=OCR0B= 0xFF;
#endif
	en_Interrupt();
    }
   dis_Interrupt();
    ISRPWMReflesh=0;
   en_Interrupt();
   
	PWMState&=(~PWM_REFLESH); 
  	OldPWMWidth=PWMWidth;
  	}
}

#ifdef BRAKE_H_H
/****************************************************************************
 * BrakeIOTest() revoling in mainloop
 *    Process anything about Brake
 *   1. Test IO pin, Set/Clr BRAKE Flag
 *   2. Set/Clr EABS Flag
 *
 * Iput:IO Pin
 *        
 * Output: PWMState, RunState...
 *
 ***************************************************************************/
void BrakeIOTest()
{
if(AssistFlg&BRAKE_FALG)  //in Brake State, Must Run ALL time.
	{			
	if(GetFlg(PINC,(1<<IOC_BRAKE)))//IO is High
		{
		PWMState&=~PWM_BRAKE;  
		PWMState|=PWM_REFLESH;
		AssistFlg&=~BRAKE_FALG;
		//AssistFlg|=REGRESS_HANDLE;//you cann't do that, or it can awake handle
#ifdef EABS_F1_H_H
		RunState&=~EABS_RUN;
#endif
		}	
#ifdef EABS_F1_H_H
	else if(0==PWMWidth) //@PWM_BRAKE State AND PWMWidth==0
		{
		//OCR2A=OCR1AL=OCR0A=0
		//OCR2B=OCR1BL=OCR0B=0xff 
		RunState|=EABS_RUN;
		}				
#endif
	}
else 
	{
	if(!GetFlg(PINC,(1<<IOC_BRAKE)))  //IO is LOW, in effect
		{
		PWMState|=PWM_BRAKE;  
		AssistFlg|=BRAKE_FALG;
		}
	}
}
#endif	

/****************************************************************************
 * main() No need I tell you anything
 *   1. Initial System
 *   2. mainloop
 *   3. 
 *   4. 
 *
 *
 ***************************************************************************/
 void main( void )
{
  dis_Interrupt();
    SysSFRInitial(); 
	
    HallTblReSet();        //??debug
    ADCState=SELECT_CURRENT_CHNNL;
	PWMState=PWM_REFLESH;
	RunState=START_RUN;		//anti Fly bike
	AssistFlg=0;
	HandleValue=25;		// Not Zero, anti Fly bike with  AssistFlg=0;
	wPWMCnt.b[1]=1;
	PWMWidth=0; //debug if not reset OK, will short ??
    ADCSRA|=0x40; //ADC Start
#ifdef UART_H_H 
    putchar(0xAE); 
    putchar(0xBB); 
	//LEDOFF();
	LEDON();
#endif
	//ReadHall();  //debug
	//PhaseOutput(); //debug
  en_Interrupt();
  dis_HallISR();
  StopOutput(); //MUST together with  dis_HallISR()!!

#ifndef SW_DEBUG_H_H
	ReadCurrentOffset();
#endif
	while(1)
		{
		ADCProcess();	
		//PWMProcess();
		
//---------------------->Start Run<----------------------------
		if(START_RUN==RunState) //only @START_RUN
			{
		  	if(HandleValue)//HandleValue>StartPWMValue
				{
				if(AssistFlg&REGRESS_HANDLE)//Handle had Regresssed!
					{
					AssistFlg&=~REGRESS_HANDLE;						
					ReadHall();            
					PhaseOutput(); 	
					PWMState&=~PWM_BLOCKUP;//Set @ADCProcess()	
					RunState&=~START_RUN;
					RunState|=ON_RUN;
					en_HallISR();
					}
				}
		  	else
			  	{
			  	AssistFlg|=REGRESS_HANDLE; //Set Regress Flag				
			  	}
			}
//---------------------->On Runing State<----------------------------		
		if(ON_RUN==RunState)    //only @ON_RUN
			{
			PWMProcess();	

			if((PWMWidth==0)&&(!(AssistFlg&BRAKE_FALG)))          //Back to the Future 1
				{
				RunState&=~ON_RUN;
				RunState|=START_RUN;
				dis_HallISR();
				StopOutput();              //Driver Cut off
				}						
			}
		
#ifdef EABS_F1_H_H
//---------------------->EABS F1 Style<----------------------------	
		if(RunState&EABS_RUN)	//Just include EABS, but no LOCK????
			{
			  if((PWMState&PWM_REFLESH)||(ISRPWMReflesh))
			  	{
			  	if(VoltValue>VOLT_60V)
			  		{EABS_PWMWidth++;}
				else if(EABS_PWMWidth>0x10)
			  		{EABS_PWMWidth--;} 
				OCR2B=OCR1BL=OCR0B=EABS_PWMWidth;
			  	}
			}
#endif

#ifdef UART_H_H
ADC1=AssistFlg;
ADC2=RunState;
#endif

		}
}

