/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : ADCProcess.c
 * Dicrition : Process ADC
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
#else
	extern unsigned char volatile ADCState;	  //ADC Select 0:invalid 1:Current 2:Handle 3:Voltage 4:COMPENSATE 
	extern unsigned char volatile PWMState;	
#endif

extern  unsigned char PWMWidth;
extern  Union16 wPWMCnt;

extern  unsigned char CrrtValue,HandleValue,VoltValue;
extern  unsigned char OffsetCrrtValue,PPCrrtValue,AverageCrrtValue;

extern unsigned char CrrtBuf[MAX_BUF_SIZE],HandleBuf[MAX_BUF_SIZE],VoltBuf[MAX_BUF_SIZE];

__flash unsigned char AVERAGE_CURRNT_VALUE=CRRT_16A;
__flash unsigned char PP_CURRNT_VALUE=CRRT_28A;

#ifdef SQRT_TBL_H_H
__flash unsigned char SQRT_TBL[256]={
	  3,  8, 12, 15, 18, 20, 22, 24, 26, 28, 30, 32, 33, 35, 36, 38,
	 40, 41, 42, 44, 45, 47, 48, 49, 51, 52, 53, 55, 56, 57, 58, 60,
	 61, 62, 63, 64, 66, 67, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78,
	 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
	 96, 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,
	111,112,113,114,115,116,117,118,119,119,120,121,122,123,124,125,
	126,127,128,129,130,130,131,132,133,134,135,136,137,138,138,139,
	140,141,142,143,144,145,145,146,147,148,149,150,151,151,152,153,
	154,155,156,157,157,158,159,160,161,162,162,163,164,165,166,167,
	168,168,169,170,171,172,172,173,174,175,176,177,177,178,179,180,
	181,182,182,183,184,185,186,186,187,188,189,190,190,191,192,193,
	194,194,195,196,197,198,198,199,200,201,202,202,203,204,205,206,
	206,207,208,209,210,210,211,212,213,213,214,215,216,217,217,218,
	219,220,220,221,222,223,224,224,225,226,227,227,228,229,230,230,
	231,232,233,234,234,235,236,237,237,238,239,240,240,241,242,243,
	243,244,245,246,246,247,248,249,249,250,251,252,252,253,254,255};
#endif

#ifdef UART_H_H
extern  unsigned char CompenPWMCnt,ADCStateTmp,ADC1,ADC2;
extern void putchar(char c);
#endif

/****************************************************************************
 * Average={SUM(ADCBuf[MAX_BUF_SIZE])-MaxValue-MaxValue}/(MAX_BUF_SIZE-2)
 *
 * Input: Address of ADCBuf
 * Output: Average
 ****************************************************************************/
unsigned char Average(unsigned char* ADCBuf)
{
	unsigned char i;
	unsigned char MaxValue,MixValue;
	unsigned int  wSum;
	
	MaxValue=0;
	MixValue=255;
	wSum=0;

	for(i=0;i<MAX_BUF_SIZE;i++) //??Can make a Funtion char Varage(char CrrtBuf[]);?
		{
		if(ADCBuf[i]>MaxValue)
			MaxValue=ADCBuf[i];
		if(ADCBuf[i]<MixValue)
			MixValue=ADCBuf[i];		
		wSum+=(unsigned int)ADCBuf[i]; 
		}
#if 1
	wSum-=MixValue;
	wSum-=MaxValue;
	wSum>>=3;   //wSum=wSum/8
#else
	wSum/=10;
#endif 
	
	return LOW(wSum);    //??Do you sure LOW(wSum)?
}

/****************************************************************************
 * ADCProcess() revolving in mainloop
 * Process many things 
 *    1. change HandleValue Range to 0~255 
 *    2. Current Limit flag
 *    3. Set/Clear Continue Current Flag
 * Input :     CrrtBuf[], HandleBuf[], VoltBuf[];
 *             CURRENT_BUF_FULL          640 us
 *               HANDLE_BUF_FULL          50   ms
 *             VOLTAGE_BUF_FULL          0.5  s
 *
 * Output :     CrrtValue,  
 *              HandleValue,
 *                  VoltValue;
 *              Clear the FULL Flag
 ***************************************************************************/
void ADCProcess()
{
    Union16  wPPTmp;
//	if(!(ADCState&CURRENT_COMPEN))
//		{
		//CrrtValue=(Sum(CrrtBuf[])-MaxValue-MixValue)/8
	if(ADCState&CURRENT_BUF_FULL)
		{						
		CrrtValue=Average(CrrtBuf);
		dis_Interrupt();
			ADCState&=~CURRENT_BUF_FULL;   //Clear FULL Flag
        en_Interrupt();
		if(CrrtValue>OffsetCrrtValue)
			{
			PPCrrtValue=CrrtValue-OffsetCrrtValue;
			
			//AverageCrrtValue=((unsigned int)PPCrrtValue*PWMWidth)/255;
			wPPTmp.w=(Uint16)PPCrrtValue;
#ifdef SQRT_TBL_H_H//2008-03-20
			wPPTmp.w*=(Uint16)SQRT_TBL[PWMWidth]; 
#else
			//wPPTmp.w*=(Uint16)PWMWidth;
			wPPTmp.w*=(Uint16)(PWMWidth-PWMWidth/8+50);
#endif
			AverageCrrtValue=wPPTmp.b[1];
			}
		else
			{
			PPCrrtValue=AverageCrrtValue=0;
			}
			
#ifdef LIMIT_CURRENT_H_H
        //Current Limit Flag, Process @PWMProcess()
		if((AverageCrrtValue>AVERAGE_CURRNT_VALUE)||(PPCrrtValue>PP_CURRNT_VALUE))
			{
				if(ADCState&CURRENT_COMPEN) //the same as (CompenFlag!=0) @ Compen State
					PWMState|= LIMIT_CURRENT;
				else
					PWMState|=(LIMIT_CURRENT|PWM_REFLESH);
	#ifdef BLOCKUP_H_H
			if(wPWMCnt.b[1]>PHASIC_2s) //How long do you wanna? do @Limint Current
				{PWMState|=PWM_BLOCKUP;}
	#endif
			}
		else
			{
			PWMState&=(~LIMIT_CURRENT);
			}	
#endif		

#ifdef CONTINUE_CURRENT_H_H
		//Set/Clear Continue Current Flag
		if(AverageCrrtValue>CONTINUE_CRRNT_TLV)
			{
			PWMState|=CONTINUE_CURRENT;
			}
		else
			{
			PWMState&=(~CONTINUE_CURRENT);
			}
#endif
		}

		
	if(ADCState&HANDLE_BUF_FULL)
		{			
		HandleValue=Average(HandleBuf);
		
#ifdef UART_H_H 
			//putchar(HandleValue);
#endif
		dis_Interrupt();
			ADCState&=~HANDLE_BUF_FULL;
		en_Interrupt();
		
		if((HandleValue<50+HANDLE_INITIAL_VALUE)||(HandleValue>220))      //0.8V Cannot Change!! 1.1>X>4.3V
			{HandleValue=0;}
		else if(HandleValue>177) //3.5V
			{HandleValue=255;}
		else
			{
		    HandleValue=(HandleValue-50)*2;
			}
		}

	if(ADCState&VOLTAGE_BUF_FULL)
		{			
		VoltValue=Average(VoltBuf);
		dis_Interrupt();
			ADCState&=~VOLTAGE_BUF_FULL;
		en_Interrupt();
			
#ifdef UART_H_H 
	//putchar(PPCrrtValue);
	//putchar(HandleValue);
	//putchar(AverageCrrtValue);
	//putchar(CompenPWMCnt);
	//putchar(ADCStateTmp);
	putchar(ADC1);
	putchar(ADC2);
	if(GetFlg(PINC,(1<<IOC_LED)))
		{LEDOFF();}
	else
		{LEDON();}
	//putchar(0xAA);
#endif
		}					
//		}
}

/****************************************************************************
 * ReadCurrentOffset() 
 * Initial the Offset Current Value, when the Current is ZERO
 *
 * Iput:CrrtBuf
 * Output: OffsetCrrtValue *
 *
 ***************************************************************************/
void ReadCurrentOffset()
{
	while(!(ADCState&CURRENT_BUF_FULL));
				
   	OffsetCrrtValue=Average(CrrtBuf);
	ADCState&=~CURRENT_BUF_FULL;   //Clear FULL Flag
#ifdef UART_H_H 	
	putchar(OffsetCrrtValue);
    putchar(0x0F); //0xof short for offset
#endif
}



