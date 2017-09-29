#include "main.h"

unsigned int StateLoTable[8] = {0};

#define FEED_DOG	PORTD^=0x80
#define D14_BLINK	PORTB^=0x04
#define D14_OFF		PORTB&=~0x04
#define D11_BLINK	PORTB^=0x02
#define D11_ON	PORTB|=0x02
#define D11_OFF	PORTB&=~(0x02)
enum MOS{
    APH = 0x01,
    BPH = (1<<5),
    CPH = (1<<4),
    APL = (1<<6),
    BPL = (1<<5),
    CPL = (1<<3),
    ABCH = 0xE0,
};

#define LMOS_OFF	{ TCCR0A&=0xF; TCCR2A&=0x3F; }	// close all arm of lower mos
#define HMOS_OFF	{ PORTD|=0x01; PORTC|=0x30; }   // close all arm of upper mos
#define AMOS_OFF    { LMOS_OFF; HMOS_OFF; }
#define HMOS_ON		{ PORTD&=~(0x01); PORTC&=~(0x30); }
#define ALMOS_ON	TCCR0A|=1<<COM0A1
#define BLMOS_ON	TCCR0A|=1<<COM0B1
#define CLMOS_ON	TCCR2A|=1<<COM2A1
#define BREAK_ON	{ LMOS_OFF; _delay_us(15); HMOS_ON; }

//asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
#define AHCL	{ AMOS_OFF; asm("nop"); PORTD&=~APH; CLMOS_ON; }
#define AHBL	{ AMOS_OFF; asm("nop"); PORTD&=~APH; BLMOS_ON; }
#define CHBL	{ AMOS_OFF; asm("nop"); PORTC&=~CPH; BLMOS_ON; }
#define CHAL	{ AMOS_OFF; asm("nop"); PORTC&=~CPH; ALMOS_ON; }
#define BHAL	{ AMOS_OFF; asm("nop"); PORTC&=~BPH; ALMOS_ON; }
#define BHCL	{ AMOS_OFF; asm("nop"); PORTC&=~BPH; CLMOS_ON; }
#define DIS_PCINT	PCICR&=~1<<PCIE1
#define ENB_PCINT	PCICR|=1<<PCIE1

#define T1_ON   { TCCR1B=1<<CS10; TIMSK1|=1<<TOIE1; TCNT1=0; }  // 0 divide   8.19ms
#define T1_OFF  { TCCR1B=0; TIMSK1&=~1<<TOIE1; }
#define I1_ON   EIMSK=1<<INT1;
#define I1_OFF  EIMSK&=~(1<<INT1);

#define CAPT_T_FALL	{ TCCR1B=1<<ICNC1|1<<CS11;TCNT1=0; ICR1=0;}
#define CAPT_T_RIS	{ TCCR1B=1<<ICNC1|1<<ICES1|1<<CS11; TCNT1=0; }

#define FAN_ON	PORTD|=0x4
#define FAN_OFF	PORTD&=~0x4

volatile unsigned int VelInte = 0;  // velocity integral
volatile unsigned int LastValue = 0;
volatile unsigned char stall = 1;
volatile unsigned int speed = 0;
unsigned char dir = 0;
volatile unsigned char CaptStat = 1;
int value = 0;


const unsigned char porp[256] = {
	1,2,3,4,5,6,6,7,7,8,
	8,9,9,10,10,11,12,13,14,15,
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
	236,237,239,241,243,245,247,249,251,253,
	255,255,255,255,255,255
};

static void commut(unsigned char phase);        //  commutation

void PortInit(void)
{
    DDRB = 0B00001110;
    PORTB= 0B00001000;
    PINB = 0x00;

    DDRD = 0B11100101;
    PORTD= 0B01100001;
    PIND = 0x00;

    DDRC = 0B00110000;
    PORTC= 0B00110000;
    PINC = 0x00;
}

/**************************************
* ADC bit: 10bit
* Aref voltage: 2.5V
* sampling frequency 64 divide: 172.8KHz
**************************************/
void ADCInit(void)
{
	ADMUX =  1<<REFS0 | 1<<ADLAR;	// Avcc Ref  左对齐
	ADCSRA = 1<<ADPS2 | 1<<ADPS0 | 1<<ADEN;	//| 1<<ADATE ;		// 32分频
	ADCSRB = 0x00;						
	DIDR0 = 0xC8;			// ADC3.6.7
}

unsigned int AdConvert(unsigned char channal)
{
	ADMUX |= channal;
	ADCSRA |= 1<<ADSC;					// start converter
	while ((ADCSRA & 0x40));			// wait converter over  ADSC==0 on converter over
	ADMUX &= 0xF0;
	return(ADCH);
}

void T1Init(void)
{
	TCCR1B = 1<<ICNC1 | 1<<ICES1 | 1<<CS11;			// rising trigger
	TIMSK1 = 1<<ICIE1;
	TCNT1 = 0;
}

void I1Init(void)
{
    EICRA = 1<<ISC10 | 1<<ISC11;		// rising init
}

void PCInit(void)
{
    PCICR |= 1<<PCIE1;			// PCINT[14:8] 
    PCMSK1 |= 0x07;				//
}

void T0Init(void)
{
    TCCR0A = 1<<COM0A1 | 1<<COM0B1 | 1<<WGM00;		// 8bit phase
    TCCR0B = 1<<CS00;			// 0 divide
    OCR0A = OCR0B = 0x00;
}

void T2Init(void)
{
    TCCR2A = 1<<COM2A1 | 1<<WGM20;
    TCCR2B = 1<<CS20;
    OCR2A = 0x00;
}

volatile unsigned char update = 0;
unsigned char times = 0;

ISR(TIMER1_CAPT_vect)
{
	if(CaptStat)
	{
		CaptStat = 0;
		CAPT_T_FALL;
	}
	else			// falling tigger
	{
		CaptStat=1;
		CAPT_T_RIS;
		D14_BLINK;
		speed = ICR1;
		if(times++ > 2)
		{
			if(LastValue == VelInte)
				stall = 1;
			else
			{
				LastValue = VelInte;
				stall = 0;
			}
			times = 0;
		}
		if(VelInte > 60000)
			VelInte = 0;
		update = 1;
	}
}

ISR(PCINT1_vect)
{
    VelInte++;
    commut(PINC&0x07);
}

int StartFun(unsigned char SetSpeed)
{
    unsigned char i = 0;
    unsigned char temp = 0xE0;
	stall = times = 0;
    DIS_PCINT;
 //   T1_OFF;
 //   I1_OFF;
    OCR0A = OCR0B = OCR2A = temp;
    ENB_PCINT;				// Enable PC interrupt
    while(1)
    {
        for(i=0; i<8; i++)
        {
            if(VelInte > 4)     // motor start success
                goto over;
            commut(i);
            _delay_ms(10);
        }
        temp -= 8;
        if(temp < 24)
        {
            OCR0A = OCR0B = OCR2A = 0xFF;
            return 0;
        }
        else
            OCR0A = OCR0B = OCR2A = temp;
		FEED_DOG;
    }
over:
    return 1;
}

void commut(unsigned char phase)        //  commutation
{
    if(dir)
    {
        switch(phase)
        {
        case 5:
            CHBL;
            break;
        case 4:
            AHBL;
            break;
        case 6:
            AHCL;
            break;
        case 2:
            BHCL;
            break;
        case 3:
            BHAL;
            break;
        case 1:
            CHAL;
            break;
        default:
            break;
        }
    }
    else
    {
        switch(phase)
        {
        case 5:
            BHCL;
            break;
        case 4:
            BHAL;
            break;
        case 6:
            CHAL;
            break;
        case 2:
            CHBL;
            break;
        case 3:
            AHBL;
            break;
        case 1:
            AHCL;
            break;
        default:
            break;
        }
    }
}

int main(void)
{
	unsigned int lost = 0;
//	unsigned int volt = 0;
	unsigned int curr = 0;
//	unsigned int vCount = 0;
	
	unsigned char overCurr = 0;
	unsigned int temp = 0;
    int tCount = 0;
	int cCount = 0;
	cli();
    PortInit();
    D11_ON;
    _delay_ms(500);
    D11_OFF;
    FEED_DOG;
	_delay_ms(500);
    D11_ON;
    FEED_DOG;
    _delay_ms(500);
    FEED_DOG;
    D11_OFF;
	PCInit();
    T0Init();
    T2Init();
	T1Init();
	ADCInit();
	sei();
    while(1)
    {
    	FEED_DOG;
		if(update && overCurr)
		{
			update = 0;
			value = speed-1500;
			if(value >= 0)
			{
				value >>= 1;
				if(value > 255)
					value = 255;
				value = porp[255-value];
				dir = 0;
			}
			else
			{
				value = abs(value);
				value >>= 1;
				if(value > 255)
					value = 255;
				value = porp[255-value];
				dir = 1;
			}
			if(stall && value<255)
			{
				while(!StartFun(0x80))
					FEED_DOG;
				OCR0A = OCR0B = OCR2A = value;
			}
			OCR0A = OCR0B = OCR2A = value;
			if(value == 255)
				BREAK_ON;
			lost = 0;
		}
		temp = (AdConvert(3)<<2);
//		volt = (AdConvert(7)<<2);
		curr = (AdConvert(6)<<2);
	/*
		if(volt < 240)
		{
			
			if(vCount++>6000)
			{
				if(vCount < 7200)	
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
		}
*/
		if(curr > 720)		
		{
			if(cCount++>650)		// 130ms
			{
				D11_ON;
				overCurr = 0;
				OCR0A = OCR0B = OCR2A = 255;
				cCount = 0;
			}
		}
		else
		{
			if(cCount-- < -20000)		// wait for two second
			{
				D11_OFF;
				overCurr = 1;
				cCount = 0;
			}
		}

		if(temp < 362)
		{
			if(tCount++ > 2000)
			{
				tCount = 0;
				FAN_ON;
			}
		}
		else if(tCount-- < -2000)
		{
			tCount = 0;
			FAN_OFF;
		}
		
		if(lost++ > 12000)
		{
			OCR0A = OCR0B = OCR2A = 255;
			lost = 0;
			D14_OFF;
		}
    }
}


