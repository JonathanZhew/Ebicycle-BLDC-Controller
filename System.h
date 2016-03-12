/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : System.h
 * Dicrition : System Define
 *  Version : V0.0
 *  Author  : Jonathan
 *      Date : Mar-3-2008
 *
 * Amender:
 *   Content:
 *       Date:
 *
 ***********************************************************************/
#ifndef SYS_H_H
#define SYS_H_H
#include <intrinsics.h>

 /***  Type definition ***/ 
 typedef unsigned char			Byte;
 typedef unsigned char			Uchar;
 typedef unsigned int 			Uint16;
 typedef int					Int16;
 typedef float					Float16;
 typedef unsigned long int	    Uint32;
 typedef long int 				Int32;
 typedef unsigned char			Bool;
 
 typedef union 
 {
   Uint32 l;
   Uint16 w[2];
   Byte   b[4];
 } Union32;
 
 typedef union 
 { 
   Uint16 w;
   Byte   b[2];
 } Union16;

#define LOW(U16)   ((Byte)U16)
#define HIGH(U16)  ((Byte)(U16>>8))

#define SetFlg(x,y) (x|=(y))
#define ClrFlg(x,y) (x&=~(y))
#define GetFlg(x,y) (x&(y))
#define EORFlg(x,y) (x^=(y))

#define en_Interrupt()	__enable_interrupt( ) //_SEI() or SREG_Bit7=1;
#define dis_Interrupt()	__disable_interrupt( ) //_CLI() or SREG_Bit7=0

#define en_HallISR()	(PCMSK1=0x38)
#define dis_HallISR()	(PCMSK1=0x0)
//-----------------IO Define---------------------------
//PORTB
#define IOB_REVERSE 0
#define IOB_VT      1
#define IOB_VB      2
#define IOB_WT      3
#define IOB_ANGLE   4
#define IOB_VOID    5
#define IOB_EABSEN  6
#define IOB_LOCK    7
//PORTC
#define IOC_ADC_0   0
#define IOC_BRAKE   1
#define IOC_LED     2
#define IOC_HA      3
#define IOC_HB      4
#define IOC_HC      5
#define IOC_RESET   6
#define IOC_        7
//PORTD
#define IOD_ASSIST11 0
#define IOD_TSK     1  //Three speed key
#define IOD_OC      2  //Over Current
#define IOD_WB      3
#define IOD_ALARM   4
#define IOD_UB      5
#define IOD_UT      6
#define IOD_CRUISE  7

//-----------------LED---------------------------------
#define LEDPin PORTC
#define LEDON()  SetFlg(LEDPin,(1<<IOC_LED))
#define LEDOFF() ClrFlg(LEDPin,(1<<IOC_LED))
#define LEDEOR() if(GetFlg(PINC,(1<<IOC_LED))){LEDOFF();}else{LEDON();}//EORFlg(LEDPin,(1<<IOC_LED)) //if LED on, LED off;else, just on
//----------------System Run State-----------------------
//RunState
#define START_RUN           0x01			//wait for RUN
#define ON_RUN              0x02			//Normal RUN
#define BRAKE_RUN           0x04			//Break
#define BLOCK_RUN           0x08			//Block up the motor
#define EABS_RUN            0x10			//EABS
#define LOWVLT_RUN          0x20			//LOW Voltage

//-----------------ASSIST Flag-----------------------------
//AssistFlg   only used in mailloop
#define REGRESS_HANDLE      0x01			//Wait for Handle Regress
#define BRAKE_FALG          0x02
#define reservef2           0x04
#define reservef3           0x08
#define reservef4           0x10

//-----------------HANDEL USED ---------------------------
//PWMState
#define PWM_REFLESH         0x01  //	1.28ms Reflesh once time  
#define HANDLE_OVERVOLT     0x02  //Fly bike, Handle short ciruit
#define LIMIT_CURRENT       0x04  //Over limit Current Flag
#define CONTINUE_CURRENT    0x08  //Current Continuse Flag 
#define HANDLE_ZERO         0x10  //Wait for Handle Restart
#define CRUISE_STATE        0x20  //Cruise
#define PWM_BRAKE           0x40  //Brake
#define PWM_BLOCKUP         0x80

#define MASK_HANDLE_VALID   (HANDLE_OVERVOLT|LIMIT_CURRENT|CRUISE_STATE|PWM_BRAKE|PWM_BLOCKUP)

#define HANDLE_INITIAL_VALUE 6  //Duty=12/255
//#define START_RUN_PWM   20        //8%

//-----------------ADC USED ---------------------------
//ADCState
#define CURRENT_CHNNL           0x01			//ADC State
#define HANDLE_CHNNL            0x02
#define VOLTAGE_CHNNL           0x04
#define CHNNL_CHANGE            0x08
#define CURRENT_BUF_FULL        0x10			//
#define HANDLE_BUF_FULL         0x20
#define VOLTAGE_BUF_FULL        0x40
#define CURRENT_COMPEN          0x80

#define SELECT_CURRENT_CHNNL    (CHNNL_CHANGE|CURRENT_CHNNL)
#define SELECT_HANDLE_CHNNL     (CHNNL_CHANGE|HANDLE_CHNNL)
#define SELECT_VOLTAGE_CHNNL    (CHNNL_CHANGE|VOLTAGE_CHNNL)

#define MASK_CURRENT_PROCESS 	(CURRENT_CHNNL|CURRENT_BUF_FULL)
#define MASK_HANDLE_PROCESS     (HANDLE_CHNNL|HANDLE_BUF_FULL)
#define MASK_VOLTAGE_PROCESS    (VOLTAGE_CHNNL|VOLTAGE_BUF_FULL)
#define MASK_CLEAR_ALLCHNNL     ~(CURRENT_CHNNL|HANDLE_CHNNL|VOLTAGE_CHNNL)

#define MAX_BUF_SIZE 		10
//#define MAX_HANDLE_BUF  10
//#define MAX_VOLT_BUF    10

#define MAX_CRRT_CNT 		78
#define MAX_HANDLE_CNT      10
#define MAX_VOLT_CNT        1

#define ChnnlCrrt()     ADMUX=0x60 // Reference: AVCC pin;  ADC Left Adjust Result
#define ChnnlHndl()     ADMUX=0x66 
#define ChnnlVolt()     ADMUX=0x67 

//Current Peak-Peak Value 
#define CRRT_1A 	0x02 // 0.01V
#define CRRT_1_5A 	0x03 // 0.05V
#define CRRT_2A 	0x04 // 0.07V
#define CRRT_3A 	0x06 // 0.12V
#define CRRT_4A 	0x09 // 0.17V
#define CRRT_5A 	0x0B // 0.21V
#define CRRT_6A 	0x0E // 0.27V
#define CRRT_8A 	0x13 // 0.37V
#define CRRT_10A 	0x18 // 0.47V

#define CRRT_15A 	0x27 // 0.76V
#define CRRT_16A 	0x29 // 0.79V
#define CRRT_17A 	0x2B // 0.82V
#define CRRT_18A 	0x2F // 0.86V

#define CRRT_28A 	0x43 // 1.3V
#define CRRT_30A 	0x48 // 1.4V
#define CRRT_40A 	0x61 // 1.9V
#define CRRT_50A 	0x77 // 2.3V

#define AHEAN_CRRT  CRRT_5A
//Voltage
#define VOLT_40V 	0x43 // 1.3V
#define VOLT_42V 	0x43 // 1.3V
#define VOLT_48V 	0x43 // 1.3V
#define VOLT_60V 	0x43 // 1.3V

//-----------------HALL  USED ---------------------------
#define OCnA 0xc0  // OCnA Inverted PWM;
#define OCnB 0x20  // OCnB Non-Inverted PWM
#define Mode 0x01  // Phase Correct

#define MASK_HALL      0x38     //Hall PC3\4\5 0011 1000
//#define OCnB_DISABLE   0xCF     //TCCRnA Bits 5:4- COMnA1:0: OCnB disconnected.        0b 1100 1111
//#define OCnB_ENABLE    0x20     //                                       OC0B Non-Inverted PWM      0010 000	

#define PHASIC_0640ms  	0x0A

#define PHASIC_1s   	0x3D
#define PHASIC_2s   	0x7A
#define PHASIC_3s      	0xB8
#define PHASIC_4s      	0xF5

//---------------KEY for select funtion---------
//#define SW_DEBUG_H_H
#define UART_H_H 
#define PWM_H_H
#define LIMIT_CURRENT_H_H
#define CONTINUE_CURRENT_H_H
#define CURRENT_COMPEN_H_H
#define BRAKE_H_H
#define BLOCKUP_H_H
#define EABS_F1_H_H
//---------------KEY for some system set--------
#define REGVAR_H_H   //@R4-R15   12pcs Set:c/c++ complier>code
#define SQRT_TBL_H_H //Limit Current adjust Mode:Find in Table

// -------------Adjustable Parameter---------------
#define ACCELERATION_PWM        2 //acceleration 2:1.28ms  3:1.92ms
#define DEAD_TIME_PWM           7 //dead time 7=888ns  6=768ns
#define CONTINUE_CRRNT_TLV      CRRT_1_5A// 88=0.5A(PP)
                                  //TLV:Threshold Limit Values(Threshold:The place or point of beginning)
#endif

