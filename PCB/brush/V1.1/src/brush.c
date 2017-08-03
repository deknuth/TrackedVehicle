#define F_CPU 8000000UL
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>
#define setbit(x,y) x |= (1<<y) 
#define clrbit(x,y) x &= ~(1<<y)
#ifndef BIT
#define BIT(x) _BV(x)
#endif
#define BBO	0xF8			// Byte order  big endian	high three bit
#define LBO	0x07			// Byte order  little endian	low three bit

//#define D3_ON	PORTB|=0x08
//#define D3_OFF	PORTB&=~(0x08)

#define D4_ON	PORTB|=0x10
#define D4_OFF	PORTB&=~0x10
#define D4_BLINK	PORTB^=0x10

#define D5_ON	PORTB|=0x20
#define D5_OFF	PORTB&=~0x20
#define D5_BLINK	PORTB^=0x20

#define BEEP_ON		PORTD|=1<<5
#define BEEP_OFF	PORTD&=~(1<<5)
#define BEEP	PORTD^=1<<5

#define FEED_DOG	PORTD ^= 1<<6

#define T0_ON 	{ TCNT0=0; TCCR0B|=1<<CS00; TIMSK0|=1<<TOIE0; }			// ppm count 0 divide
#define T0_OFF 	{ TCCR0B&=~(1<<CS01); TIMSK0&=~(1<<TOIE0); }

#define FAN_ON	PORTD|=0x80
#define FAN_OFF	PORTD&=~0x80

const unsigned char porp[256] = {
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

void CloseHB(void);

enum LED{
	RUN = (1<<4),
	FALT = (1<<5),
};

enum MOS{
	AD2H = 1,
	AD1H = 2,
	BD2H = 4,
	BD1H = 8,
	DRA = 1,
	DRB = 2,
	FAN = 0x80,
	DOG = 0x40,
};

void PortInit(void)
{
	DDRB =  0B00111110;		// PB1(PWMB),PB2(PWBA),PB3(D3),PB4(D4),PB5(D5)
	PORTB = 0B00000000;		
	PINB =	0x00;

	DDRC =  0B00001111;		// PC0(AD2H),PC1(AD1H),PC2(BD2H),PC3(BD1H),PC5(TEMP),PC6(VSA),PC7(CSA)
	PORTC = 0B00010000;
	PINC =	0x00;

	DDRD =  0B11100011;		// PD0(DRA),PD1(DRB),PD5(BEEP),PD6(DOG),PD7(FAN);
	PORTD = 0B00000000;			
	PIND =	0x00;
}

void T2Int(void)			// Bootstraps pwm
{
	TCCR2A |= 1<<COM2A1 | 1<<WGM20 | 1<<WGM21; 		// fast pwm mode
	TCCR2B |= 1<<CS20;		// 8 divide
	OCR2A = 0x80;
}

void T1Int(void)					
{	
	OCR1A = OCR1B = 0x00;
	TCCR1A = 1<<COM1A1 | 1<<COM1B1 | 1<<WGM10;	// 8 bit  phase correction PWM (CS31 CS21 CS11 divid bit) 15.68K
	TCCR1B = 1<<CS10;							// 0 divid
}

void EiInit(void)
{
	EICRA = 1<<ISC10 | 1<<ISC11 | 1<<ISC00 | 1<<ISC01;
	EIMSK = 1<<INT1 | 1<<INT0;
}

/**************************************
* ADC bit: 10bit
* Aref voltage: 2.5V
* sampling frequency 64 divide: 172.8KHz
**************************************/
void ADCInit(void)
{
	ADMUX =  1<<REFS0 | 1<<ADLAR;	// Avcc Ref  ×ó¶ÔÆë
	ADCSRA = 1<<ADPS2 | 1<<ADPS1 | 1<<ADEN;	//| 1<<ADATE ;		// 32·ÖÆµ
	ADCSRB = 0x00;						
	DIDR0 = 0xE0;			// Êý×ÖÊäÈë½ûÖ¹	ADC5.6.7
}

unsigned int AdConvert(unsigned char channal)
{
	unsigned int c_value = 0;
	ADMUX |= channal;
	ADCSRA |= 1<<ADSC;					// start converter
	while ((ADCSRA & 0x10) == 0);			// wait converter over
	ADCSRA &= ~(1<<ADSC);				// stop converter
	ADCSRA |= 0x10;
	c_value = ADCH;
	c_value <<= 2;
	c_value |= (ADCL>>6);
	ADMUX &= 0xF0;
	return(c_value);
}

void display(void)
{
	D4_ON;
	D5_OFF;
	BEEP;
	_delay_ms(200);
	FEED_DOG;
	D5_ON;
	D4_OFF;
	BEEP;
	_delay_ms(200);
	FEED_DOG;
	D4_ON;
	D5_OFF;
	BEEP;
	_delay_ms(200);
	FEED_DOG;
	D5_ON;
	D4_OFF;
	BEEP;
	_delay_ms(200);
	FEED_DOG;
	D5_OFF;
	BEEP_OFF;
}

volatile unsigned char  t0Count = 0;
volatile unsigned char t0Start = 0;

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 0;
	t0Count++;
	if(t0Count > 100)
		t0Count = 0;
}

volatile unsigned char i0Flag = 1;
volatile unsigned char i1Flag = 1;
volatile int s0Count = 0;
volatile int s1Count = 0;
volatile unsigned int i0Start = 0;
volatile unsigned int i1Start = 0;
volatile unsigned char s0Ready = 0;
volatile unsigned char s1Ready = 0;
volatile unsigned char lost = 1;

ISR(INT0_vect)
{
	if(i0Flag)		// rising 
	{
		if(t0Start == 0)
		{
			T0_ON;
			i0Start = 0;
		}
		else
			i0Start = (((t0Count+2)<<5)+(TCNT0>>3));
		t0Start++;
		EICRA &= ~(1<<ISC00);		// change to falling
		i0Flag = 0;
	}
	else 
	{	
		s0Count = ((((t0Count+2)<<5)+(TCNT0>>3)) - i0Start);			// get ppm count time 
		t0Start--;
		if(t0Start == 0)
		{
			t0Count = 0;
			T0_OFF;
		}
		s0Ready=1;
		lost |= 0x1;
		EICRA |= (1<<ISC00);	 // change to rising request
		i0Flag = 1;
	}

}	

ISR(INT1_vect)
{
	if(i1Flag)		// rising 
	{
		if(t0Start == 0)
		{
			T0_ON;
			i1Start = 0;
		}
		else
			i1Start = (((t0Count+2)<<5)+(TCNT0>>3));
		t0Start++;
		EICRA &= ~(1<<ISC10);		// change to falling
		i1Flag = 0;
	}
	else 
	{	
		s1Count = ((((t0Count+2)<<5)+(TCNT0>>3)) - i1Start);			// get ppm count time 
		t0Start--;
		if(t0Start == 0)
		{
			t0Count = 0;
			T0_OFF;
		}
		s1Ready = 1;
		lost |= 0x2;
		EICRA |= (1<<ISC10);	 // change to rising request
		i1Flag = 1;
	}
	
}

int history[2] = {-1,-1};

void move(int LV,int RV)
{
	//if((LV&0x8000) != (history[0]&0x8000))		// 
	{
		if(LV > 0)
		{
			PORTD &= ~DRA;
			PORTC |= AD1H;		// AD1H close
			PORTC &= ~AD2H;		// AD2H open
		}
		else
		{
			PORTD |= DRA;
			PORTC |= AD2H;
			PORTC &= ~AD1H;
		}
		history[0] = LV;
	}
	
	//if((RV&0x8000) != (history[1]&0x8000))
	{
		if(RV > 0)
		{
			PORTD &= ~DRB;
			PORTC |= BD1H;
			PORTC &= ~BD2H;
		}
		else
		{
			PORTD |= DRB;
			PORTC |= BD2H;
			PORTC &= ~BD1H;
		}
		history[1] = RV;
	}
	
	LV = abs(LV);
	RV = abs(RV);
	if(LV > 508)
		LV = 508;
	if(RV > 508)
		RV = 508;

	OCR1A = LV>>1;
	OCR1B = RV>>1;
}

void CloseHB(void)		// close H bridge and break
{
	OCR1A = OCR1B = 0x00;
	_delay_us(5);
	PORTC &= ~AD1H;		// AD1H open
	PORTC &= ~AD2H;		// AD2H open
	PORTC &= ~BD1H;		// BD1H open
	PORTC &= ~BD2H;		// BD2H open
}

int main(void)
{
	cli();
	unsigned int volt = 0;
	unsigned int curr = 0;
	unsigned int vCount = 0;
	unsigned int lCount = 0;
	unsigned int tCount = 0;
	unsigned char overCurr = 1;
	unsigned int temp = 0;
	int cCount = 0;
	
	PortInit();
	display();
	T1Int();
	T2Int();
	ADCInit();
	SREG |= BIT(7);
	CloseHB();
	EiInit();
	
//	move(0,0);		// init hitory
	while(1)		// drop six time signal
	{
		FEED_DOG;
		if(lost==3)
		{
			lost = 0;
			if(volt++ > 6)
				break;
		}
		_delay_us(500);
		if(curr++ > 15000)
			break;
	}
	volt = curr = 0;
	while(1)
	{
		FEED_DOG;
		if(lost==3 && overCurr)
		{
			D4_BLINK;
			lCount = 0;
//			if(s1Ready)
			{
				s1Ready = 0;		
				s1Count -= 1500;
				if(s1Count > 0)
				{
					PORTD &= ~DRB;
					PORTC |= BD1H;
					PORTC &= ~BD2H;
				}
				else
				{
					s1Count = abs(s1Count);
					PORTD |= DRB;
					PORTC |= BD2H;
					PORTC &= ~BD1H;
				}
				if(s1Count > 511)
					s1Count = 511;
				s1Count >>= 1;
				OCR1A = porp[s1Count];
				if(porp[s1Count] == 0)		// car break
				{
					PORTC &= ~BD2H;
					PORTC &= ~BD1H;
				}
				lost &= ~(0x2);
			}
		
//			if(s0Ready)
			{
				s0Ready = 0;
				s0Count -= 1500;
				if(s0Count > 0)
				{
					PORTD &= ~DRA;
					PORTC |= AD1H;		// AD1H close
					PORTC &= ~AD2H;		// AD2H open
				}
				else
				{
					s0Count = abs(s0Count);
					PORTD |= DRA;
					PORTC |= AD2H;
					PORTC &= ~AD1H;
				}
				
				if(s0Count > 511)
					s0Count = 511;
				s0Count >>= 1;
				OCR1B = porp[s0Count];
				if(porp[s0Count] == 0)
				{
					PORTC &= ~AD2H;		// AD2H open
					PORTC &= ~AD1H;		
				}
				lost &= ~(0x1);
			}
		}
		
	//	temp = AdConvert(5);
		volt = AdConvert(6);
		curr = AdConvert(7);
		
		if(volt < 240)
		{
			
			if(vCount++>40000)
			{
				if(vCount < 46000)	
					BEEP_ON;
				else
				{
					vCount = 0;
					BEEP_OFF;
				}
			}
		}
		else
		{
			vCount = 0;
			BEEP_OFF;
		}
		

		if(curr > 1010)
		{
			if(cCount++>2000)
			{
				D5_ON;
				overCurr = 0;
				OCR1A = OCR1B = 0;
				cCount = 0;
			}
		}
		else
		{
			if(cCount-- < -32000)
			{
				D5_OFF;
				overCurr = 1;
				cCount = 0;
			}
		}
		
		
		if(temp < 358)
		{
			if(tCount++ > 5000)
			{
				tCount = 0;
				FAN_ON;
			}
		}
		else if(tCount-- < -3000)
		{
			tCount = 0;
			FAN_OFF;
		}

		if(lCount++ > 6000)
		{
			lost = 0;			// lost control
			lCount = 0;
			D4_OFF;
			OCR1A = OCR1B = 0;
		}
	}
}
