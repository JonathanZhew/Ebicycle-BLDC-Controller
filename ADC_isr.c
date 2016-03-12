/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : ADC_isr.c
 * Dicrition : interrupt Conversion Complete
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

#ifdef REGVAR_H_H	
	extern __regvar __no_init unsigned char ADCState @ 15;
	extern __regvar __no_init unsigned char PWMState @ 14;	
	extern __regvar __no_init unsigned char RunState @ 13;
#else
	extern unsigned char volatile ADCState;	  //ADC Select 0:invalid 1:Current 2:Handle 3:Voltage 4:COMPENSATE 
	extern unsigned char volatile PWMState;	
	extern unsigned char volatile RunState;	
#endif

extern  unsigned char PWMWidth;
extern  unsigned char ISRPWMReflesh;
extern Union16 wPWMCnt;

#ifdef CURRENT_COMPEN_H_H
	unsigned char CompenValue; 
#endif
 
unsigned char CrrtBuf[MAX_BUF_SIZE],HandleBuf[MAX_BUF_SIZE],VoltBuf[MAX_BUF_SIZE];


	
#ifdef UART_H_H
	//extern unsigned char LastCompenV,ADC1,ADC2;
#endif


 /****************************************************************************
  * adc_isr() happend when ADC Conversion Complete, PWM Synchronization;
  * 1. Fill the Buffers CrrtBuf[], HandleBuf[], VoltBuf[];
  * 2. Process Change Phase Compenation
  * 3. set PWM Reflesh Flag
  *
  * Output: Set ADCState the Flag of Buffers FULL  
  * 			CURRENT_BUF_FULL		  640 us
  * 			  HANDLE_BUF_FULL		   50	ms
  * 			VOLTAGE_BUF_FULL		  0.5  s
  *
  ***************************************************************************/
#pragma vector=ADC_vect      /* ADC Conversion Complete PC0*/
 __interrupt void adc_isr(void)
 {	
	 static unsigned char IndexCrrt,IndexHandle,IndexVolt;
	 static unsigned char CrrtCnt;	  // VoltCnt = 10*HandleCnt = 10*156*VoltCnt;
	 static unsigned char HandleCnt;  // Time 100ms=10*10ms=10*156*64us
	 //static unsigned char VoltCnt;	//VoltCnt=...
	 static unsigned char PWMFleshCnt;	 
	 static unsigned char ChnnlCnt;  
 
	 if(ADCState&CHNNL_CHANGE)
		 {
		 //Since the next conversion has already started automatically,
		 //the next result will reflect the previous channel selection.
		 ChnnlCnt++;
		 if((ChnnlCnt==2)||(ADCState&CURRENT_COMPEN))  //Max Delay 64*2us
			 {
			 ChnnlCnt=0;
			 ADCState&=(~CHNNL_CHANGE); //clear the flag
			 }
		 }
#ifdef CURRENT_COMPEN_H_H
	else if(ADCState&CURRENT_COMPEN)
		{
		// Clear Mark here, Process PWM+0x80 @Hall_isr()
		if((ADCH > CompenValue)||(wPWMCnt.b[0]>PHASIC_0640ms))	//Set Flag for exit compensate
			{
			//OCR2B=OCR1BL=OCR0B= 0xFF;
			OCR2A=OCR1AL=OCR0A= PWMWidth;	 //POP PWM width value;This word should run ASAP
			
			ADCState&=(~CURRENT_COMPEN);
			//PWMState|=PWM_REFLESH;
			ISRPWMReflesh=1;
			//CompenFlg=0; //why?tell me why, who change my mark CURRENT_COMPEN? 
			//Nobody change yours, but you. CURRENT_COMPEN change at INTERRUPT..20080313
			//Close ADC autorun?
			//LEDOFF();//debug???compen
			}
		}
#endif

	 else if((ADCState&MASK_CURRENT_PROCESS)==CURRENT_CHNNL)
		{		 
		CrrtBuf[IndexCrrt++]=ADCH; 		 //Record Handle Buffer 10 times
		if(MAX_BUF_SIZE==IndexCrrt)
			 {
			 IndexCrrt=0;			 //  640us
			 ADCState|=CURRENT_BUF_FULL;	//Buffer FULL, set MARK @ADCProcess()
#ifdef PWM_H_H			
			 PWMFleshCnt++;
			 if(ACCELERATION_PWM==PWMFleshCnt)	 // 2*640us=1.28ms BE CARE:the time not equably
				 {
				 PWMFleshCnt=0;
				 //PWMState|=PWM_REFLESH;
				 ISRPWMReflesh=1;
				 }
#endif //PWM_H_H	
			 }		 
 
		 CrrtCnt++; 				// 64us
		 if(CrrtCnt>MAX_CRRT_CNT)	//Provent exiting compensate,CrrtCnt Large than MAX_CRRT_CNT debug??
			 {
			 CrrtCnt=0; 						 //Go to Handle
			 ChnnlHndl();
			 ADCState&=MASK_CLEAR_ALLCHNNL;
			 ADCState|=SELECT_HANDLE_CHNNL;  // 5ms
			 }
		 }
	 else if((ADCState&MASK_HANDLE_PROCESS)==HANDLE_CHNNL)
		 {			 
		 HandleBuf[IndexHandle++]=ADCH;  //Record Handle Buffer 10 times
		 if(MAX_BUF_SIZE==IndexHandle)
			 {
			 IndexHandle=0; 		   // 50ms
			 ADCState|=HANDLE_BUF_FULL;
			 }
 
		 HandleCnt++;			   // 5ms
		 if(MAX_HANDLE_CNT==HandleCnt)
			 {
			 HandleCnt=0;								 //Go to Voltage
			 ChnnlVolt();
			 ADCState&=MASK_CLEAR_ALLCHNNL;
			 ADCState|=SELECT_VOLTAGE_CHNNL; //50ms
			 }
		 else
			 {
			 ChnnlCrrt();								  //Go to Current
			 ADCState&=MASK_CLEAR_ALLCHNNL;
			 ADCState|=SELECT_CURRENT_CHNNL;
			 }
		 }
	 else if((ADCState&MASK_VOLTAGE_PROCESS)==VOLTAGE_CHNNL)
		 {		 
		 VoltBuf[IndexVolt++]=ADCH; 	 //Record Voltage Buffer 10 times
		 if(MAX_BUF_SIZE==IndexVolt)
			 {
			 IndexVolt=0;			  //500ms
			 ADCState|=VOLTAGE_BUF_FULL; 
			 }
#ifdef EABS_F1_H_H
	     if(!(RunState&EABS_RUN))	//Just include EABS???if lock?
#endif
			{		 	
			ChnnlCrrt();				 //50ms
			ADCState&=MASK_CLEAR_ALLCHNNL; 	 // Clear all chnannel
			ADCState|=SELECT_CURRENT_CHNNL;	 //Set currnt channel
	     	}
		 }	 
 }


