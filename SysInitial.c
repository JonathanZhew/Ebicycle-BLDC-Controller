/********************************************************************
 * Copyright (c) 2008,AIT(China)
 * All rights reserved.
 *
 *       FILE : SysInitial.c
 * Dicrition : any things about System Initial
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
#include <intrinsics.h>
#include "System.h"
//#include "Variable.h"

/****************************************************************************
 * SysSFRInitial() 
 * 1. Deadtime PWM initialization
 * 2. ADC initialization  
 *
 *
 * Notice: MUST call __delay_cycles(1000) againt TCCR1A set PWM Output pull I/O LOW
 *
 ***************************************************************************/
void SysSFRInitial()
{	
  //Deadtime PWM initialization
  TCCR0A=0xE1;  // OC0A Inverted PWM; OC0B Non-Inverted PWM
  TCCR0B=0x01;  // Phase correct PWM top=FFh; Clock value: 8000.000 kHz
  //TCNT0=0x00; // PWM Synchronization
  OCR0A=0x00;   //Close TopBridge. Output Hi
  OCR0B=0xff;   //Close BottomBridge. Output Hi
  
  TCCR1A=0xE1; // OC0A Inverted PWM; OC0B Non-Inverted PWM
  TCCR1B=0x01; // Phase correct PWM top=FFh; Clock value: 8000.000 kHz
  TCNT1H=0x00;
  //TCNT1L=0x00; // PWM Synchronization
  ICR1H=0x00;    //no used
  ICR1L=0x00;
  OCR1AH=0x00;
  OCR1AL=0x00;
  OCR1BH=0x00;
  OCR1BL=0xff;

  ASSR=0x00;
  TCCR2A=0xE1;
  TCCR2B=0x01;
  //TCNT2=0x00; // PWM Synchronization
  OCR2A=0x00;
  OCR2B=0xff;
  
  // PWM Synchronization
  TCNT2=0x00; 
  TCNT1L=0x03;   
  TCNT0=0x05;  
	
	__delay_cycles(1000);  //130us, provent ALL Driver Output same time???
	//Input/Output Ports initialization
	PORTB = 0x0E;
	DDRB  = 0x0E;
	PORTD = 0x68;
	DDRD  = 0x68;

	DDRC  = (1<<IOC_LED); //LED

#ifdef UART_H_H
  // USART initialization
  // Communication Parameters: 8 Data, 1 Stop, No Parity
  // USART Receiver: Off
  // USART Transmitter: On
  // USART0 Mode: Asynchronous
  // USART Baud rate: 9600
  UCSR0A=0x00; 
  UCSR0B=0x08;
  UCSR0C=0x06;
  UBRR0H=0x00;
  UBRR0L=0x33;
#endif
 
  // ADC initialization  
  DIDR0=0x00;  // Digital input buffers on; off can reduce power consumption
  ADMUX=0x60;  // Reference: AVCC pin;  ADC Left Adjust Result
  ADCSRA=0xAB; // ADC Clock frequency: 0xAC=500 kHz   26.4us 38kpis;0xAB=1 MHz 13us; Must <1/4duty for compen
  ADCSRB=0x04; // ADC Auto Trigger Source: Timer0 Overflow

  // External Interrupt(s) initialization
	// INT0: On
	// INT0 Mode: Falling Edge
	// INT1: Off
	// Interrupt on any change on pins PCINT0-7: Off
	// Interrupt on any change on pins PCINT8-14: On
	// Interrupt on any change on pins PCINT16-23: Off
	EICRA=0x02;
	EIMSK=0x01;
	EIFR=0x01;
	PCICR=0x02;
	PCMSK1=0x38;
	PCIFR=0x02;
  
  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0=0x01;
  // Timer/Counter 1 Interrupt(s) initialization
  TIMSK1=0x00;
  // Timer/Counter 2 Interrupt(s) initialization
  TIMSK2=0x00;  
}


