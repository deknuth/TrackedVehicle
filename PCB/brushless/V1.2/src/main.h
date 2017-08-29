#ifndef MAIN_H_
#define MAIN_H_
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>
#include <util/twi.h>

#define LOW           0
#define HIGH          (!LOW)
#define FALSE         0
#define TRUE		  (!FALSE)	

#define setbit(x,y) x |= (1<<y)
#define clrbit(x,y) x &= ~(1<<y)


#define SPI_SCS_H	PORTB|=1<<2
#define	SPI_SCS_L	PORTB&=~(1<<2)

#define	SPI_SCK_H	PORTB|=1<<1
#define	SPI_SCK_L	PORTB&=~(1<<1)

#define	SPI_DATA_OUT	DDRB|=1
#define	SPI_DATA_IN   	DDRB&=~1
#define	SPI_DATA_H	PORTB|=1
#define	SPI_DATA_L	PORTB&=~1
#define	SPI_DATA_HL	(PINB&0x01)
/*
#define	LED_ON		PORTB|=(1<<3)
#define	LED_OFF		PORTB&=~(1<<3)
#define	LED_BLINK	PORTB^=(1<<3)
*/
#define BAUD	38400

extern void SendStr(unsigned char* data,unsigned char len);

#endif


