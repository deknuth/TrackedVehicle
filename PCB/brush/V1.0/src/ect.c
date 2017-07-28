#include <stdlib.h>
#include <math.h>
#include <util/delay.h>
#include <avr/iom48p.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#define setbit(x,y) x |= (1<<y) 
#define clrbit(x,y) x &= ~(1<<y)
#ifndef BIT
#define BIT(x) _BV(x)
#endif
#define BBO	0xF8			// Byte order  big endian	high three bit
#define LBO	0x07			// Byte order  little endian	low three bit

volatile unsigned int count = 0;		// timing

void CloseHB(void);

enum LED{
	RUN = (1<<4),
	FALT = (1<<5),
};

const unsigned int porp[256] = {
	0,0,0,0,0,1,2,3,4,5,
	6,7,8,9,10,11,12,13,14,15,
	16,17,18,19,20,21,22,23,24,25,
	26,27,28,29,30,31,32,33,34,35,
	36,37,38,39,40,41,42,43,44,45,
	46,47,48,49,50,51,52,53,54,55,
	56,57,58,59,60,61,62,63,64,65,
	66,67,68,69,70,71,72,73,74,75,
	76,77,78,79,80,81,82,83,84,85,
	86,87,88,89,90,91,92,93,94,95,
	96,97,98,99,100,101,102,103,104,105,
	106,107,108,109,110,111,112,113,114,115,
	116,117,118,119,120,121,122,123,124,125,
	126,127,128,129,130,131,132,133,134,135,
	136,137,138,139,140,141,142,143,144,145,
	146,147,148,149,150,151,152,153,154,155,
	156,157,158,159,160,161,162,163,164,165,
	166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,
	186,187,188,189,190,191,192,193,194,195,
	196,197,198,199,200,201,202,203,204,205,
	206,207,208,209,210,211,212,213,214,215,
	216,217,218,219,220,221,222,223,224,225,
	226,227,228,229,230,231,232,233,234,235,
	236,237,238,239,240,241,242,243,244,245,
	246,247,248,249,250,251
};


void PortInt(void)
{
	DDRB =  0B00000111;		// PB0(beep),PB1(pwm1),PB2(pwm2)
	PORTB = 0B00000000;		
	PINB =	0x00;

	DDRC =  0B00111111;		// PC0(DR2),PC1(DR1),PC2(ENA),PC3(ENB),PC4(RUN),PC5(FALT),PC6(CSA),PC7(VSA)
	PORTC = 0B00010000;
	PINC =	0x00;

	DDRD =  0B00000000;
	PORTD = 0B00000000;		// PD2(S1),PD3(S2)	
	PIND =	0x00;
}

void T2Int(void)			// ppm count
{
	TIMSK |= 1<<TOIE2;		// Overflow interrupt
	TCCR2 = 0;
}

void T1Int(void)					
{	
	TCCR1A = 1<<COM1A1 | 1<<COM1B1 | 1<<WGM10;	// 8 bit  phase correction PWM (CS31 CS21 CS11 divid bit) 15.68K
	TCCR1B = 1<<CS10;							// 0 divid
	OCR1A = OCR1B = 0x00;
}

void T0Int(void)			// LED pwm
{
	;
}

/**************************************
* ADC bit: 10bit
* Aref voltage: 2.5V
* sampling frequency 64 divide: 172.8KHz
**************************************/
/*
void ADCInit(void)
{
	ADCSRA = 0;
	ADMUX = 0;					// Aref 2.5V   Right-aligned
	ADCSRA = 1<<ADPS2 | 1<<ADPS0 | 1<<ADEN | 1<<ADFR;		// 64 divide 172.8KHz
}

int ADport(unsigned char port)
{
	ADMUX |= port;
	ADCSRA |= 1<<ADSC;				// start converter
	while(!(ADCSRA&(1<<ADIF)));		// wait converter over
	ADCSRA &= ~(1<<ADSC);			// stop converter
	return (ADCL | (ADCH<<8));
}
*/
void display(void)
{
	_delay_ms(300);
}

ISR(TIMER2_OVF_vect)
{
	count++;
	if(count > 127)
	{
		TCCR2 = 0;
		count = TCNT2 = 0;
	}
}


ISR(INT0_vect)
{
	;
}
ISR(INT1_vect)
{
	;
}

int main(void)
{
	PortInt();
	T1Int();
	PORTC |= BIT(4);
	PORTC &= ~BIT(5);
	PORTC &= ~BIT(2);
	PORTC &= ~BIT(3);
	SREG |= BIT(7);
	while(1)
	{
		OCR1A = OCR1B = 0x80;
	}
}
